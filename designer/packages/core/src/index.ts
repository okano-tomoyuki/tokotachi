export type * from './catalog/types.ts';
export {
  findClass,
  findHandler,
  findProperty,
  getCatalog,
  isSubclassOf,
} from './catalog/catalog.ts';
export {
  describeIdentifierProblem,
  isValidIdentifier,
  memberNameProblem,
  RESERVED_PREFIX,
  type IdentifierProblem,
} from './identifier.ts';
export { configureL10n, isJapanese, l10n, type L10nBundle } from './l10n.ts';

export * from './dsl/schema.ts';
export {
  hasErrors,
  type Diagnostic,
  type DiagnosticCode,
  type JsonPath,
  type Severity,
} from './dsl/diagnostics.ts';
export { locate, type TextRange } from './dsl/locate.ts';
export { parseDocument, type ParseResult } from './dsl/parse.ts';
export { serializeDocument } from './dsl/serialize.ts';
export * from './dsl/tree.ts';
export { validateDocument } from './dsl/validate.ts';
export { checkValue, COLOR_PATTERN, isEnumValue, isShortcut } from './dsl/values.ts';
