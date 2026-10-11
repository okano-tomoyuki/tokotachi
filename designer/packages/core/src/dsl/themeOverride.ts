/**
 * テーマの上書き(Tokotachi の ThemeOverride。docs/adr/0042)の書き方(docs/designer/dsl-spec.md §5.3)。
 * 部品の theme_override と、プロジェクトのテーマ(project-spec.md §4.1)で使う。
 *
 *   { "preset": "danger", "colors": { "ACCENT": "#DC2626", "FIELD": { "light": "#FEF2F2", "dark": "#450A0A" } }, "radius": 0 }
 *
 * preset はプロジェクトに登録したテーマ(theme.presets)の名前。部品の theme_override だけに書ける。
 */
import { getCatalog } from '../catalog/catalog.ts';
import { isValidIdentifier } from '../identifier.ts';
import { l10n } from '../l10n.ts';
import type { JsonPath } from './diagnostics.ts';
import type { TtuiDocument } from './schema.ts';
import { COLOR_PATTERN } from './values.ts';

/** 上書きできる寸法(Theme のメンバ)。 */
export const THEME_DIMENSIONS = ['radius', 'field_radius', 'field_padding'] as const;
export type ThemeDimension = (typeof THEME_DIMENSIONS)[number];

/** 読んだ上書き(色は役割の順)。 */
export interface ParsedThemeOverride {
  /** 参照するプロジェクトのテーマの名前(あれば)。 */
  readonly preset?: string;
  readonly colors: readonly {
    readonly role: string;
    readonly light: string;
    readonly dark: string;
  }[];
  readonly dimensions: readonly { readonly name: ThemeDimension; readonly value: number }[];
}

/** プロジェクトに登録したテーマ(名前 → 上書き)。 */
export type ThemePresets = Readonly<Record<string, unknown>>;

