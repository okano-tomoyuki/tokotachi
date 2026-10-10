/**
 * アクティビティバーのプロジェクトのビュー(docs/designer/project-spec.md §6)。プロジェクトごとにウィンドウを並べ、メインのウィンドウに ★ を付ける。
 * どのプロジェクトにも属さないウィンドウは「プロジェクトに属さないウィンドウ」にまとめる。
 */
import { DOCUMENT_EXTENSION, PROJECT_EXTENSION, sameWindowPath } from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { findProjects, windowPathIn, windowUri, type ProjectInfo } from './projects.ts';

type Node =
  | { readonly kind: 'project'; readonly project: ProjectInfo }
  | {
      readonly kind: 'window';
      readonly uri: vscode.Uri;
      readonly project: ProjectInfo | undefined;
      readonly main: boolean;
    }
  | { readonly kind: 'loose'; readonly windows: readonly vscode.Uri[] };

function fileName(uri: vscode.Uri): string {
  return uri.path.split('/').pop() ?? '';
}

export class ProjectsView implements vscode.TreeDataProvider<Node> {
  private readonly changed = new vscode.EventEmitter<undefined>();
  readonly onDidChangeTreeData = this.changed.event;

  refresh(): void {
    this.changed.fire(undefined);
  }

  getTreeItem(node: Node): vscode.TreeItem {
    switch (node.kind) {
      case 'project': {
        const item = new vscode.TreeItem(
          fileName(node.project.uri).replace(PROJECT_EXTENSION, ''),
          vscode.TreeItemCollapsibleState.Expanded,
        );
        item.iconPath = new vscode.ThemeIcon('project');
        item.description = vscode.workspace.asRelativePath(
          vscode.Uri.joinPath(node.project.uri, '..'),
        );
        item.contextValue = 'project';
        item.command = { command: 'vscode.open', title: '', arguments: [node.project.uri] };
        item.tooltip = node.project.doc ? undefined : vscode.l10n.t('The project file has errors');
        return item;
      }
      case 'window': {
        const item = new vscode.TreeItem(fileName(node.uri).replace(DOCUMENT_EXTENSION, ''));
        item.iconPath = new vscode.ThemeIcon(node.main ? 'star-full' : 'window');
        item.description = node.main ? vscode.l10n.t('main window') : undefined;
        item.resourceUri = node.uri;
        item.contextValue = node.project
          ? node.main
            ? 'window.main'
            : 'window.project'
          : 'window.loose';
        item.command = {
          command: 'vscode.openWith',
          title: '',
          arguments: [node.uri, 'tokotachiDesigner.designer'],
        };
        return item;
      }
      case 'loose': {
        const item = new vscode.TreeItem(
          vscode.l10n.t('Windows not in a project'),
          vscode.TreeItemCollapsibleState.Expanded,
        );
        item.iconPath = new vscode.ThemeIcon('folder');
        return item;
      }
    }
  }

  async getChildren(node?: Node): Promise<Node[]> {
    if (node?.kind === 'project') {
      const doc = node.project.doc;
      return (doc?.windows ?? []).map((path) => ({
        kind: 'window' as const,
        uri: windowUri(node.project.uri, path),
        project: node.project,
        main: doc?.main_window !== undefined && sameWindowPath(doc.main_window, path),
      }));
    }
    if (node?.kind === 'loose') {
      return node.windows.map((uri) => ({
        kind: 'window' as const,
        uri,
        project: undefined,
        main: false,
      }));
    }
    if (node) {
      return [];
    }
    const projects = await findProjects();
    const windows = await vscode.workspace.findFiles(
      `**/*${DOCUMENT_EXTENSION}`,
      '**/{node_modules,.git,.cache}/**',
    );
    const loose = windows
      .filter(
        (w) =>
          !projects.some((p) =>
            (p.doc?.windows ?? []).some((path) => sameWindowPath(path, windowPathIn(p.uri, w))),
          ),
      )
      .sort((a, b) => a.path.localeCompare(b.path));
    return [
      ...projects.map((project) => ({ kind: 'project' as const, project })),
      ...(loose.length > 0 ? [{ kind: 'loose' as const, windows: loose }] : []),
    ];
  }
}

/** ビューを作り、ファイルが変わったら描き直す。 */
export function registerProjectsView(): { view: ProjectsView; disposable: vscode.Disposable } {
  const view = new ProjectsView();
  const watcher = vscode.workspace.createFileSystemWatcher(
    `**/*{${PROJECT_EXTENSION},${DOCUMENT_EXTENSION}}`,
  );
  const refresh = (): void => {
    view.refresh();
  };
  return {
    view,
    disposable: vscode.Disposable.from(
      vscode.window.registerTreeDataProvider('tokotachiDesigner.projects', view),
      watcher,
      watcher.onDidCreate(refresh),
      watcher.onDidDelete(refresh),
      watcher.onDidChange(refresh),
    ),
  };
}
