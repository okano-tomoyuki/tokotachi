/**
 * 新しいウィンドウの定義ファイルを作って開く(コマンド「新しいウィンドウ」)。
 * ファイルの名前(snake_case)を入力してもらい、クラスの名前はそこから PascalCase で作る(main_window → MainWindow)。
 */
import {
  DOCUMENT_EXTENSION,
  isValidIdentifier,
  PROJECT_EXTENSION,
  serializeDocument,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { addToNearestProject } from './projectCommands.ts';
import { addWindowToProject, findProjects } from './projects.ts';

/** snake_case から PascalCase(main_window → MainWindow)。 */
function className(name: string): string {
  return name
    .split('_')
    .filter((p) => p !== '')
    .map((p) => p.charAt(0).toUpperCase() + p.slice(1))
    .join('');
}

/** 新しいウィンドウの既定の内容(タイトルとラベル 1 つ)。 */
export function newWindowDocument(name: string): TtuiDocument {
  return {
    format_version: 1,
    window: {
      name: className(name),
      class: 'Window',
      properties: { width: 480, height: 320, title: className(name) },
      layout: {
        class: 'BoxLayout',
        properties: { margins: 12 },
        items: [
          {
            widget: {
              name: 'label_message',
              class: 'Label',
              properties: { text: 'Hello, Tokotachi' },
            },
          },
        ],
      },
    },
  };
}

async function exists(uri: vscode.Uri): Promise<boolean> {
  try {
    await vscode.workspace.fs.stat(uri);
    return true;
  } catch {
    return false;
  }
}

/**
 * 新しいウィンドウを作るフォルダと、加えるプロジェクト。project が 'nearest' なら、同じか上のフォルダのいちばん近いプロジェクトに加え、
 * undefined ならどのプロジェクトにも加えない。
 */
interface Destination {
  readonly folder: vscode.Uri;
  readonly project: vscode.Uri | 'nearest' | undefined;
}

/**
 * コマンドの引数から、作る場所を決める。引数は呼ばれた場所で違う: エクスプローラーはフォルダの URI、プロジェクトのビューの項目は
 * その項目、「新しいファイル」のメニューなどは別のオブジェクトか無し。URI・項目でなければ、プロジェクトを選んでもらう。
 * @returns 選ぶのをやめたら undefined
 */
async function destinationOf(arg: unknown): Promise<Destination | undefined> {
  if (arg instanceof vscode.Uri) {
    const isFolder = await vscode.workspace.fs.stat(arg).then(
      (s) => (s.type & vscode.FileType.Directory) !== 0,
      () => false,
    );
    return { folder: isFolder ? arg : vscode.Uri.joinPath(arg, '..'), project: 'nearest' };
  }
  const node = arg as { kind?: unknown; uri?: unknown; project?: { uri?: unknown } } | undefined;
  const projectUri = node?.project?.uri instanceof vscode.Uri ? node.project.uri : undefined;
  if (node?.kind === 'project' && projectUri) {
    return { folder: vscode.Uri.joinPath(projectUri, '..'), project: projectUri };
  }
  if (node?.kind === 'window' && node.uri instanceof vscode.Uri) {
    return { folder: vscode.Uri.joinPath(node.uri, '..'), project: projectUri };
  }
  const root = vscode.workspace.workspaceFolders?.[0]?.uri;
  const projects = (await findProjects()).filter((p) => p.doc !== undefined);
  const first = projects[0];
  if (projects.length === 0 || !first) {
    return root ? { folder: root, project: undefined } : undefined;
  }
  if (projects.length === 1) {
    return { folder: vscode.Uri.joinPath(first.uri, '..'), project: first.uri };
  }
  const picked = await vscode.window.showQuickPick(
    [
      ...projects.map((p) => ({
        label: (p.uri.path.split('/').pop() ?? '').replace(PROJECT_EXTENSION, ''),
        description: vscode.workspace.asRelativePath(vscode.Uri.joinPath(p.uri, '..')),
        destination: { folder: vscode.Uri.joinPath(p.uri, '..'), project: p.uri },
      })),
      ...(root
        ? [
            {
              label: vscode.l10n.t('Not in a project'),
              description: vscode.workspace.asRelativePath(root),
              destination: { folder: root, project: undefined } as Destination,
            },
          ]
        : []),
    ],
    { title: vscode.l10n.t('Add the new window to which project?') },
  );
  return picked?.destination;
}

export async function newWindow(arg?: unknown): Promise<void> {
  if (!vscode.workspace.workspaceFolders?.length && !(arg instanceof vscode.Uri)) {
    void vscode.window.showErrorMessage(vscode.l10n.t('Open a folder first.'));
    return;
  }
  const destination = await destinationOf(arg);
  if (!destination) {
    return;
  }
  const base = destination.folder;
  let initial = 'main_window';
  for (let i = 2; await exists(vscode.Uri.joinPath(base, `${initial}${DOCUMENT_EXTENSION}`)); i++) {
    initial = `window${String(i)}`;
  }
  const name = await vscode.window.showInputBox({
    title: vscode.l10n.t('New Window'),
    prompt: vscode.l10n.t(
      'File name of the window definition (the class name is made from it: main_window → MainWindow)',
    ),
    value: initial,
    validateInput: async (value) => {
      if (isValidIdentifier(value) !== null || !/^[a-z]/.test(value)) {
        return vscode.l10n.t('Use lowercase letters, digits and _ (e.g. main_window)');
      }
      if (await exists(vscode.Uri.joinPath(base, `${value}${DOCUMENT_EXTENSION}`))) {
        return vscode.l10n.t('The file already exists');
      }
      return undefined;
    },
  });
  if (name === undefined) {
    return;
  }
  const uri = vscode.Uri.joinPath(base, `${name}${DOCUMENT_EXTENSION}`);
  await vscode.workspace.fs.writeFile(
    uri,
    new TextEncoder().encode(serializeDocument(newWindowDocument(name))),
  );
  if (destination.project === 'nearest') {
    // 同じフォルダか上のフォルダにプロジェクトがあれば、そこに加える
    await addToNearestProject(uri);
  } else if (destination.project) {
    await addWindowToProject(destination.project, uri);
  }
  await vscode.commands.executeCommand('vscode.openWith', uri, 'tokotachiDesigner.designer');
}
