/**
 * 数値のプロパティの範囲(カタログの range。docs/designer/catalog.md §3)。端は数値か、同じ部品のほかのプロパティの値か、
 * 配列のプロパティ・子の数で決まるので、部品のノード(定義ファイルの JSON)から求める。
 */
import { findClass } from '../catalog/catalog.ts';
import type { PropertyInfo, RangeBound } from '../catalog/types.ts';
import { l10n } from '../l10n.ts';

export interface NumberRange {
  readonly min?: number;
  readonly max?: number;
}

function isObject(value: unknown): value is Record<string, unknown> {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

function boundValue(bound: RangeBound | undefined, cls: string, node: unknown): number | undefined {
  if (bound === undefined || typeof bound === 'number') {
    return bound;
  }
  const properties = isObject(node) && isObject(node['properties']) ? node['properties'] : {};
  if ('property' in bound) {
    // 書いていなければ既定値。Variable と結び付けているなど、数値でなければ決まらない
    const value =
      bound.property in properties
        ? properties[bound.property]
        : findClass(cls)?.properties[bound.property]?.default;
    return typeof value === 'number' ? value : undefined;
  }
  const list = properties[bound.count] ?? (isObject(node) ? node[bound.count] : undefined);
  return (Array.isArray(list) ? list.length : 0) + (bound.offset ?? 0);
}

/**
 * 部品のプロパティの範囲。範囲を持たないプロパティなら undefined。
 * @param node プロパティを持つノード(部品・タブ・列など。定義ファイルの JSON)
 */
export function propertyRange(
  cls: string,
  info: PropertyInfo,
  node: unknown,
): NumberRange | undefined {
  if (!info.range) {
    return undefined;
  }
  const min = boundValue(info.range.min, cls, node);
  let max = boundValue(info.range.max, cls, node);
  // minimum を maximum より大きくすると、実行するとき maximum も同じ値になる
  if (min !== undefined && max !== undefined && max < min) {
    max = min;
  }
  return { ...(min !== undefined ? { min } : {}), ...(max !== undefined ? { max } : {}) };
}

/** 値を範囲に収める。 */
export function clampToRange(value: number, range: NumberRange | undefined): number {
  let v = value;
  if (range?.min !== undefined && v < range.min) {
    v = range.min;
  }
  if (range?.max !== undefined && v > range.max) {
    v = range.max;
  }
  return v;
}

export function inRange(value: number, range: NumberRange | undefined): boolean {
  return clampToRange(value, range) === value;
}

/** 範囲の説明(「0〜10」「8 以上」)。端が無ければ undefined。 */
export function describeRange(range: NumberRange | undefined): string | undefined {
  if (range?.min !== undefined && range.max !== undefined) {
    return l10n.t('{0} to {1}', range.min, range.max);
  }
  if (range?.min !== undefined) {
    return l10n.t('{0} or more', range.min);
  }
  if (range?.max !== undefined) {
    return l10n.t('{0} or less', range.max);
  }
  return undefined;
}
