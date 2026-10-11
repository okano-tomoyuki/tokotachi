/**
 * テーマの上書き(Tokotachi の ThemeOverride。docs/adr/0042)の書き方(docs/designer/dsl-spec.md §5)。
 * 部品の theme_override と、プロジェクトのテーマ(project-spec.md)で使う。
 *
 *   { "colors": { "ACCENT": "#DC2626", "FIELD": { "light": "#FEF2F2", "dark": "#450A0A" } }, "radius": 0 }
 */
import { getCatalog } from '../catalog/catalog.ts';
import { l10n } from '../l10n.ts';
import { COLOR_PATTERN } from './values.ts';

/** 上書きできる寸法(Theme のメンバ)。 */
export const THEME_DIMENSIONS = ['radius', 'field_radius', 'field_padding'] as const;
export type ThemeDimension = (typeof THEME_DIMENSIONS)[number];

/** 読んだ上書き(色は役割の順)。 */
export interface ParsedThemeOverride {
  readonly colors: readonly {
    readonly role: string;
    readonly light: string;
    readonly dark: string;
  }[];
  readonly dimensions: readonly { readonly name: ThemeDimension; readonly value: number }[];
}

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
 * @returns 合えば null、合わなければ利用者向けの説明
 */
export function checkThemeOverride(value: unknown): string | null {
  if (!isObject(value)) {
    return l10n.t(
      'Expected an object with "colors" and the dimensions ({0})',
      THEME_DIMENSIONS.join(', '),
    );
  }
  const roles = colorRoles();
  for (const [key, v] of Object.entries(value)) {
    if (key === 'colors') {
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
        'A theme override cannot have "{0}" (keys: colors, {1})',
        key,
        THEME_DIMENSIONS.join(', '),
      );
    }
  }
  return null;
}

/** 検証を通った上書きを読む(色は役割の順、寸法は THEME_DIMENSIONS の順)。 */
export function parseThemeOverride(value: unknown): ParsedThemeOverride {
  const object = isObject(value) ? value : {};
  const colors = isObject(object['colors']) ? object['colors'] : {};
  return {
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
