/**
 * ワークスペースのプロジェクトファイル(*.ttproj.json。docs/designer/project-spec.md)。
 */
import {
  dirname,
  parseProject,
  PROJECT_EXTENSION,
  relativePath,
  sameWindowPath,
  serializeProject,
  type TtprojDocument,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';

export interface ProjectInfo {
  readonly uri: vscode.Uri;
  /** 読めなければ undefined。 */
  readonly doc: TtprojDocument | undefined;
}

/** 開いている(保存していない変更を含む)内容、無ければファイルの内容。 */
export async function readText(uri: vscode.Uri): Promise<string | undefined> {
  const opened = vscode.workspace.textDocuments.find((d) => d.uri.toString() === uri.toString());
  if (opened) {
    return opened.getText();
  }
  try {
    return new TextDecoder().decode(await vscode.workspace.fs.readFile(uri));
  } catch {
    return undefined;
  }
}

export async function findProjects(): Promise<ProjectInfo[]> {
  const uris = await vscode.workspace.findFiles(
    `**/*${PROJECT_EXTENSION}`,
    '**/{node_modules,.git}/**',
  );
  const out: ProjectInfo[] = [];
  for (const uri of uris.sort((a, b) => a.path.localeCompare(b.path))) {
    const text = await readText(uri);
    out.push({ uri, doc: text === undefined ? undefined : parseProject(text).project });
  }
  return out;
}

/** プロジェクトのフォルダからの、ウィンドウの定義ファイルの相対パス。 */
export function windowPathIn(project: vscode.Uri, window: vscode.Uri): string {
  return relativePath(dirname(project.path), window.path);
}

/** プロジェクトのウィンドウの定義ファイルの場所。 */
export function windowUri(project: vscode.Uri, path: string): vscode.Uri {
  return vscode.Uri.joinPath(project, '..', path);
}

/** ウィンドウが属する(最初に見つかった)プロジェクトと、その中でのウィンドウのパス。 */
export async function projectOf(
  window: vscode.Uri,
): Promise<{ project: ProjectInfo & { doc: TtprojDocument }; path: string } | undefined> {
  for (const p of await findProjects()) {
    if (!p.doc) {
      continue;
    }
    const path = windowPathIn(p.uri, window);
    if ((p.doc.windows ?? []).some((w) => sameWindowPath(w, path))) {
      return { project: { uri: p.uri, doc: p.doc }, path };
    }
  }
  return undefined;
}

/** プロジェクトファイルを書き換える(開いているエディタにも反映され、Undo できる。書く前に保存していなかったものは保存する)。 */
export async function updateProject(
  uri: vscode.Uri,
  change: (doc: TtprojDocument) => TtprojDocument,
): Promise<boolean> {
  const text = await readText(uri);
  const doc = text === undefined ? undefined : parseProject(text).project;
  if (!doc || text === undefined) {
    void vscode.window.showErrorMessage(
      vscode.l10n.t('The project file cannot be read: {0}', uri.path),
    );
    return false;
  }
  const document = await vscode.workspace.openTextDocument(uri);
  const wasDirty = document.isDirty;
  const edit = new vscode.WorkspaceEdit();
  edit.replace(
    uri,
    new vscode.Range(document.positionAt(0), document.positionAt(document.getText().length)),
    serializeProject(change(doc)),
  );
  const ok = await vscode.workspace.applyEdit(edit);
  if (ok && !wasDirty) {
    await document.save();
  }
  return ok;
}

/** ウィンドウをプロジェクトに加える(最初のウィンドウはメインのウィンドウにもする)。 */
export async function addWindowToProject(
  project: vscode.Uri,
  window: vscode.Uri,
): Promise<boolean> {
  const path = windowPathIn(project, window);
  return updateProject(project, (doc) => {
    if ((doc.windows ?? []).some((w) => sameWindowPath(w, path))) {
      return doc;
    }
    return {
      ...doc,
      windows: [...(doc.windows ?? []), path],
      ...(doc.main_window === undefined ? { main_window: path } : {}),
    };
  });
}
