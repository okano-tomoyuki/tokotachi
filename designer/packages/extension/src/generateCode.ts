/**
 * コード生成(docs/designer/codegen-design.md・editor-design.md §6)。コマンド「コードを生成」・デザイナーの画面・保存したときに呼ばれる。
 * Bethany の generateCode を土台にする。ウィンドウがプロジェクトに属していればプロジェクトの codegen の設定で、
 * 属していなければ拡張の設定(tokotachiDesigner.codegen.*)で生成する。プロジェクトファイルからは起動部分(main.cpp・CMakeLists.txt)を生成する。
 */
import {
  generateCpp,
  generateProject,
  resolveCppTarget,
  windowCppOptions,
  type CppTarget,
  type GenerateResult,
  type ProjectWindow,
} from '@tokotachi-designer/codegen';
import {
  DOCUMENT_EXTENSION,
  hasErrors,
  imageReferences,
  normalizePath,
  parseDocument,
  parseProject,
  PROJECT_EXTENSION,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { projectOf, readText, windowUri } from './projects.ts';
import { cppOptions, generateOnSave } from './settings.ts';

const output = { channel: undefined as vscode.OutputChannel | undefined };

export interface GenerateOptions {
  /** 成功の通知を出さない(ハンドラへの移動・保存したとき)。 */
  readonly quiet: boolean;
  /** 保存したときの生成。検証のエラーを知らせず、手で変えた区間があれば上書きせずに知らせる。 */
  readonly onSave?: boolean;
  /** 手で変えた区間を確かめずに上書きする。 */
  readonly overwrite?: boolean;
}

/** 生成した結果(出力先は定義ファイルのフォルダからの相対パス)。 */
export interface CodeResult {
  readonly directory: vscode.Uri;
  readonly target: CppTarget;
}

/** 定義ファイルを保存したら、コードを生成する(設定 tokotachiDesigner.codegen.generateOnSave)。 */
export function registerGenerateOnSave(): vscode.Disposable {
  // 同じファイルの保存が続いたとき(自動保存など)に、生成が重ならないように順に行う
  const running = new Map<string, Promise<unknown>>();
  return vscode.workspace.onDidSaveTextDocument((document) => {
    const isProject = document.uri.path.endsWith(PROJECT_EXTENSION);
    if (!isProject && !document.uri.path.endsWith(DOCUMENT_EXTENSION)) {
      return;
    }
    const key = document.uri.toString();
    const run = (running.get(key) ?? Promise.resolve())
      .then(async (): Promise<unknown> => {
        if (isProject) {
          return generateProjectCode(document.uri, { quiet: true, onSave: true }, false);
        }
        return generateWindowCode(document, { quiet: true, onSave: true });
      })
      .catch((error: unknown) => {
        void vscode.window.showErrorMessage(String(error));
      });
    running.set(key, run);
    void run.finally(() => {
      if (running.get(key) === run) {
        running.delete(key);
      }
    });
  });
}

/**
 * ウィンドウのコードを生成する。生成できなかった・取りやめたときは undefined。
 */
export async function generateWindowCode(
  document: vscode.TextDocument,
  options: GenerateOptions,
): Promise<CodeResult | undefined> {
  const parsed = parseDocument(document.getText());
  if (!parsed.document || hasErrors(parsed.diagnostics)) {
    if (!options.onSave) {
      void vscode.window.showErrorMessage(
        vscode.l10n.t(
          'Cannot generate code because the window definition has errors. See the Problems panel.',
        ),
      );
    }
    return undefined;
  }
  const doc = parsed.document;
  const fileName = document.uri.path.split('/').pop() ?? `window${DOCUMENT_EXTENSION}`;
  const directory = vscode.Uri.joinPath(document.uri, '..');
  // プロジェクトに属していれば、プロジェクトの設定で生成する
  const owner = await projectOf(document.uri);
  if (
    options.onSave &&
    (owner ? owner.project.doc.codegen?.generate_on_save === false : !generateOnSave(document.uri))
  ) {
    return undefined;
  }
  const settings = owner
    ? windowCppOptions(owner.project.doc, owner.path)
    : cppOptions(document.uri);
  const target = resolveCppTarget(doc, fileName, settings);
  // 出力先の既存の内容を先に読んでおく(生成は同期的に行う)
  const existing = new Map<string, string | undefined>();
  for (const path of [target.header, target.source]) {
    existing.set(path, await readText(vscode.Uri.joinPath(directory, path)));
  }
  // 画像も先に読む(生成したコードに埋め込む)
  const images = await readImages(directory, doc);
  const generated = generateCpp(
    doc,
    fileName,
    (path) => existing.get(path),
    settings,
    (path) => images.get(path),
  );
  const written = await writeGenerated(directory, generated, existing, options, () =>
    generateWindowCode(document, { quiet: true, overwrite: true }),
  );
  return written ? { directory, target } : undefined;
}

/** 定義ファイルが参照する画像を読む(正規化したパス → 中身。読めないものは入れない)。 */
export async function readImages(
  directory: vscode.Uri,
  doc: TtuiDocument,
): Promise<Map<string, Uint8Array>> {
  const images = new Map<string, Uint8Array>();
  for (const ref of imageReferences(doc)) {
    const path = normalizePath(ref.path);
    if (images.has(path)) {
      continue;
    }
    try {
      images.set(path, await vscode.workspace.fs.readFile(vscode.Uri.joinPath(directory, path)));
    } catch {
      // 読めない画像はコード生成がエラーにする
    }
  }
  return images;
}

/**
 * プロジェクトの起動部分(main.cpp・CMakeLists.txt)を生成する。all ならプロジェクトのすべてのウィンドウのコードも生成する。
 * 生成できなかった・取りやめたときは false。
 */
export async function generateProjectCode(
  uri: vscode.Uri,
  options: GenerateOptions,
  all: boolean,
): Promise<boolean> {
  const text = await readText(uri);
  const parsed = text === undefined ? undefined : parseProject(text);
  if (!parsed?.project || hasErrors(parsed.diagnostics)) {
    if (!options.onSave) {
      void vscode.window.showErrorMessage(
        vscode.l10n.t(
          'Cannot generate code because the project file has errors. See the Problems panel.',
        ),
      );
    }
    return false;
  }
  const project = parsed.project;
  if (options.onSave && project.codegen?.generate_on_save === false) {
    return false;
  }
  const windows: ProjectWindow[] = [];
  const failed: string[] = [];
  for (const path of project.windows ?? []) {
    const wUri = windowUri(uri, path);
    const wText = await readText(wUri);
    const w = wText === undefined ? undefined : parseDocument(wText);
    if (!w?.document || hasErrors(w.diagnostics)) {
      failed.push(path);
      continue;
    }
    windows.push({ path, doc: w.document });
    if (all) {
      const result = await generateWindowCode(await vscode.workspace.openTextDocument(wUri), {
        quiet: true,
      });
      if (!result) {
        failed.push(path);
      }
    }
  }
  if (failed.length > 0 && !options.onSave) {
    void vscode.window.showWarningMessage(
      vscode.l10n.t('These windows were not generated because of errors: {0}', failed.join(', ')),
    );
  }
  const fileName = uri.path.split('/').pop() ?? `app${PROJECT_EXTENSION}`;
  const directory = vscode.Uri.joinPath(uri, '..');
  const existing = new Map<string, string | undefined>();
  for (const path of ['main.cpp', 'CMakeLists.txt']) {
    existing.set(path, await readText(vscode.Uri.joinPath(directory, path)));
  }
  const generated = generateProject(project, fileName, windows, (path) => existing.get(path));
  if ('error' in generated) {
    if (!options.onSave) {
      void vscode.window.showErrorMessage(generated.error);
    }
    return false;
  }
  return writeGenerated(directory, generated, existing, options, () =>
    generateProjectCode(uri, { quiet: true, overwrite: true }, false),
  );
}

/**
 * 生成した結果を確かめて書く(手で変えた区間があれば上書きしてよいか聞く)。
 * 書いた(変わらなかったときを含む)なら true、エラー・取りやめなら false。
 */
async function writeGenerated(
  directory: vscode.Uri,
  generated: GenerateResult,
  existing: ReadonlyMap<string, string | undefined>,
  { quiet, onSave = false, overwrite = false }: GenerateOptions,
  regenerate: () => Promise<unknown>,
): Promise<boolean> {
  const failed = generated.files.find((f) => !f.result.ok);
  if (failed && !failed.result.ok) {
    void vscode.window.showErrorMessage(`${failed.path}: ${failed.result.error}`);
    return false;
  }
  if (generated.warnings.length > 0) {
    const channel = (output.channel ??= vscode.window.createOutputChannel('Tokotachi Designer'));
    for (const warning of generated.warnings) {
      channel.appendLine(warning);
    }
    const message = vscode.l10n.t('Code generation reported warnings. See the output for details.');
    const show = vscode.l10n.t('Show Details');
    if (onSave) {
      vscode.window.setStatusBarMessage(message, 5000);
    } else {
      void vscode.window.showWarningMessage(message, show).then((answer) => {
        if (answer === show) {
          channel.show();
        }
      });
    }
  }
  const files = generated.files.flatMap((f) =>
    f.result.ok ? [{ path: f.path, result: f.result }] : [],
  );
  const modified = files.filter((f) => f.result.modifiedRegions.length > 0);
  if (modified.length > 0 && !overwrite) {
    const overwriteLabel = vscode.l10n.t('Overwrite');
    const list = modified
      .map((f) => `${f.path} (${f.result.modifiedRegions.join(', ')})`)
      .join(', ');
    if (onSave) {
      // 保存のたびにダイアログを出さないよう、上書きせずに知らせる(答えを待たない)
      void vscode.window
        .showWarningMessage(
          vscode.l10n.t(
            'Code was not generated on save because generated regions have been edited by hand: {0}',
            list,
          ),
          overwriteLabel,
        )
        .then((answer) => {
          if (answer === overwriteLabel) {
            void regenerate();
          }
        });
      return false;
    }
    const answer = await vscode.window.showWarningMessage(
      vscode.l10n.t('Generated regions have been edited by hand: {0}. Overwrite them?', list),
      { modal: true },
      overwriteLabel,
    );
    if (answer !== overwriteLabel) {
      return false;
    }
  }
  const changed = files.filter((f) => existing.get(f.path) !== f.result.text);
  if (changed.length === 0) {
    if (!quiet) {
      void vscode.window.showInformationMessage(
        vscode.l10n.t('{0} is up to date.', files.map((f) => f.path).join(', ')),
      );
    }
    return true;
  }
  for (const file of changed) {
    await writeText(
      vscode.Uri.joinPath(directory, file.path),
      existing.get(file.path),
      file.result.text,
    );
  }
  const stubs = [...new Set(changed.flatMap((f) => f.result.addedStubs))];
  const details =
    stubs.length > 0 ? vscode.l10n.t(' (added handler stubs: {0})', stubs.join(', ')) : '';
  const summary = changed
    .map((f) =>
      existing.get(f.path) === undefined
        ? vscode.l10n.t('{0} (created)', f.path)
        : vscode.l10n.t('{0} (updated)', f.path),
    )
    .join(', ');
  if (quiet) {
    vscode.window.setStatusBarMessage(vscode.l10n.t('Generated: {0}{1}', summary, details), 5000);
    return true;
  }
  const open = vscode.l10n.t('Open');
  const answer = await vscode.window.showInformationMessage(
    vscode.l10n.t('Generated: {0}{1}', summary, details),
    open,
  );
  if (answer === open) {
    for (const file of changed) {
      await vscode.window.showTextDocument(vscode.Uri.joinPath(directory, file.path), {
        preview: false,
      });
    }
  }
  return true;
}

/** WorkspaceEdit で書く(開いているエディタにも反映され、Undo もできる)。書く前に保存していない変更が無かったファイルは保存する。 */
async function writeText(
  uri: vscode.Uri,
  existing: string | undefined,
  text: string,
): Promise<void> {
  const edit = new vscode.WorkspaceEdit();
  if (existing === undefined) {
    await vscode.workspace.fs.createDirectory(vscode.Uri.joinPath(uri, '..'));
    edit.createFile(uri, { contents: new TextEncoder().encode(text) });
    await vscode.workspace.applyEdit(edit);
    return;
  }
  const document = await vscode.workspace.openTextDocument(uri);
  const wasDirty = document.isDirty;
  // 改行コードは既存のファイルに合わせる
  const eol = document.eol === vscode.EndOfLine.CRLF ? '\r\n' : '\n';
  edit.replace(
    uri,
    new vscode.Range(document.positionAt(0), document.positionAt(document.getText().length)),
    text.replace(/\r?\n/g, eol),
  );
  await vscode.workspace.applyEdit(edit);
  if (!wasDirty) {
    await document.save();
  }
}
