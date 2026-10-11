/** JSON 上の位置。例: ["window", "layout", "items", 2, "widget", "properties", "text"] */
export type JsonPath = readonly (string | number)[];

export type Severity = 'error' | 'warning';

export type DiagnosticCode =
  // 読み込み・構造
  | 'json-syntax'
  | 'unsupported-version'
  | 'schema'
  // 名前
  | 'invalid-identifier'
  | 'duplicate-name'
  | 'reserved-name'
  | 'naming-style'
  // カタログ
  | 'unknown-class'
  | 'wrong-class-kind'
  | 'unknown-property'
  | 'not-designable-property'
  | 'geometry-not-allowed'
  | 'invalid-property-value'
  | 'out-of-range'
  | 'unknown-event'
  // 参照・ハンドラ
  | 'unknown-reference'
  | 'reference-type-mismatch'
  | 'handler-signature-conflict'
  | 'binding-not-allowed'
  // 構造上の制約
  | 'children-not-allowed'
  | 'layout-and-children'
  | 'child-key-not-allowed'
  | 'buttons-not-allowed'
  | 'invalid-layout-item'
  | 'grid-cell-overlap'
  | 'tracks-not-allowed'
  | 'invalid-menu-item'
  | 'invalid-tool-item'
  | 'invalid-dialog-button'
  // ファイル(core では調べず、拡張が調べて出す)
  | 'missing-image'
  | 'unknown-theme';

export interface Diagnostic {
  readonly severity: Severity;
  readonly code: DiagnosticCode;
  readonly message: string;
  readonly path: JsonPath;
}

export function hasErrors(diagnostics: readonly Diagnostic[]): boolean {
  return diagnostics.some((d) => d.severity === 'error');
}
