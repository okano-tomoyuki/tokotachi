/**
 * 定義ファイルの検証の結果を問題パネルに出す(Bethany と同じ)。JSON 上の位置は core の locate でファイル上の範囲にする。
 */
import { DOCUMENT_EXTENSION, locate, parseDocument } from '@tokotachi-designer/core';
import * as vscode from 'vscode';

export function registerDiagnostics(): vscode.Disposable {
  const collection = vscode.languages.createDiagnosticCollection('tokotachiDesigner');
  const update = (document: vscode.TextDocument): void => {
    if (!document.uri.path.endsWith(DOCUMENT_EXTENSION)) {
      return;
    }
    const text = document.getText();
    const { diagnostics } = parseDocument(text);
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
