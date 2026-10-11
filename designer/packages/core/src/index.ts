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
export {
  checkThemeOverride,
  colorRoles,
  parseThemeOverride,
  replaceThemePreset,
  resolveThemeOverride,
  resolveThemePresets,
  THEME_DIMENSIONS,
  themePresetReferences,
  type ParsedThemeOverride,
  type ThemeDimension,
  type ThemePresetReference,
  type ThemePresets,
} from './dsl/themeOverride.ts';
export * from './dsl/outline.ts';
export { bindingOf, isBindable, variableTypeOf } from './dsl/bindings.ts';
export {
  IMAGE_EXTENSIONS,
  imageReferences,
  isImagePath,
  isImageType,
  type ImageReference,
} from './dsl/images.ts';
export {
  applyCommand,
  defaultEventOf,
  defaultHandlerName,
  type CommandResult,
  type EditCommand,
} from './edit/commands.ts';
export { minimalTextEdit, type TextEdit } from './edit/textEdit.ts';
export { variableValue } from './edit/variables.ts';
export type * from './protocol.ts';
export {
  newName,
  newWidget,
  snakeCase,
  valueAt,
  type Positions,
  type Slot,
} from './edit/structure.ts';
export * from './project/paths.ts';
export * from './project/schema.ts';
export {
  parseProject,
  serializeProject,
  validateProject,
  type ProjectParseResult,
} from './project/parse.ts';
export { themePreviewDocument } from './project/themePreview.ts';
