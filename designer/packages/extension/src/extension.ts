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
  /** エクスプローラー・ビューの項目から呼ばれたときの対象の URI(それ以外のオブジェクトなら undefined)。 */
  const uriOf = (arg: unknown): vscode.Uri | undefined => {
    if (arg instanceof vscode.Uri) {
      return arg;
    }
    const node = arg as { uri?: unknown; project?: { uri?: unknown } } | undefined;
    const uri = node?.uri ?? node?.project?.uri;
    return uri instanceof vscode.Uri ? uri : undefined;
  };
  context.subscriptions.push(
    projects.disposable,
    vscode.commands.registerCommand('tokotachiDesigner.newProject', (arg?: unknown) =>
      newProject(arg instanceof vscode.Uri ? arg : undefined),
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
    // 引数は呼ばれた場所で違う(フォルダの URI・ビューの項目・それ以外のオブジェクト)ので、newWindow が見分ける
    vscode.commands.registerCommand('tokotachiDesigner.newWindow', (arg?: unknown) =>
      newWindow(arg),
    ),
    vscode.commands.registerCommand('tokotachiDesigner.generateCode', async (arg?: unknown) => {
      const document = await targetDocument(uriOf(arg));
      if (document) {
        await generateWindowCode(document, { quiet: false });
      }
    }),
    vscode.commands.registerCommand('tokotachiDesigner.openAsText', async (arg?: unknown) => {
      const document = await targetDocument(uriOf(arg));
      if (document) {
        await vscode.commands.executeCommand('vscode.openWith', document.uri, 'default');
      }
    }),
  );
}

export function deactivate(): void {
  // 何もしない
}
