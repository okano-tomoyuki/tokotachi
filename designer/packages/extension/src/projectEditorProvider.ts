/**
 * *.ttproj.json を開く設定画面(docs/designer/project-spec.md §6)。デザイナーと同じく TextDocument を唯一の正とし、
 * 画面からの編集は決まった形で書き出して、変わった範囲だけを置き換える(Undo が効き、保存は利用者が行う)。
 * テーマの見本のウィンドウを、キャンバスと同じ WebAssembly の Tokotachi で描いて送る。
 */
import {
  checkThemeOverride,
  DOCUMENT_EXTENSION,
  isComponentDocument,
  minimalTextEdit,
  parseDocument,
  parseProject,
  replaceThemePreset,
  sameWindowPath,
  serializeDocument,
  serializeProject,
  themePresetReferences,
  themePreviewDocument,
  type ExtensionToWebviewMessage,
  type TtprojDocument,
  type WebviewToExtensionMessage,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { renderCanvas } from './canvasService.ts';
import { renderHtml } from './designerEditorProvider.ts';
import { generateProjectCode } from './generateCode.ts';
import { readText, windowPathIn, windowUri } from './projects.ts';

type Result = { readonly ok: true } | { readonly ok: false; readonly error: string };

export class ProjectEditorProvider implements vscode.CustomTextEditorProvider {
  static readonly viewType = 'tokotachiDesigner.projectEditor';

  static register(context: vscode.ExtensionContext): vscode.Disposable {
    return vscode.window.registerCustomEditorProvider(
      ProjectEditorProvider.viewType,
      new ProjectEditorProvider(context),
    );
  }

  private readonly context: vscode.ExtensionContext;

  private constructor(context: vscode.ExtensionContext) {
    this.context = context;
  }

  resolveCustomTextEditor(document: vscode.TextDocument, panel: vscode.WebviewPanel): void {
    const webviewRoot = vscode.Uri.joinPath(this.context.extensionUri, 'dist', 'webview');
    panel.webview.options = { enableScripts: true, localResourceRoots: [webviewRoot] };
    panel.webview.html = renderHtml(panel.webview, webviewRoot, 'project');

    const post = (message: ExtensionToWebviewMessage): Thenable<boolean> =>
      panel.webview.postMessage(message);
    // 見本で描いているテーマ(登録したテーマの名前。null ならプロジェクトのテーマそのもの)
    let previewTarget: string | null = null;
    let previewTimer: ReturnType<typeof setTimeout> | undefined;
    const schedulePreview = (): void => {
      clearTimeout(previewTimer);
      previewTimer = setTimeout(() => {
        void preview();
      }, 30);
    };
    const preview = async (): Promise<void> => {
      const project = parseProject(document.getText()).project;
      if (!project) {
        return;
      }
      const target = previewTarget;
      const theme = project.theme;
      const override =
        theme?.override !== undefined && checkThemeOverride(theme.override, false) === null
          ? theme.override
          : undefined;
      const doc = themePreviewDocument(target === null ? undefined : { preset: target });
      try {
        const [light, dark] = await Promise.all(
          (['LIGHT', 'DARK'] as const).map((scheme) =>
            renderCanvas(
              {},
              this.context,
              doc,
              document.version,
              scheme,
              {},
              new Map(),
              override,
              theme?.presets,
            ),
          ),
        );
        await post({ type: 'themePreview', target, light, dark });
      } catch (e) {
        await post({
          type: 'themePreview',
          target,
          error: e instanceof Error ? e.message : String(e),
        });
      }
    };

    const postDocument = async (): Promise<void> => {
      await post({
        type: 'document',
        version: document.version,
        text: document.getText(),
        fileName: document.uri.path.split('/').pop() ?? '',
      });
      schedulePreview();
      await post({ type: 'projectInfo', ...(await projectInfo(document)) });
    };

    // 編集は届いた順に 1 つずつ適用する(WorkspaceEdit の適用が重ならないように)
    let editQueue = Promise.resolve();
    const enqueue = (requestId: number, run: () => Promise<Result>): void => {
      editQueue = editQueue.then(async () => {
        const result = await run().catch((e: unknown) => ({
          ok: false as const,
          error: vscode.l10n.t('Unexpected error: {0}', e instanceof Error ? e.message : String(e)),
        }));
        await post({ type: 'editResult', requestId, ...result });
      });
    };

    // ウィンドウの定義ファイルができた・消えたら、加えられるウィンドウを送り直す
    const watcher = vscode.workspace.createFileSystemWatcher(`**/*${DOCUMENT_EXTENSION}`);
    const refreshInfo = (): void => {
      void projectInfo(document).then((info) => post({ type: 'projectInfo', ...info }));
    };
    const subscriptions = [
      watcher,
      watcher.onDidCreate(refreshInfo),
      watcher.onDidDelete(refreshInfo),
      panel.webview.onDidReceiveMessage((message: WebviewToExtensionMessage) => {
        switch (message.type) {
          case 'ready':
            void postDocument();
            break;
          case 'editProject': {
            const { project } = message;
            enqueue(message.requestId, () => applyProject(document, project));
            break;
          }
          case 'renamePreset': {
            const { from, to } = message;
            enqueue(message.requestId, () => renamePreset(document, from, to));
            break;
          }
          case 'previewTheme':
            previewTarget = message.target;
            schedulePreview();
            break;
          case 'generateProject':
            void generateProjectCode(document.uri, { quiet: false }, true);
            break;
          case 'openWindow':
            void vscode.commands.executeCommand(
              'vscode.openWith',
              windowUri(document.uri, message.path),
              'tokotachiDesigner.designer',
            );
            break;
          case 'openAsText':
            void vscode.commands.executeCommand('vscode.openWith', document.uri, 'default');
            break;
          default:
            break;
        }
      }),
      // テキストエディタでの編集や Undo・Redo も、この経路で Webview に届く
      vscode.workspace.onDidChangeTextDocument((e) => {
        if (e.document.uri.toString() === document.uri.toString()) {
          void postDocument();
        } else if (e.document.uri.path.endsWith(DOCUMENT_EXTENSION)) {
          // ウィンドウでテーマの参照を変えたら、使っている数を送り直す
          refreshInfo();
        }
      }),
    ];
    panel.onDidDispose(() => {
      clearTimeout(previewTimer);
      for (const s of subscriptions) {
        s.dispose();
      }
    });
  }
}

/** 加えられるウィンドウと、登録したテーマを参照している部品の数。 */
async function projectInfo(
  document: vscode.TextDocument,
): Promise<{ candidates: string[]; presetUsage: Record<string, number>; components: string[] }> {
  const project = parseProject(document.getText()).project;
  const windows = project?.windows ?? [];
  const files = await vscode.workspace.findFiles(
    `**/*${DOCUMENT_EXTENSION}`,
    '**/{node_modules,.git,.cache}/**',
  );
  const candidates = files
    .map((uri) => windowPathIn(document.uri, uri))
    .filter((path) => !windows.some((w) => sameWindowPath(w, path)))
    .sort((a, b) => a.localeCompare(b));
  const presetUsage: Record<string, number> = {};
  const components: string[] = [];
  for (const path of [...windows, ...candidates]) {
    const text = await readText(windowUri(document.uri, path));
    const doc = text === undefined ? undefined : parseDocument(text).document;
    if (doc && isComponentDocument(doc)) {
      components.push(path);
    }
    if (!windows.includes(path)) {
      continue;
    }
    for (const ref of doc ? themePresetReferences(doc) : []) {
      presetUsage[ref.name] = (presetUsage[ref.name] ?? 0) + 1;
    }
  }
  return { candidates, presetUsage, components };
}

function eolOf(document: vscode.TextDocument): string {
  return document.eol === vscode.EndOfLine.CRLF ? '\r\n' : '\n';
}

/** document の内容を text にする編集を edit に足す(変わった範囲だけ)。 */
function replaceText(
  edit: vscode.WorkspaceEdit,
  document: vscode.TextDocument,
  text: string,
): void {
  const change = minimalTextEdit(document.getText(), text.replace(/\n/g, eolOf(document)));
  if (change) {
    edit.replace(
      document.uri,
      new vscode.Range(document.positionAt(change.start), document.positionAt(change.end)),
      change.text,
    );
  }
}

/** 画面で編集したプロジェクトを書き出して当てる。形が合わないものは書かない。 */
async function applyProject(
  document: vscode.TextDocument,
  project: TtprojDocument,
): Promise<Result> {
  const text = serializeProject(project);
  if (!parseProject(text).project) {
    return { ok: false, error: vscode.l10n.t('The project settings are invalid') };
  }
  const edit = new vscode.WorkspaceEdit();
  replaceText(edit, document, text);
  return (await vscode.workspace.applyEdit(edit))
    ? { ok: true }
    : { ok: false, error: vscode.l10n.t('Failed to apply the change to the file') };
}

/**
 * 登録したテーマを改名する(to が null なら消す)。プロジェクトのウィンドウの参照も 1 つの編集で書き換え、
 * 書き換える前に保存していたウィンドウは保存する(プロジェクトファイルは画面の編集と同じく、保存は利用者が行う)。
 */
async function renamePreset(
  document: vscode.TextDocument,
  from: string,
  to: string | null,
): Promise<Result> {
  const project = parseProject(document.getText()).project;
  const presets = project?.theme?.presets ?? {};
  if (!project || !Object.hasOwn(presets, from)) {
    return {
      ok: false,
      error: vscode.l10n.t('The theme "{0}" is not registered in the project', from),
    };
  }
  if (to !== null && to !== from && Object.hasOwn(presets, to)) {
    return { ok: false, error: vscode.l10n.t('The theme "{0}" already exists', to) };
  }
  // 参照しているウィンドウを書き換える
  const windows: { document: vscode.TextDocument; text: string; wasDirty: boolean }[] = [];
  let count = 0;
  for (const path of project.windows ?? []) {
    const uri = windowUri(document.uri, path);
    const text = await readText(uri);
    const doc = text === undefined ? undefined : parseDocument(text).document;
    if (!doc) {
      continue;
    }
    const replaced = replaceThemePreset(doc, from, to ?? undefined);
    if (replaced.count === 0) {
      continue;
    }
    count += replaced.count;
    const opened = await vscode.workspace.openTextDocument(uri);
    windows.push({
      document: opened,
      text: serializeDocument(replaced.document),
      wasDirty: opened.isDirty,
    });
  }
  if (to === null && count > 0) {
    const remove = vscode.l10n.t('Delete');
    const answer = await vscode.window.showWarningMessage(
      vscode.l10n.t(
        'The theme "{0}" is used by {1} widget(s). Delete it and remove the references?',
        from,
        count,
      ),
      { modal: true },
      remove,
    );
    if (answer !== remove) {
      return { ok: true };
    }
  }
  // キーの順を保ったまま改名する(消すときは除く)
  const nextPresets = Object.fromEntries(
    Object.entries(presets).flatMap(([name, value]) =>
      name !== from ? [[name, value]] : to === null ? [] : [[to, value]],
    ),
  );
  const theme = { ...project.theme };
  if (Object.keys(nextPresets).length > 0) {
    theme.presets = nextPresets;
  } else {
    delete theme.presets;
  }
  const next: TtprojDocument = { ...project };
  if (Object.keys(theme).length > 0) {
    next.theme = theme;
  } else {
    delete next.theme;
  }
  const edit = new vscode.WorkspaceEdit();
  replaceText(edit, document, serializeProject(next));
  for (const w of windows) {
    replaceText(edit, w.document, w.text);
  }
  if (!(await vscode.workspace.applyEdit(edit))) {
    return { ok: false, error: vscode.l10n.t('Failed to apply the change to the file') };
  }
  for (const w of windows) {
    if (!w.wasDirty) {
      await w.document.save();
    }
  }
  return { ok: true };
}
