/**
 * 定義ファイルの検証の結果を問題パネルに出す(Bethany と同じ)。JSON 上の位置は core の locate でファイル上の範囲にする。
 */
import { existsSync } from 'node:fs';
import {
  DOCUMENT_EXTENSION,
  imageReferences,
  locate,
  parseDocument,
  parseProject,
  PROJECT_EXTENSION,
  type Diagnostic,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';

export function registerDiagnostics(): vscode.Disposable {
  const collection = vscode.languages.createDiagnosticCollection('tokotachiDesigner');
  const update = (document: vscode.TextDocument): void => {
    const isProject = document.uri.path.endsWith(PROJECT_EXTENSION);
    if (!isProject && !document.uri.path.endsWith(DOCUMENT_EXTENSION)) {
      return;
    }
    const text = document.getText();
    const parsed = isProject ? undefined : parseDocument(text);
    const diagnostics: Diagnostic[] = [...(parsed?.diagnostics ?? parseProject(text).diagnostics)];
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
  for (const document of vscode.workspace.textDocuments) {
    update(document);
  }
  return vscode.Disposable.from(
    collection,
    vscode.workspace.onDidOpenTextDocument(update),
    vscode.workspace.onDidChangeTextDocument((e) => {
      update(e.document);
    }),
    vscode.workspace.onDidCloseTextDocument((d) => {
      collection.delete(d.uri);
    }),
  );
}
