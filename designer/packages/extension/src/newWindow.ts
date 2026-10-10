/**
 * 新しいウィンドウの定義ファイルを作って開く(コマンド「新しいウィンドウ」)。
 * ファイルの名前(snake_case)を入力してもらい、クラスの名前はそこから PascalCase で作る(main_window → MainWindow)。
 */
import {
  DOCUMENT_EXTENSION,
  isValidIdentifier,
  serializeDocument,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';

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
              name: 'message_label',
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

export async function newWindow(folder?: vscode.Uri): Promise<void> {
  const base = folder ?? vscode.workspace.workspaceFolders?.[0]?.uri;
  if (!base) {
    void vscode.window.showErrorMessage(vscode.l10n.t('Open a folder first.'));
    return;
  }
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
  await vscode.commands.executeCommand('vscode.openWith', uri, 'tokotachiDesigner.designer');
}
