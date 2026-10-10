/**
 * ハンドラの定義へ移動する(docs/designer/editor-design.md §6)。デザイナーのダブルクリック・ハンドラの欄のボタンから呼ばれる。
 * C++Builder と同じく、先にコードを生成して(足りない雛形を足して)から、定義の本体にカーソルを置いて開く。
 */
import { findHandler } from '@tokotachi-designer/codegen';
import * as vscode from 'vscode';
import { generateWindowCode } from './generateCode.ts';

export async function goToHandler(document: vscode.TextDocument, handler: string): Promise<void> {
  const result = await generateWindowCode(document, { quiet: true });
  if (!result) {
    return;
  }
  const uri = vscode.Uri.joinPath(result.directory, result.target.source);
  const code = await vscode.workspace.openTextDocument(uri);
  const position = findHandler(code.getText(), result.target.className, handler);
  if (!position) {
    void vscode.window.showWarningMessage(
      vscode.l10n.t('The handler {0} was not found in {1}.', handler, result.target.source),
    );
    return;
  }
  const at = new vscode.Position(position.line, position.character);
  // 既にどこかのエディタで開いていればそこで、無ければデザイナーと同じグループで開く
  const visible = vscode.window.visibleTextEditors.find(
    (e) => e.document.uri.toString() === uri.toString(),
  );
  await vscode.window.showTextDocument(code, {
    viewColumn: visible?.viewColumn ?? vscode.ViewColumn.Active,
    selection: new vscode.Range(at, at),
    preview: false,
  });
}
