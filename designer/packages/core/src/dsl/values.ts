/**
 * プロパティの値が型に合うかを調べる(docs/designer/dsl-spec.md §5)。
 * 参照(`{ ref }`)は名前が文字列かだけを調べ、相手があるか・型が合うかは validate.ts で調べる。
 */
import { getCatalog } from '../catalog/catalog.ts';
import type { PrimitiveType, PropertyType } from '../catalog/types.ts';
import { l10n } from '../l10n.ts';
import { IMAGE_EXTENSIONS, isImagePath } from './images.ts';
import type { PropertyValue } from './schema.ts';

/** 型の表示(メッセージ用)。 */
export function describeType(type: PropertyType): string {
  if (typeof type === 'string') {
    return type;
  }
  if ('enum' in type) {
    return type.enum;
  }
  if ('value' in type) {
    return type.value;
  }
  if ('array' in type) {
    return `${type.array}[]`;
  }
  return `${type.ref}*`;
}

function checkPrimitive(type: PrimitiveType, value: PropertyValue): boolean {
  switch (type) {
    case 'bool':
      return typeof value === 'boolean';
    case 'int':
      return typeof value === 'number' && Number.isInteger(value);
    case 'float':
    case 'double':
      return typeof value === 'number' && Number.isFinite(value);
    case 'string':
      return typeof value === 'string';
  }
}

function isObject(value: PropertyValue): value is { [key: string]: PropertyValue } {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

/** 列挙型の値か。 */
export function isEnumValue(enumName: string, value: PropertyValue): boolean {
  return (
    typeof value === 'string' && (getCatalog().enums[enumName]?.values.includes(value) ?? false)
  );
}

/** `#RRGGBB`・`#RRGGBBAA`。 */
export const COLOR_PATTERN = /^#[0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?$/;

const SHORTCUT_MODIFIERS = new Set(['ctrl', 'shift', 'alt', 'meta']);
const SHORTCUT_KEYS = new Set([
  'delete',
  'insert',
  'backspace',
  'tab',
  'enter',
  'escape',
  'space',
  'left',
  'right',
  'up',
  'down',
  'home',
  'end',
  'pageup',
  'pagedown',
]);

/** Shortcut の文字(Tokotachi の Shortcut(const char*) が読めるもの。空は「割り当てなし」)。 */
export function isShortcut(text: string): boolean {
  if (text === '') {
    return true;
  }
  const parts = text.split('+').map((p) => p.trim().toLowerCase());
  const key = parts.pop() ?? '';
  if (!parts.every((p) => SHORTCUT_MODIFIERS.has(p))) {
    return false;
  }
  return /^[a-z0-9]$/.test(key) || /^f([1-9]|1[0-2])$/.test(key) || SHORTCUT_KEYS.has(key);
}

/**
 * 値が型に合うかを調べる。
 * @returns 合えば null、合わなければ利用者向けの説明
 */
export function checkValue(type: PropertyType, value: PropertyValue): string | null {
  const expected = (): string => l10n.t('Expected a value of type {0}', describeType(type));
  if (typeof type === 'string') {
    return checkPrimitive(type, value) ? null : expected();
  }
  const catalog = getCatalog();
  if ('enum' in type) {
    if (isEnumValue(type.enum, value)) {
      return null;
    }
    const values = catalog.enums[type.enum]?.values ?? [];
    return l10n.t('Expected one of: {0}', values.join(', '));
  }
  if ('array' in type) {
    return Array.isArray(value) && value.every((v) => checkPrimitive(type.array, v))
      ? null
      : expected();
  }
  if ('ref' in type) {
    return typeof value === 'string' ? null : l10n.t('Expected the name of a {0}', type.ref);
  }
  // 値の型
  const name = type.value;
  if (name === 'ThemeColor') {
    if (
      isEnumValue('ColorRole', value) ||
      (typeof value === 'string' && COLOR_PATTERN.test(value))
    ) {
      return null;
    }
    return l10n.t('Expected a color role ({0}) or "#RRGGBB" / "#RRGGBBAA"', 'ColorRole');
  }
  if (name === 'Shortcut') {
    if (typeof value === 'string' && isShortcut(value)) {
      return null;
    }
    return l10n.t('Expected a shortcut such as "Ctrl+S" (modifiers: Ctrl, Shift, Alt, Meta)');
  }
  if (name === 'Image') {
    if (typeof value === 'string' && isImagePath(value)) {
      return null;
    }
    return l10n.t(
      'Expected a relative path from this file to an image file ({0}), separated by "/"',
      IMAGE_EXTENSIONS.join(', '),
    );
  }
  // Margins は 4 辺に同じ値を数値 1 つで書ける
  if (name === 'Margins' && typeof value === 'number') {
    return Number.isFinite(value) ? null : expected();
  }
  return checkMembers(name, value);
}

/** メンバの名前のオブジェクト(一部だけ書ける)。 */
export function checkMembers(valueType: string, value: PropertyValue): string | null {
  const members = getCatalog().values[valueType]?.members;
  if (!members || !isObject(value)) {
    return l10n.t('Expected an object with the members of {0}', valueType);
  }
  for (const [key, v] of Object.entries(value)) {
    const t = Object.hasOwn(members, key) ? members[key] : undefined;
    if (t === undefined) {
      return l10n.t(
        '{0} has no member "{1}" (members: {2})',
        valueType,
        key,
        Object.keys(members).join(', '),
      );
    }
    const ok = typeof t === 'string' ? checkPrimitive(t, v) : isEnumValue(t.enum, v);
    if (!ok) {
      return l10n.t(
        'The member "{0}" of {1} expects a value of type {2}',
        key,
        valueType,
        typeof t === 'string' ? t : t.enum,
      );
    }
    if (valueType === 'Color' && typeof v === 'number' && (v < 0 || v > 255)) {
      return l10n.t('The member "{0}" of {1} must be between 0 and 255', key, valueType);
    }
  }
  return null;
}
