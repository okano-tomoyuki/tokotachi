/**
 * *.ttui.json を開くデザイナー(docs/designer/editor-design.md §3)。TextDocument を唯一の正とする(Bethany と同じ)。
 * 内容が変わるたびに Webview に送り、検証のエラーが無ければキャンバスを描き直して送る。
 */
import type {
  EditCommand,
  ExtensionToWebviewMessage,
  WebviewToExtensionMessage,
} from '@tokotachi-designer/core';
import {
  checkThemeOverride,
  withCustomClasses,
  type ClassInfo,
  DOCUMENT_EXTENSION,
  hasErrors,
  IMAGE_EXTENSIONS,
  isImagePath,
  parseDocument,
  PROJECT_EXTENSION,
  relativePath,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { applyEditCommand } from './applyEditCommand.ts';
import { renderCanvas } from './canvasService.ts';
import { customClassesOf, isHeaderFile } from './customWidgets.ts';
import {
  componentResolver,
  listComponents,
  loadComponents,
  readComponentImages,
} from './components.ts';
import { generateWindowCode, readImages } from './generateCode.ts';
import { goToHandler } from './goToHandler.ts';
import { projectOf } from './projects.ts';
import { colorScheme, SECTION } from './settings.ts';

export class DesignerEditorProvider implements vscode.CustomTextEditorProvider {
  static readonly viewType = 'tokotachiDesigner.designer';

  static register(context: vscode.ExtensionContext): vscode.Disposable {
    return vscode.window.registerCustomEditorProvider(
      DesignerEditorProvider.viewType,
      new DesignerEditorProvider(context),
      { webviewOptions: { retainContextWhenHidden: true } },
    );
  }

  private readonly context: vscode.ExtensionContext;

  private constructor(context: vscode.ExtensionContext) {
    this.context = context;
  }

  resolveCustomTextEditor(document: vscode.TextDocument, panel: vscode.WebviewPanel): void {
    const webviewRoot = vscode.Uri.joinPath(this.context.extensionUri, 'dist', 'webview');
    panel.webview.options = { enableScripts: true, localResourceRoots: [webviewRoot] };
    panel.webview.html = renderHtml(panel.webview, webviewRoot);

    const post = (message: ExtensionToWebviewMessage): Thenable<boolean> =>
      panel.webview.postMessage(message);
    const postDocument = (): void => {
      void post({
        type: 'document',
        version: document.version,
        text: document.getText(),
        fileName: document.uri.path.split('/').pop() ?? '',
      });
      scheduleRender();
    };

    // 自作部品(docs/adr/0044)。プロジェクト・ヘッダが変わったら読み直し、変わっていれば Webview に送る
    let customClasses: Record<string, ClassInfo> = {};
    let customJson = '{}';
    const refreshCustomClasses = async (): Promise<void> => {
      const classes = await customClassesOf(document.uri);
      const json = JSON.stringify(classes);
      if (json !== customJson) {
        customJson = json;
        customClasses = classes;
        await post({ type: 'customClasses', classes });
      }
    };

    // キャンバスで表示するタブ(Webview の選択に合わせる。定義ファイルは変えない)
    let tabs: Readonly<Record<string, number>> = {};
    // 描き直しは、変更が続いたときは最後の内容だけにする
    let renderTimer: ReturnType<typeof setTimeout> | undefined;
    const scheduleRender = (): void => {
      if (renderTimer !== undefined) {
        clearTimeout(renderTimer);
      }
      renderTimer = setTimeout(() => {
        renderTimer = undefined;
        void render();
      }, 30);
    };
    const render = async (): Promise<void> => {
      await refreshCustomClasses();
      const classes = customClasses;
      const parsed = withCustomClasses(classes, () => parseDocument(document.getText()));
      if (!parsed.document || hasErrors(parsed.diagnostics)) {
        return;
      }
      try {
        const images = await readImages(vscode.Uri.joinPath(document.uri, '..'), parsed.document);
        // 置いた部品ファイル(docs/adr/0043)は、その中身を展開して描く
        const { components } = await loadComponents(document.uri, parsed.document, classes);
        await readComponentImages(components, images);
        // プロジェクトに属していれば、そのテーマで描く(project-spec.md §4.1)
        const theme = (await projectOf(document.uri))?.project.doc.theme;
        await post({ type: 'themePresets', names: Object.keys(theme?.presets ?? {}) });
        const override =
          theme?.override !== undefined && checkThemeOverride(theme.override) === null
            ? theme.override
            : undefined;
        const frame = await renderCanvas(
          classes,
          this.context,
          parsed.document,
          document.version,
          colorScheme(theme?.color_scheme),
          tabs,
          images,
          override,
          theme?.presets,
          componentResolver(components),
        );
        await post({ type: 'canvas', frame });
      } catch (e) {
        await post({ type: 'canvasError', message: e instanceof Error ? e.message : String(e) });
      }
    };

    // 編集は届いた順に 1 つずつ適用する(WorkspaceEdit の適用が重ならないように)
    let editQueue = Promise.resolve();
    const enqueueEdit = (requestId: number, command: EditCommand): void => {
      editQueue = editQueue.then(async () => {
        const result = await applyEditCommand(document, command).catch((e: unknown) => ({
          ok: false as const,
          error: vscode.l10n.t('Unexpected error: {0}', e instanceof Error ? e.message : String(e)),
        }));
        await post({ type: 'editResult', requestId, ...result });
      });
    };

    // 画像を選んでもらい、定義ファイルからの相対パスを設定する
    const pickImage = async (target: readonly (string | number)[], property: string) => {
      const directory = vscode.Uri.joinPath(document.uri, '..');
      const picked = await vscode.window.showOpenDialog({
        defaultUri: directory,
        canSelectMany: false,
        filters: { [vscode.l10n.t('Images')]: [...IMAGE_EXTENSIONS] },
      });
      const file = picked?.[0];
      if (!file) {
        return;
      }
      const path =
        file.scheme === directory.scheme && file.authority === directory.authority
          ? relativePath(directory.path, file.path)
          : '';
      if (!isImagePath(path)) {
        void vscode.window.showErrorMessage(
          vscode.l10n.t('This file cannot be used as an image: {0}', file.fsPath),
        );
        return;
      }
      enqueueEdit(0, { kind: 'setProperties', target, properties: { [property]: path } });
    };

    // 画像のファイルが変わったら描き直す(参照しているかは問わない。上のフォルダの画像も参照できるので、ワークスペース全体を見る)
    const imageWatcher = vscode.workspace.createFileSystemWatcher(
      `**/*.{${IMAGE_EXTENSIONS.join(',')}}`,
    );

    // 自作部品のヘッダが変わったら、読み直して描き直す
    const headerWatcher = vscode.workspace.createFileSystemWatcher('**/*.{h,hpp,hh,hxx}');
    // 置いた部品ファイルが作られた・消えたときも描き直す
    const componentWatcher = vscode.workspace.createFileSystemWatcher(`**/*${DOCUMENT_EXTENSION}`);
    // パレットの部品ファイルの一覧(ほかの定義ファイルが変わったら作り直す。続いたときは最後だけ)
    let componentsTimer: ReturnType<typeof setTimeout> | undefined;
    const scheduleComponents = (): void => {
      if (componentsTimer !== undefined) {
        clearTimeout(componentsTimer);
      }
      componentsTimer = setTimeout(() => {
        componentsTimer = undefined;
        void listComponents(document.uri, customClasses).then((items) =>
          post({ type: 'components', items }),
        );
      }, 300);
    };

    const subscriptions = [
      imageWatcher,
      imageWatcher.onDidChange(scheduleRender),
      imageWatcher.onDidCreate(scheduleRender),
      imageWatcher.onDidDelete(scheduleRender),
      panel.webview.onDidReceiveMessage((message: WebviewToExtensionMessage) => {
        switch (message.type) {
          case 'ready':
            // 自作部品を先に送ってから、内容を送る(Webview は自作部品をカタログに足して検証する)
            void refreshCustomClasses().then(postDocument);
            scheduleComponents();
            break;
          case 'edit':
            enqueueEdit(message.requestId, message.command);
            break;
          case 'generateCode':
            void generateWindowCode(document, { quiet: false });
            break;
          case 'view':
            tabs = message.tabs;
            scheduleRender();
            break;
          case 'openAsText':
            void vscode.commands.executeCommand('vscode.openWith', document.uri, 'default');
            break;
          case 'openComponent':
            void vscode.commands.executeCommand(
              'vscode.openWith',
              vscode.Uri.joinPath(document.uri, '..', message.path),
              DesignerEditorProvider.viewType,
            );
            break;
          case 'pickImage':
            void pickImage(message.target, message.property);
            break;
          case 'goToHandler': {
            // 直前の編集(ハンドラの名前の設定)が適用されてから生成する
            const { handler } = message;
            editQueue = editQueue.then(() =>
              goToHandler(document, handler).catch((e: unknown) => {
                void vscode.window.showErrorMessage(
                  vscode.l10n.t(
                    'Unexpected error: {0}',
                    e instanceof Error ? e.message : String(e),
                  ),
                );
              }),
            );
            break;
          }
        }
      }),
      // テキストエディタでの編集や Undo・Redo も、この経路で Webview に届く
      vscode.workspace.onDidChangeTextDocument((e) => {
        if (e.document.uri.toString() === document.uri.toString()) {
          postDocument();
        }
      }),
      vscode.window.onDidChangeActiveColorTheme(() => {
        scheduleRender();
      }),
      // プロジェクトのテーマ・ほかの定義ファイル(置いた部品ファイル)が変わったら描き直す(保存していない変更も)
      vscode.workspace.onDidChangeTextDocument((e) => {
        const path = e.document.uri.path;
        if (
          path.endsWith(PROJECT_EXTENSION) ||
          isHeaderFile(e.document.uri) ||
          (path.endsWith(DOCUMENT_EXTENSION) &&
            e.document.uri.toString() !== document.uri.toString())
        ) {
          scheduleRender();
        }
        if (path.endsWith(DOCUMENT_EXTENSION)) {
          scheduleComponents();
        }
      }),
      componentWatcher,
      headerWatcher,
      headerWatcher.onDidChange(scheduleRender),
      headerWatcher.onDidCreate(scheduleRender),
      headerWatcher.onDidDelete(scheduleRender),
      ...[
        componentWatcher.onDidChange,
        componentWatcher.onDidCreate,
        componentWatcher.onDidDelete,
      ].map((on) =>
        on(() => {
          scheduleRender();
          scheduleComponents();
        }),
      ),
      vscode.workspace.onDidChangeConfiguration((e) => {
        if (e.affectsConfiguration(`${SECTION}.canvas`)) {
          scheduleRender();
        }
      }),
    ];
    panel.onDidDispose(() => {
      if (renderTimer !== undefined) {
        clearTimeout(renderTimer);
      }
      if (componentsTimer !== undefined) {
        clearTimeout(componentsTimer);
      }
      for (const s of subscriptions) {
        s.dispose();
      }
    });
  }
}

/**
 * Webview の HTML。デザイナーとプロジェクトの設定画面で同じ成果物を使い、view で出す画面を選ぶ。
 */
export function renderHtml(
  webview: vscode.Webview,
  webviewRoot: vscode.Uri,
  view: 'designer' | 'project' = 'designer',
): string {
  const scriptUri = webview.asWebviewUri(vscode.Uri.joinPath(webviewRoot, 'main.js'));
  const styleUri = webview.asWebviewUri(vscode.Uri.joinPath(webviewRoot, 'main.css'));
  const nonce = createNonce();
  // 訳は描画の前に要るので、HTML に埋め込んで渡す
  const l10n = escapeAttribute(JSON.stringify(vscode.l10n.bundle ?? {}));
  const language = escapeAttribute(vscode.env.language);
  return `<!DOCTYPE html>
<html lang="${language}">
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Content-Security-Policy"
    content="default-src 'none'; img-src ${webview.cspSource} blob: data:; style-src ${webview.cspSource} 'unsafe-inline'; font-src ${webview.cspSource}; script-src 'nonce-${nonce}';">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="stylesheet" href="${styleUri.toString()}">
  <title>Tokotachi Designer</title>
</head>
<body>
  <div id="root" data-view="${view}" data-language="${language}" data-l10n="${l10n}"></div>
  <script type="module" nonce="${nonce}" src="${scriptUri.toString()}"></script>
</body>
</html>`;
}

function escapeAttribute(text: string): string {
  return text
    .replace(/&/g, '&amp;')
    .replace(/"/g, '&quot;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

function createNonce(): string {
  const bytes = new Uint8Array(16);
  crypto.getRandomValues(bytes);
  return Array.from(bytes, (b) => b.toString(16).padStart(2, '0')).join('');
}