function isObject(value: unknown): value is Record<string, unknown> {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

function isColor(value: unknown): value is string {
  return typeof value === 'string' && COLOR_PATTERN.test(value);
}

/** 色の役割(ColorRole の値。カタログの順)。 */
export function colorRoles(): readonly string[] {
  return getCatalog().enums['ColorRole']?.values ?? [];
}

/**
 * 上書きの書き方に合うかを調べる。
 * @param allowPreset preset(プロジェクトのテーマの参照)を書けるか(部品の theme_override だけ)
 * @returns 合えば null、合わなければ利用者向けの説明
 */
export function checkThemeOverride(value: unknown, allowPreset = true): string | null {
  if (!isObject(value)) {
    return l10n.t(
      'Expected an object with "colors" and the dimensions ({0})',
      THEME_DIMENSIONS.join(', '),
    );
  }
  const roles = colorRoles();
  for (const [key, v] of Object.entries(value)) {
    if (key === 'preset' && allowPreset) {
      if (typeof v !== 'string' || isValidIdentifier(v) !== null) {
        return l10n.t('"preset" is the name of a theme registered in the project');
      }
    } else if (key === 'colors') {
      if (!isObject(v)) {
        return l10n.t('"colors" is an object from a color role to a color');
      }
      for (const [role, color] of Object.entries(v)) {
        if (!roles.includes(role)) {
          return l10n.t('"{0}" is not a color role (roles: {1})', role, roles.join(', '));
        }
        const ok =
          isColor(color) ||
          (isObject(color) &&
            Object.keys(color).every((k) => k === 'light' || k === 'dark') &&
            isColor(color['light']) &&
            isColor(color['dark']));
        if (!ok) {
          return l10n.t(
            'The color of {0} is "#RRGGBB" / "#RRGGBBAA", or {"light": ..., "dark": ...}',
            role,
          );
        }
      }
    } else if ((THEME_DIMENSIONS as readonly string[]).includes(key)) {
      if (typeof v !== 'number' || !Number.isFinite(v) || v < 0) {
        return l10n.t('"{0}" is a number of 0 or more (dp)', key);
      }
    } else {
      return l10n.t(
        'A theme override cannot have "{0}" (keys: {1})',
        key,
        [...(allowPreset ? ['preset'] : []), 'colors', ...THEME_DIMENSIONS].join(', '),
      );
    }
  }
  return null;
}

/** 検証を通った上書きを読む(色は役割の順、寸法は THEME_DIMENSIONS の順)。 */
export function parseThemeOverride(value: unknown): ParsedThemeOverride {
  const object = isObject(value) ? value : {};
  const colors = isObject(object['colors']) ? object['colors'] : {};
  const preset = object['preset'];
  return {
    ...(typeof preset === 'string' ? { preset } : {}),
    colors: colorRoles().flatMap((role) => {
      const c = colors[role];
      if (isColor(c)) {
        return [{ role, light: c, dark: c }];
      }
      if (isObject(c) && isColor(c['light']) && isColor(c['dark'])) {
        return [{ role, light: c['light'], dark: c['dark'] }];
      }
      return [];
    }),
    dimensions: THEME_DIMENSIONS.flatMap((name) => {
      const v = object[name];
      return typeof v === 'number' ? [{ name, value: v }] : [];
    }),
  };
}

/**
 * preset を、登録したテーマに書いたもので展開する(テーマの上に、部品に書いた色・寸法を重ねる)。
 * @returns 展開した上書き。参照するテーマが無ければ undefined
 */
export function resolveThemeOverride(
  value: unknown,
  presets: ThemePresets | undefined,
): Record<string, unknown> | undefined {
  if (!isObject(value)) {
    return undefined;
  }
  const { preset, ...own } = value;
  if (typeof preset !== 'string') {
    return own;
  }
  const base = presets?.[preset];
  if (!isObject(base)) {
    return undefined;
  }
  const colors = {
    ...(isObject(base['colors']) ? base['colors'] : {}),
    ...(isObject(own['colors']) ? own['colors'] : {}),
  };
  return {
    ...base,
    ...own,
    ...(Object.keys(colors).length > 0 ? { colors } : {}),
  };
}

/** プロジェクトのテーマを参照している場所。 */
export interface ThemePresetReference {
  readonly name: string;
  /** preset を書いた位置。 */
  readonly at: JsonPath;
}

/** 部品の theme_override で、preset を書いた場所(ノードの properties を順にたどる)。 */
export function themePresetReferences(doc: TtuiDocument): ThemePresetReference[] {
  const out: ThemePresetReference[] = [];
  const visit = (node: unknown, path: JsonPath): void => {
    if (Array.isArray(node)) {
      node.forEach((child, i) => {
        visit(child, [...path, i]);
      });
      return;
    }
    if (!isObject(node)) {
      return;
    }
    for (const [key, child] of Object.entries(node)) {
      if (key === 'properties' && isObject(child)) {
        const override = child['theme_override'];
        if (isObject(override) && typeof override['preset'] === 'string') {
          out.push({
            name: override['preset'],
            at: [...path, 'properties', 'theme_override', 'preset'],
          });
        }
      } else {
        visit(child, [...path, key]);
      }
    }
  };
  visit(doc, []);
  return out;
}

/** preset を書いた theme_override を、登録したテーマで展開した写し(キャンバスで描くため)。展開できないものは preset を外す。 */
export function resolveThemePresets(
  doc: TtuiDocument,
  presets: ThemePresets | undefined,
): TtuiDocument {
  const copy = JSON.parse(JSON.stringify(doc)) as TtuiDocument;
  for (const ref of themePresetReferences(copy)) {
    let properties: unknown = copy;
    for (const key of ref.at.slice(0, -2)) {
      properties = (properties as Record<string | number, unknown>)[key];
    }
    const target = properties as Record<string, unknown>;
    target['theme_override'] =
      resolveThemeOverride(target['theme_override'], presets) ??
      resolveThemeOverride(target['theme_override'], { [ref.name]: {} });
  }
  return copy;
}
