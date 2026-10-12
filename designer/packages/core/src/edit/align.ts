/**
 * 整列(docs/designer/editor-design.md §3.8)。自由な配置の部品(レイアウトの無い入れ物の子・PlaceLayout の項目)を、
 * 主に選んでいる部品に揃える・等間隔に並べる・大きさを揃える。
 * 位置はキャンバスの描画の結果(ウィンドウの座標)で求め、動かす量を書いた位置(子の x・y、placement の x・y)に足す。
 * こうすると、PlaceLayout の相対の位置・基準点によらず、見た目どおりに動く。
 */
import type { JsonPath } from '../dsl/diagnostics.ts';
import type { PropertyValue, TtuiDocument } from '../dsl/schema.ts';
import type { EditCommand } from './commands.ts';
import { valueAt } from './structure.ts';

export type AlignOperation =
  | 'left'
  | 'center_horizontal'
  | 'right'
  | 'top'
  | 'center_vertical'
  | 'bottom'
  | 'distribute_horizontal'
  | 'distribute_vertical'
  | 'same_width'
  | 'same_height';

export const ALIGN_OPERATIONS: readonly AlignOperation[] = [
  'left',
  'center_horizontal',
  'right',
  'top',
  'center_vertical',
  'bottom',
  'distribute_horizontal',
  'distribute_vertical',
  'same_width',
  'same_height',
];

export interface Rect {
  readonly x: number;
  readonly y: number;
  readonly width: number;
  readonly height: number;
}

/** 揃える部品と、その位置と大きさ(キャンバスの描画の結果。ウィンドウの座標)。 */
export interface AlignTarget {
  readonly path: JsonPath;
  readonly rect: Rect;
}

type Json = Record<string, unknown>;

function isObject(value: unknown): value is Json {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

/** 自由な配置の部品か(子なら 'child'、PlaceLayout の項目なら 'place')。 */
export function freePlacement(doc: TtuiDocument, path: JsonPath): 'child' | 'place' | undefined {
  const n = path.length;
  if (path[n - 2] === 'children') {
    return 'child';
  }
  if (path[n - 1] === 'widget' && path[n - 3] === 'items') {
    const layout = valueAt(doc, path.slice(0, -3));
    return isObject(layout) && layout['class'] === 'PlaceLayout' ? 'place' : undefined;
  }
  return undefined;
}

/** 揃えられるか(自由な配置の部品が 2 つ以上。等間隔は 3 つ以上)。 */
export function canAlign(
  doc: TtuiDocument,
  paths: readonly JsonPath[],
  operation?: AlignOperation,
): boolean {
  const min = operation === 'distribute_horizontal' || operation === 'distribute_vertical' ? 3 : 2;
  return paths.length >= min && paths.every((p) => freePlacement(doc, p) !== undefined);
}

/** 揃えた後の位置と大きさ。 */
function aligned(targets: readonly AlignTarget[], anchor: Rect, operation: AlignOperation): Rect[] {
  const rects = targets.map((t) => t.rect);
  switch (operation) {
    case 'left':
      return rects.map((r) => ({ ...r, x: anchor.x }));
    case 'center_horizontal':
      return rects.map((r) => ({ ...r, x: anchor.x + (anchor.width - r.width) / 2 }));
    case 'right':
      return rects.map((r) => ({ ...r, x: anchor.x + anchor.width - r.width }));
    case 'top':
      return rects.map((r) => ({ ...r, y: anchor.y }));
    case 'center_vertical':
      return rects.map((r) => ({ ...r, y: anchor.y + (anchor.height - r.height) / 2 }));
    case 'bottom':
      return rects.map((r) => ({ ...r, y: anchor.y + anchor.height - r.height }));
    case 'same_width':
      return rects.map((r) => ({ ...r, width: anchor.width }));
    case 'same_height':
      return rects.map((r) => ({ ...r, height: anchor.height }));
    case 'distribute_horizontal':
    case 'distribute_vertical': {
      // 両端の部品はそのままにして、間の空きを等しくする
      const horizontal = operation === 'distribute_horizontal';
      const start = (r: Rect): number => (horizontal ? r.x : r.y);
      const size = (r: Rect): number => (horizontal ? r.width : r.height);
      const order = rects.map((r, i) => ({ r, i })).sort((a, b) => start(a.r) - start(b.r));
      const first = order[0]?.r;
      const last = order[order.length - 1]?.r;
      if (!first || !last) {
        return rects;
      }
      const used = rects.reduce((sum, r) => sum + size(r), 0);
      const gap = (start(last) + size(last) - start(first) - used) / (rects.length - 1);
      const out = [...rects];
      let at = start(first);
      for (const { r, i } of order) {
        out[i] = horizontal ? { ...r, x: at } : { ...r, y: at };
        at += size(r) + gap;
      }
      return out;
    }
  }
}

/** dp の値を 0.5 単位に丸める(揃えた位置に半端な値を書かない)。 */
function round(v: number): number {
  return Math.round(v * 2) / 2;
}

/**
 * 揃える編集のコマンド(変わる部品だけ)。揃えられなければ空。
 * @param anchor 基準にする部品(主に選んでいる部品)の targets の中の位置
 */
export function alignCommands(
  doc: TtuiDocument,
  targets: readonly AlignTarget[],
  anchor: number,
  operation: AlignOperation,
): EditCommand[] {
  const base = targets[anchor];
  if (
    !base ||
    !canAlign(
      doc,
      targets.map((t) => t.path),
      operation,
    )
  ) {
    return [];
  }
  const next = aligned(targets, base.rect, operation);
  return targets.flatMap((t, i): EditCommand[] => {
    const r = next[i];
    if (!r) {
      return [];
    }
    const dx = round(r.x - t.rect.x);
    const dy = round(r.y - t.rect.y);
    const dw = round(r.width - t.rect.width);
    const dh = round(r.height - t.rect.height);
    if (dx === 0 && dy === 0 && dw === 0 && dh === 0) {
      return [];
    }
    const kind = freePlacement(doc, t.path);
    const node = valueAt(doc, kind === 'place' ? t.path.slice(0, -1) : t.path);
    const current: Json = isObject(node)
      ? kind === 'place'
        ? isObject(node['placement'])
          ? node['placement']
          : {}
        : isObject(node['properties'])
          ? node['properties']
          : {}
      : {};
    const num = (key: string): number | undefined =>
      typeof current[key] === 'number' ? current[key] : undefined;
    const values: Record<string, PropertyValue> = {};
    if (dx !== 0) {
      values['x'] = round((num('x') ?? 0) + dx);
    }
    if (dy !== 0) {
      values['y'] = round((num('y') ?? 0) + dy);
    }
    // 大きさを書いていなければ(望む大きさ。PlaceLayout で相対の大きさも無ければ)、揃えた大きさをそのまま書く
    const sized = (key: 'width' | 'height', delta: number, now: number): void => {
      if (delta === 0) {
        return;
      }
      const relative = kind === 'place' && (num(`relative_${key}`) ?? 0) !== 0;
      values[key] = round((num(key) ?? (relative ? 0 : now)) + delta);
    };
    sized('width', dw, t.rect.width);
    sized('height', dh, t.rect.height);
    if (kind === 'place') {
      return [
        {
          kind: 'setItem',
          target: t.path.slice(0, -1),
          values: { placement: { ...current, ...values } as PropertyValue },
        },
      ];
    }
    return [{ kind: 'setProperties', target: t.path, properties: values }];
  });
}
