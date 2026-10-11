/**
 * Tokotachi Designer(docs/designer/editor-design.md)。
 */
import * as vscode from 'vscode';
import { DesignerEditorProvider } from './designerEditorProvider.ts';
import { registerDiagnostics } from './diagnostics.ts';
import { generateProjectCode, generateWindowCode, registerGenerateOnSave } from './generateCode.ts';
import { addToProject, chooseColorScheme, newProject, setMainWindow } from './projectCommands.ts';
import { ProjectEditorProvider } from './projectEditorProvider.ts';
import { registerProjectsView } from './projectsView.ts';
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
  const projects = registerProjectsView();
  /** ビューの項目から呼ばれたときの対象の URI。 */
  const uriOf = (arg: unknown): vscode.Uri | undefined =>
    arg instanceof vscode.Uri
      ? arg
      : ((arg as { uri?: vscode.Uri; project?: { uri: vscode.Uri } } | undefined)?.uri ??
        (arg as { project?: { uri: vscode.Uri } } | undefined)?.project?.uri);
  context.subscriptions.push(
    projects.disposable,
    vscode.commands.registerCommand('tokotachiDesigner.newProject', (folder?: vscode.Uri) =>
      newProject(folder instanceof vscode.Uri ? folder : undefined),
    ),
    vscode.commands.registerCommand('tokotachiDesigner.generateProject', async (arg?: unknown) => {
      const uri = uriOf(arg) ?? vscode.window.activeTextEditor?.document.uri;
      if (uri?.path.endsWith('.ttproj.json')) {
        await generateProjectCode(uri, { quiet: false }, true);
      }
    }),
    vscode.commands.registerCommand('tokotachiDesigner.setMainWindow', async (arg?: unknown) => {
      const uri = uriOf(arg);
      if (uri) {
        await setMainWindow(uri);
      }
    }),
    vscode.commands.registerCommand(
      'tokotachiDesigner.chooseColorScheme',
      async (arg?: unknown) => {
        const uri = uriOf(arg) ?? vscode.window.activeTextEditor?.document.uri;
        if (uri?.path.endsWith('.ttproj.json')) {
          await chooseColorScheme(uri);
        }
      },
    ),
    vscode.commands.registerCommand('tokotachiDesigner.addToProject', async (arg?: unknown) => {
      const uri = uriOf(arg);
      if (uri) {
        await addToProject(uri);
      }
    }),
    vscode.commands.registerCommand('tokotachiDesigner.refreshProjects', () => {
      projects.view.refresh();
    }),
    DesignerEditorProvider.register(context),
    ProjectEditorProvider.register(context),
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
