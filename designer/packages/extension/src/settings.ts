/**
 * 拡張の設定(tokotachiDesigner.*)。コード生成の設定は、プロジェクトファイル(*.ttproj.json)を作るまではここに持つ
 * (docs/designer/codegen-design.md §8)。
 */
import {
  DEFAULT_CPP_OPTIONS,
  type CommentLocale,
  type CppOptions,
} from '@tokotachi-designer/codegen';
import * as vscode from 'vscode';

export const SECTION = 'tokotachiDesigner';

/** 空の文字は undefined(既定)にする。 */
function optional(value: string | undefined): string | undefined {
  const v = value?.trim();
  return v === undefined || v === '' ? undefined : v;
}

/** コード生成の設定。 */
export function cppOptions(scope?: vscode.Uri): CppOptions {
  const config = vscode.workspace.getConfiguration(`${SECTION}.codegen`, scope);
  const namespace = optional(config.get<string>('namespace'));
  const locale = config.get<string>('commentLocale', 'en');
  return {
    ...DEFAULT_CPP_OPTIONS,
    namespace: namespace === undefined ? [] : namespace.split('::').map((n) => n.trim()),
    headerDir: optional(config.get<string>('headerDir')),
    sourceDir: optional(config.get<string>('sourceDir')),
    commentLocale: (locale === 'ja' ? 'ja' : 'en') satisfies CommentLocale,
  };
}

export function generateOnSave(scope?: vscode.Uri): boolean {
  return vscode.workspace
    .getConfiguration(`${SECTION}.codegen`, scope)
    .get<boolean>('generateOnSave', true);
}

/** キャンバスの配色(auto は VS Code の配色テーマに合わせる)。 */
export function colorScheme(): 'LIGHT' | 'DARK' {
  const setting = vscode.workspace
    .getConfiguration(`${SECTION}.canvas`)
    .get<string>('colorScheme', 'auto');
  if (setting === 'light') {
    return 'LIGHT';
  }
  if (setting === 'dark') {
    return 'DARK';
  }
  const kind = vscode.window.activeColorTheme.kind;
  return kind === vscode.ColorThemeKind.Dark || kind === vscode.ColorThemeKind.HighContrast
    ? 'DARK'
    : 'LIGHT';
}
