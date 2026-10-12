/**
 * プロジェクトのコマンド(docs/designer/project-spec.md §6): 新しいプロジェクト・プロジェクトのコード生成・メインのウィンドウに設定・
 * プロジェクトに加える。
 */
import {
  DOCUMENT_EXTENSION,
  isJapanese,
  PROJECT_EXTENSION,
  serializeDocument,
  serializeProject,
  type TtprojDocument,
  type ProjectTheme,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { generateProjectCode } from './generateCode.ts';
import { newWindowDocument } from './newWindow.ts';
import { addWindowToProject, findProjects, updateProject, windowPathIn } from './projects.ts';

async function exists(uri: vscode.Uri): Promise<boolean> {
  try {
    await vscode.workspace.fs.stat(uri);
    return true;
  } catch {
    return false;
  }
}

/**
 * 新しいプロジェクト: プロジェクトファイル・メインのウィンドウ(main_window.ttui.json)を作り、ウィンドウ・main.cpp・CMakeLists.txt を生成して開く。
 */
export async function newProject(folder?: vscode.Uri): Promise<void> {
  let base = folder ?? vscode.workspace.workspaceFolders?.[0]?.uri;
  if (!base) {
    void vscode.window.showErrorMessage(vscode.l10n.t('Open a folder first.'));
    return;
  }
  const chooseFolder = vscode.l10n.t('Choose Folder...');
  const name = await vscode.window.showInputBox({
    title: vscode.l10n.t('New Project'),
    prompt: vscode.l10n.t(
      'Project name (also used as the CMake target name). It is created in {0}',
      vscode.workspace.asRelativePath(base),
    ),
    value: 'my_app',
    validateInput: async (value) => {
      if (!/^[A-Za-z][A-Za-z0-9_-]*$/.test(value)) {
        return vscode.l10n.t('Use letters, digits, _ and - (e.g. my_app)');
      }
      if (
        await exists(
          vscode.Uri.joinPath(base ?? vscode.Uri.file('/'), `${value}${PROJECT_EXTENSION}`),
        )
      ) {
        return vscode.l10n.t('The file already exists');
      }
      return undefined;
    },
  });
  if (name === undefined) {
    return;
  }
  if (!folder) {
    // 作る場所: ワークスペースの先頭のフォルダか、選んだフォルダ
    const here = vscode.workspace.asRelativePath(base);
    const picked = await vscode.window.showQuickPick([here, chooseFolder], {
      title: vscode.l10n.t('Where to create the project?'),
    });
    if (picked === undefined) {
      return;
    }
    if (picked === chooseFolder) {
      const selected = await vscode.window.showOpenDialog({
        canSelectFolders: true,
        canSelectFiles: false,
        defaultUri: base,
      });
      if (!selected?.[0]) {
        return;
      }
      base = selected[0];
    }
  }
  const projectUri = vscode.Uri.joinPath(base, `${name}${PROJECT_EXTENSION}`);
  const windowUri = vscode.Uri.joinPath(base, `main_window${DOCUMENT_EXTENSION}`);
  if (!(await exists(windowUri))) {
    await vscode.workspace.fs.writeFile(
      windowUri,
      new TextEncoder().encode(serializeDocument(newWindowDocument('main_window'))),
    );
  }
  const project: TtprojDocument = {
    format_version: 1,
    codegen: { comment_locale: isJapanese(vscode.env.language) ? 'ja' : 'en' },
    main_window: `main_window${DOCUMENT_EXTENSION}`,
    windows: [`main_window${DOCUMENT_EXTENSION}`],
  };
  await vscode.workspace.fs.writeFile(
    projectUri,
    new TextEncoder().encode(serializeProject(project)),
  );
  await generateProjectCode(projectUri, { quiet: true }, true);
  await vscode.commands.executeCommand('vscode.openWith', windowUri, 'tokotachiDesigner.designer');
  void vscode.window.showInformationMessage(
    vscode.l10n.t(
      'Created the project {0}. Build it with CMake (find_package(tokotachi)): main.cpp and CMakeLists.txt are generated next to the project file.',
      name,
    ),
  );
}

/** メインのウィンドウに設定する。 */
export async function setMainWindow(window: vscode.Uri): Promise<void> {
  for (const p of await findProjects()) {
    const path = windowPathIn(p.uri, window);
    if (p.doc?.windows?.includes(path)) {
      await updateProject(p.uri, (doc) => ({ ...doc, main_window: path }));
      return;
    }
  }
}

/** プロジェクトに加える(プロジェクトが複数あれば選んでもらう)。 */
export async function addToProject(window: vscode.Uri): Promise<void> {
  const projects = (await findProjects()).filter((p) => p.doc !== undefined);
  if (projects.length === 0) {
    void vscode.window.showWarningMessage(vscode.l10n.t('There is no project. Create one first.'));
    return;
  }
  const project =
    projects.length === 1
      ? projects[0]
      : (
          await vscode.window.showQuickPick(
            projects.map((p) => ({ label: vscode.workspace.asRelativePath(p.uri), p })),
            { title: vscode.l10n.t('Add to which project?') },
          )
        )?.p;
  if (project) {
    await addWindowToProject(project.uri, window);
  }
}

/** 新しいウィンドウを、同じフォルダか上のフォルダのプロジェクトに加える(あれば)。 */
export async function addToNearestProject(window: vscode.Uri): Promise<void> {
  const candidates = (await findProjects())
    .filter(
      (p) =>
        p.doc !== undefined &&
        window.path.startsWith(p.uri.path.slice(0, p.uri.path.lastIndexOf('/') + 1)),
    )
    .sort((a, b) => b.uri.path.length - a.uri.path.length);
  const nearest = candidates[0];
  if (nearest) {
    await addWindowToProject(nearest.uri, window);
  }
}

/** プロジェクトのテーマの配色を選んでもらう(project-spec.md §4.1)。 */
export async function chooseColorScheme(project: vscode.Uri): Promise<void> {
  const current = (await findProjects()).find((p) => p.uri.toString() === project.toString())?.doc
    ?.theme?.color_scheme;
  const items: (vscode.QuickPickItem & { value: NonNullable<ProjectTheme['color_scheme']> })[] = [
    { label: 'SYSTEM', value: 'SYSTEM', description: vscode.l10n.t('Follow the OS setting') },
    { label: 'LIGHT', value: 'LIGHT', description: vscode.l10n.t('Always light') },
    { label: 'DARK', value: 'DARK', description: vscode.l10n.t('Always dark') },
  ];
  const picked = await vscode.window.showQuickPick(
    items.map((i) => ({ ...i, picked: i.value === (current ?? 'SYSTEM') })),
    { title: vscode.l10n.t('Color scheme of the application') },
  );
  if (!picked) {
    return;
  }
  await updateProject(project, (doc) => {
    // SYSTEM は既定なので書かない
    const theme: ProjectTheme = {
      ...(picked.value !== 'SYSTEM' ? { color_scheme: picked.value } : {}),
      ...(doc.theme?.override !== undefined ? { override: doc.theme.override } : {}),
      ...(doc.theme?.presets !== undefined ? { presets: doc.theme.presets } : {}),
    };
    const rest = { ...doc };
    Reflect.deleteProperty(rest, 'theme');
    return Object.keys(theme).length === 0 ? rest : { ...rest, theme };
  });
}
