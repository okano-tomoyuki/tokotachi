/**
 * アクティビティバーのプロジェクトのビュー(docs/designer/project-spec.md §6)。プロジェクトごとにウィンドウを並べ、メインのウィンドウに ★ を付ける。
 * どのプロジェクトにも属さないウィンドウは「プロジェクトに属さないウィンドウ」にまとめる。
 * プロジェクトの下には起動部分(main.cpp など)、ウィンドウの下にはヘッダ・ソースの生成先を出す(まだ無ければ「未生成」)。
 */
import { cppOutputPaths, projectOutputPaths, windowCppOptions } from '@tokotachi-designer/codegen';
import {
  DOCUMENT_EXTENSION,
  isComponentDocument,
  parseDocument,
  PROJECT_EXTENSION,
  sameWindowPath,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { findProjects, readText, windowPathIn, windowUri, type ProjectInfo } from './projects.ts';
import { cppOptions } from './settings.ts';

/** ファイルの変更をまとめて描き直すまでの時間(ms) */
const REFRESH_DELAY_MS = 300;

type Node =
  | { readonly kind: 'project'; readonly project: ProjectInfo }
  | {
      readonly kind: 'window';
      readonly uri: vscode.Uri;
      readonly project: ProjectInfo | undefined;
      readonly main: boolean;
      /** 部品ファイル(根が Widget。docs/adr/0043)か。 */
      readonly component: boolean;
    }
  | { readonly kind: 'loose'; readonly windows: readonly vscode.Uri[] }
  | { readonly kind: 'generated'; readonly uri: vscode.Uri; readonly missing: boolean };

function fileName(uri: vscode.Uri): string {
  return uri.path.split('/').pop() ?? '';
}

/** 部品ファイル(根が Widget)か。読めなければ false。 */
async function isComponentFile(uri: vscode.Uri): Promise<boolean> {
  const text = await readText(uri);
  if (text === undefined) {
    return false;
  }
  const parsed = parseDocument(text);
  return parsed.document !== undefined && isComponentDocument(parsed.document);
}

async function exists(uri: vscode.Uri): Promise<boolean> {
  try {
    await vscode.workspace.fs.stat(uri);
    return true;
  } catch {
    return false;
  }
}

async function generatedNodes(uris: readonly vscode.Uri[]): Promise<Node[]> {
  return Promise.all(
    uris.map(async (uri) => ({ kind: 'generated' as const, uri, missing: !(await exists(uri)) })),
  );
}

/** ウィンドウのヘッダ・ソースの生成先(コード生成と同じく、プロジェクトに属せばプロジェクトの設定、属さなければ拡張の設定で決める)。 */
function windowOutputs(uri: vscode.Uri, project: ProjectInfo | undefined): vscode.Uri[] {
  const options = project?.doc
    ? windowCppOptions(project.doc, windowPathIn(project.uri, uri))
    : cppOptions(uri);
  const { header, source } = cppOutputPaths(fileName(uri), options);
  return [header, source].map((path) => vscode.Uri.joinPath(uri, '..', path));
}

export class ProjectsView implements vscode.TreeDataProvider<Node>, vscode.Disposable {
  private readonly changed = new vscode.EventEmitter<undefined>();
  readonly onDidChangeTreeData = this.changed.event;
  private timer: ReturnType<typeof setTimeout> | undefined;

  /** 描き直す(続けて呼ばれたらまとめる)。 */
  refresh(delay = 0): void {
    clearTimeout(this.timer);
    this.timer = setTimeout(() => {
      this.changed.fire(undefined);
    }, delay);
  }

  dispose(): void {
    clearTimeout(this.timer);
    this.changed.dispose();
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
        const item = new vscode.TreeItem(
          fileName(node.uri).replace(DOCUMENT_EXTENSION, ''),
          vscode.TreeItemCollapsibleState.Collapsed,
        );
        item.iconPath = new vscode.ThemeIcon(
          node.main ? 'star-full' : node.component ? 'symbol-module' : 'window',
        );
        item.description = node.main
          ? vscode.l10n.t('main window')
          : node.component
            ? vscode.l10n.t('component')
            : undefined;
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
      case 'generated': {
        const item = new vscode.TreeItem(fileName(node.uri));
        // resourceUri と ThemeIcon.File で、ファイルアイコンのテーマの拡張子ごとのアイコンにする
        item.resourceUri = node.uri;
        item.iconPath = vscode.ThemeIcon.File;
        item.tooltip = vscode.workspace.asRelativePath(node.uri);
        item.contextValue = node.missing ? 'generated.missing' : 'generated';
        if (node.missing) {
          item.description = vscode.l10n.t('Not generated yet');
          return item;
        }
        item.command = { command: 'vscode.open', title: '', arguments: [node.uri] };
        return item;
      }
    }
  }

  async getChildren(node?: Node): Promise<Node[]> {
    if (node?.kind === 'project') {
      const doc = node.project.doc;
      const outputs = doc
        ? projectOutputPaths(doc).map((path) => vscode.Uri.joinPath(node.project.uri, '..', path))
        : [];
      return [
        ...(await generatedNodes(outputs)),
        ...(await Promise.all(
          (doc?.windows ?? []).map(async (path) => {
            const uri = windowUri(node.project.uri, path);
            return {
              kind: 'window' as const,
              uri,
              project: node.project,
              main: doc?.main_window !== undefined && sameWindowPath(doc.main_window, path),
              component: await isComponentFile(uri),
            };
          }),
        )),
      ];
    }
    if (node?.kind === 'window') {
      return generatedNodes(windowOutputs(node.uri, node.project));
    }
    if (node?.kind === 'loose') {
      return Promise.all(
        node.windows.map(async (uri) => ({
          kind: 'window' as const,
          uri,
          project: undefined,
          main: false,
          component: await isComponentFile(uri),
        })),
      );
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
  // 生成したファイルができた・消えたら「未生成」を付け直す
  const generated = vscode.workspace.createFileSystemWatcher(
    '**/{*.hpp,*.cpp,CMakeLists.txt}',
    false,
    true,
    false,
  );
  const refresh = (): void => {
    view.refresh(REFRESH_DELAY_MS);
  };
  return {
    view,
    disposable: vscode.Disposable.from(
      vscode.window.registerTreeDataProvider('tokotachiDesigner.projects', view),
      view,
      watcher,
      watcher.onDidCreate(refresh),
      watcher.onDidDelete(refresh),
      watcher.onDidChange(refresh),
      generated,
      generated.onDidCreate(refresh),
      generated.onDidDelete(refresh),
      // 保存する前のプロジェクトファイルの変更(出力先の設定など)も反映する
      vscode.workspace.onDidChangeTextDocument((e) => {
        if (e.document.uri.path.endsWith(PROJECT_EXTENSION)) {
          refresh();
        }
      }),
      vscode.workspace.onDidChangeConfiguration((e) => {
        if (e.affectsConfiguration('tokotachiDesigner.codegen')) {
          refresh();
        }
      }),
    ),
  };
}
