/**
 * 定義ファイルの検証の結果を問題パネルに出す(Bethany と同じ)。JSON 上の位置は core の locate でファイル上の範囲にする。
 */
import { existsSync } from 'node:fs';
import {
  componentReferences,
  DOCUMENT_EXTENSION,
  imageReferences,
  locate,
  parseDocument,
  parseProject,
  PROJECT_EXTENSION,
  themePresetReferences,
  withCustomClasses,
  type Diagnostic,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { loadComponents } from './components.ts';
import { customClassesOf, isHeaderFile, readProjectCustomClasses } from './customWidgets.ts';
import { projectOf } from './projects.ts';

export function registerDiagnostics(): vscode.Disposable {
  const collection = vscode.languages.createDiagnosticCollection('tokotachiDesigner');
  const update = async (document: vscode.TextDocument): Promise<void> => {
    const isProject = document.uri.path.endsWith(PROJECT_EXTENSION);
    if (!isProject && !document.uri.path.endsWith(DOCUMENT_EXTENSION)) {
      return;
    }
    const text = document.getText();
    // 調べている間に変わったか(変わったら、新しい内容で調べ直す)
    const stale = (): boolean => document.isClosed || document.getText() !== text;
    // 自作部品(docs/adr/0044)をカタログに足して検証する
    const classes = isProject ? {} : await customClassesOf(document.uri);
    if (stale()) {
      return; // 調べている間に変わった: 新しい内容で調べ直す
    }
    const parsed = isProject ? undefined : withCustomClasses(classes, () => parseDocument(text));
    const project = isProject ? parseProject(text) : undefined;
    const diagnostics: Diagnostic[] = [...(parsed?.diagnostics ?? project?.diagnostics ?? [])];
    // プロジェクト: 自作部品のヘッダが有るか・読めないクラス・プロパティ
    if (project?.project) {
      const custom = await readProjectCustomClasses(document.uri, project.project);
      if (stale()) {
        return;
      }
      const headers = project.project.custom_widgets ?? [];
      for (const i of custom.missing) {
        diagnostics.push({
          severity: 'error',
          code: 'schema',
          message: vscode.l10n.t('The header "{0}" is not found', headers[i] ?? ''),
          path: ['custom_widgets', i],
        });
      }
      for (const p of custom.problems) {
        diagnostics.push({
          severity: 'warning',
          code: 'schema',
          message: `${p.header}:${String(p.line)}: ${p.message}`,
          path: ['custom_widgets', headers.indexOf(p.header)],
        });
      }
    }
    // 画像のファイルがあるか(無ければコードを生成できない)
    if (parsed?.document && document.uri.scheme === 'file') {
      const directory = vscode.Uri.joinPath(document.uri, '..');
      for (const ref of imageReferences(parsed.document)) {
        if (!existsSync(vscode.Uri.joinPath(directory, ref.path).fsPath)) {
          diagnostics.push({
            severity: 'error',
            code: 'missing-image',
            message: vscode.l10n.t('The image file "{0}" is not found', ref.path),
            path: ref.at,
          });
        }
      }
    }
    // 置いた部品ファイル(docs/adr/0043)が有るか・根が Widget か・循環していないか
    if (parsed?.document && componentReferences(parsed.document).length > 0) {
      const { diagnostics: problems } = await loadComponents(
        document.uri,
        parsed.document,
        classes,
      );
      if (stale()) {
        return; // 調べている間に変わった: 新しい内容で調べ直す
      }
      diagnostics.push(...problems);
    }
    // プロジェクトに登録したテーマの参照(プロジェクトに属するウィンドウだけが使え、登録した名前でなければならない)
    const references = parsed?.document ? themePresetReferences(parsed.document) : [];
    if (references.length > 0) {
      const project = await projectOf(document.uri);
      const presets = project?.project.doc.theme?.presets ?? {};
      for (const ref of references) {
        if (!project || !Object.hasOwn(presets, ref.name)) {
          diagnostics.push({
            severity: 'error',
            code: 'unknown-theme',
            message: project
              ? vscode.l10n.t('The theme "{0}" is not registered in the project', ref.name)
              : vscode.l10n.t(
                  'The theme "{0}" can be used only in a window of a project',
                  ref.name,
                ),
            path: ref.at,
          });
        }
      }
      if (stale()) {
        return; // 調べている間に変わった: 新しい内容で調べ直す
      }
    }
    collection.set(
      document.uri,
      diagnostics.map((d) => {
        const range = locate(text, d.path);
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(document.positionAt(range.start), document.positionAt(range.end)),
          d.message,
          d.severity === 'error'
            ? vscode.DiagnosticSeverity.Error
            : vscode.DiagnosticSeverity.Warning,
        );
        diagnostic.code = d.code;
        diagnostic.source = 'Tokotachi Designer';
        return diagnostic;
      }),
    );
  };
  const run = (document: vscode.TextDocument): void => {
    void update(document);
  };
  for (const document of vscode.workspace.textDocuments) {
    run(document);
  }
  return vscode.Disposable.from(
    collection,
    vscode.workspace.onDidOpenTextDocument(run),
    vscode.workspace.onDidChangeTextDocument((e) => {
      run(e.document);
      // プロジェクトのテーマ・部品ファイル・自作部品のヘッダが変わったら、開いているウィンドウ・プロジェクトを調べ直す
      if (
        e.document.uri.path.endsWith(PROJECT_EXTENSION) ||
        e.document.uri.path.endsWith(DOCUMENT_EXTENSION) ||
        isHeaderFile(e.document.uri)
      ) {
        vscode.workspace.textDocuments
          .filter(
            (d) =>
              d !== e.document &&
              (d.uri.path.endsWith(DOCUMENT_EXTENSION) || d.uri.path.endsWith(PROJECT_EXTENSION)),
          )
          .forEach(run);
      }
    }),
    vscode.workspace.onDidCloseTextDocument((d) => {
      collection.delete(d.uri);
    }),
  );
}
