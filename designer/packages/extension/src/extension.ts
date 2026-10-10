/**
 * Tokotachi Designer(docs/designer/editor-design.md)。
 */
import * as vscode from 'vscode';
import { DesignerEditorProvider } from './designerEditorProvider.ts';
import { registerDiagnostics } from './diagnostics.ts';
import { generateWindowCode, registerGenerateOnSave } from './generateCode.ts';
import { newWindow } from './newWindow.ts';

/** コマンドの対象の定義ファイル(エクスプローラーの右クリック、無ければ開いているもの)。 */
async function targetDocument(uri?: vscode.Uri): Promise<vscode.TextDocument | undefined> {
  if (uri) {
    return vscode.workspace.openTextDocument(uri);
  }
  const tab = vscode.window.tabGroups.activeTabGroup.activeTab?.input;
  if (tab instanceof vscode.TabInputCustom || tab instanceof vscode.TabInputText) {
    return vscode.workspace.openTextDocument(tab.uri);
  }
  return undefined;
}

export function activate(context: vscode.ExtensionContext): void {
  context.subscriptions.push(
    DesignerEditorProvider.register(context),
    registerDiagnostics(),
    registerGenerateOnSave(),
    vscode.commands.registerCommand('tokotachiDesigner.newWindow', (folder?: vscode.Uri) =>
      newWindow(folder),
    ),
    vscode.commands.registerCommand('tokotachiDesigner.generateCode', async (uri?: vscode.Uri) => {
      const document = await targetDocument(uri);
      if (document) {
        await generateWindowCode(document, { quiet: false });
      }
    }),
    vscode.commands.registerCommand('tokotachiDesigner.openAsText', async (uri?: vscode.Uri) => {
      const document = await targetDocument(uri);
      if (document) {
        await vscode.commands.executeCommand('vscode.openWith', document.uri, 'default');
      }
    }),
  );
}

export function deactivate(): void {
  // 何もしない
}
