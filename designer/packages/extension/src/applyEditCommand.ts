import {
  applyCommand,
  minimalTextEdit,
  parseDocument,
  serializeDocument,
  withCustomClasses,
  type EditCommand,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { customClassesOf } from './customWidgets.ts';

export type ApplyResult = { readonly ok: true } | { readonly ok: false; readonly error: string };

/**
 * 編集コマンドを TextDocument の現在の内容に適用し、WorkspaceEdit で反映する(Bethany と同じ)。
 * 結果は決まった形で書き出し、変わった範囲だけを置き換える(1 回の Undo で元に戻せる)。
 */
export async function applyEditCommand(
  document: vscode.TextDocument,
  command: EditCommand,
): Promise<ApplyResult> {
  // 自作部品(docs/adr/0044)をカタログに足して、検証・書き出しをする
  const classes = await customClassesOf(document.uri);
  const current = document.getText();
  const applied = withCustomClasses(classes, () => {
    const parsed = parseDocument(current);
    if (!parsed.document) {
      return undefined;
    }
    const result = applyCommand(parsed.document, command);
    return result.ok ? { ok: true as const, text: serializeDocument(result.document) } : result;
  });
  if (!applied) {
    return {
      ok: false,
      error: vscode.l10n.t(
        'The file cannot be edited in the designer because its content is invalid',
      ),
    };
  }
  if (!applied.ok) {
    return applied;
  }
  const eol = document.eol === vscode.EndOfLine.CRLF ? '\r\n' : '\n';
  const next = applied.text.replace(/\n/g, eol);
  const change = minimalTextEdit(current, next);
  if (!change) {
    return { ok: true };
  }
  const edit = new vscode.WorkspaceEdit();
  edit.replace(
    document.uri,
    new vscode.Range(document.positionAt(change.start), document.positionAt(change.end)),
    change.text,
  );
  const done = await vscode.workspace.applyEdit(edit);
  return done
    ? { ok: true }
    : { ok: false, error: vscode.l10n.t('Failed to apply the change to the file') };
}
