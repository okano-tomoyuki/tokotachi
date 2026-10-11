/**
 * プロパティの値を C++ の式にする(docs/designer/codegen-design.md N4)。値は検証済み(型に合う)のものを受け取る。
 */
import {
  COLOR_PATTERN,
  getCatalog,
  parseThemeOverride,
  type PrimitiveType,
  type PropertyType,
  type PropertyValue,
} from '@tokotachi-designer/core';

/** C++ の文字列リテラル(UTF-8 のまま。`"`・`\`・制御文字はエスケープする)。 */
export function cppString(text: string): string {
  let out = '"';
  for (const ch of text) {
    const code = ch.codePointAt(0) ?? 0;
    if (ch === '"' || ch === '\\') {
      out += `\\${ch}`;
    } else if (ch === '\n') {
      out += '\\n';
    } else if (ch === '\r') {
      out += '\\r';
    } else if (ch === '\t') {
      out += '\\t';
    } else if (code < 0x20 || code === 0x7f) {
      // 8 進のエスケープは最大 3 桁なので、後ろに数字が続いても安全
      out += `\\${code.toString(8).padStart(3, '0')}`;
    } else {
      out += ch;
    }
  }
  return `${out}"`;
}

/** 検証済みの値(文字列・数値・真偽)の文字。 */
export function scalarText(value: PropertyValue): string {
  return typeof value === 'string' || typeof value === 'number' || typeof value === 'boolean'
    ? String(value)
    : JSON.stringify(value);
}

function number(value: number, type: 'int' | 'float' | 'double'): string {
  if (type === 'int' || Number.isInteger(value)) {
    return String(value);
  }
  return type === 'float' ? `${String(value)}f` : String(value);
}

function primitive(type: PrimitiveType, value: PropertyValue): string {
  switch (type) {
    case 'bool':
      return value === true ? 'true' : 'false';
    case 'string':
      return cppString(scalarText(value));
    default:
      return number(Number(value), type);
  }
}

/** 列挙型の値。 */
export function enumValue(enumName: string, value: string): string {
  return `tokotachi::${enumName}::${value}`;
}

type ValueObject = Readonly<Record<string, PropertyValue>>;

function member(object: ValueObject, name: string, fallback: number): number {
  const v = object[name];
  return typeof v === 'number' ? v : fallback;
}

function floats(values: readonly number[]): string {
  return values.map((v) => number(v, 'float')).join(', ');
}

/** `#RRGGBB`・`#RRGGBBAA` を tokotachi::Color に。 */
function color(hex: string): string {
  const parts = (hex.slice(1).match(/../g) ?? []).map((p) => `0x${p.toUpperCase()}`);
  return `tokotachi::Color(${parts.join(', ')})`;
}

/** テーマの上書きを 1 つの式に(`tokotachi::ThemeOverride().set(...).set_radius(0)`。docs/adr/0042)。 */
export function themeOverrideValue(value: unknown): string {
  const parsed = parseThemeOverride(value);
  const calls = [
    ...parsed.colors.map(({ role, light, dark }) =>
      light === dark
        ? `.set(${enumValue('ColorRole', role)}, ${color(light)})`
        : `.set(${enumValue('ColorRole', role)}, ${color(light)}, ${color(dark)})`,
    ),
    ...parsed.dimensions.map(({ name, value: v }) => `.set_${name}(${number(v, 'float')})`),
  ];
  return `tokotachi::ThemeOverride()${calls.join('')}`;
}

/** 値の型(Margins・Size 等)。 */
function valueType(name: string, value: PropertyValue): string {
  if (name === 'ThemeOverride') {
    return themeOverrideValue(value);
  }
  if (name === 'ThemeColor') {
    const text = scalarText(value);
    return COLOR_PATTERN.test(text) ? color(text) : enumValue('ColorRole', text);
  }
  if (name === 'Shortcut') {
    return `tokotachi::Shortcut(${cppString(scalarText(value))})`;
  }
  if (name === 'Margins' && typeof value === 'number') {
    return `tokotachi::Margins(${number(value, 'float')})`;
  }
  const o = (value ?? {}) as ValueObject;
  switch (name) {
    case 'Margins':
      return `tokotachi::Margins(${floats(['left', 'top', 'right', 'bottom'].map((m) => member(o, m, 0)))})`;
    case 'Size':
      return `tokotachi::Size(${floats([member(o, 'width', 0), member(o, 'height', 0)])})`;
    case 'Point':
      return `tokotachi::Point(${floats([member(o, 'x', 0), member(o, 'y', 0)])})`;
    case 'Rect':
      return `tokotachi::Rect(${floats(['x', 'y', 'width', 'height'].map((m) => member(o, m, 0)))})`;
    case 'Font':
      return 'size' in o
        ? `tokotachi::Font(${floats([member(o, 'size', 0)])})`
        : 'tokotachi::Font()';
    default:
      throw new Error(`unsupported value type: ${name}`);
  }
}

/** プロパティの値の C++ の式。参照は相手の名前(メンバか一時的な変数)。 */
export function cppValue(type: PropertyType, value: PropertyValue): string {
  if (typeof type === 'string') {
    return primitive(type, value);
  }
  if ('enum' in type) {
    return enumValue(type.enum, scalarText(value));
  }
  if ('array' in type) {
    const items = (value as readonly PropertyValue[]).map((v) => primitive(type.array, v));
    const element = type.array === 'string' ? 'std::string' : type.array;
    return `std::vector<${element}>{${items.join(', ')}}`;
  }
  if ('ref' in type) {
    return scalarText(value);
  }
  return valueType(type.value, value);
}

/** Placement のメンバを設定する文(`placement.relative_x = 0.5f;`)。 */
export function placementStatements(variable: string, value: ValueObject): string[] {
  const members = getCatalog().values['Placement']?.members ?? {};
  return Object.entries(value).map(([name, v]) => {
    const t = members[name];
    const expr =
      typeof t === 'object' ? enumValue(t.enum, scalarText(v)) : primitive(t ?? 'float', v);
    return `${variable}.${name} = ${expr};`;
  });
}
