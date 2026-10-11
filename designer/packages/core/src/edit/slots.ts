/**
 * 部品を入れる場所(Slot)を求める(docs/designer/editor-design.md §4.1)。パレットのクリック・複製・まとめて移すときに使う。
 */
import type { JsonPath } from '../dsl/diagnostics.ts';
import { flattenOutline, outline } from '../dsl/outline.ts';
import type { TtuiDocument } from '../dsl/schema.ts';
import { moveWidget, valueAt, type Slot } from './structure.ts';

type Json = Readonly<Record<string, unknown>>;

/** レイアウトの終わりのスロット(GridLayout は新しい行の 0 列)。 */
export function endOfLayout(document: TtuiDocument, layoutPath: JsonPath): Slot | undefined {
  const layout = valueAt(document, layoutPath) as Json | undefined;
  if (!layout) {
    return undefined;
  }
  const items = (layout['items'] as Json[] | undefined) ?? [];
  if (layout['class'] === 'GridLayout') {
    const row = items.reduce(
      (m, i) => Math.max(m, Number(i['row'] ?? 0) + Number(i['row_span'] ?? 1)),
      0,
    );
    return { kind: 'item', layout: layoutPath, index: items.length, row, column: 0 };
  }
  if (layout['class'] === 'PlaceLayout') {
    return { kind: 'item', layout: layoutPath, index: items.length, x: 8, y: 8 };
  }
  return { kind: 'item', layout: layoutPath, index: items.length };
}

/**
 * 部品のすぐ後ろ(複製を入れる場所)。BoxLayout は次の項目、ほかのレイアウトは終わり、自由な配置は少しずらした位置、
 * タブ・区画・ツールバー・ステータスバーは次の位置。部品でなければ undefined。
 */
export function slotAfter(document: TtuiDocument, path: JsonPath): Slot | undefined {
  const n = path.length;
  const last = path[n - 1];
  const index = (key: string | number | undefined): number => Number(key) + 1;
  if (last === 'widget' && path[n - 3] === 'items') {
    const owner = path.slice(0, -3);
    if (owner.length === 2 && owner[1] === 'tool_bar') {
      return { kind: 'tool_bar', index: index(path[n - 2]) };
    }
    const layout = valueAt(document, owner) as Json | undefined;
    return layout?.['class'] === 'BoxLayout'
      ? { kind: 'item', layout: owner, index: index(path[n - 2]) }
      : endOfLayout(document, owner);
  }
  if (path[n - 2] === 'children') {
    const json = valueAt(document, path) as { properties?: Json } | undefined;
    const at = (key: string): number => {
      const v = json?.properties?.[key];
      return typeof v === 'number' ? v : 0;
    };
    return { kind: 'child', parent: path.slice(0, -2), x: at('x') + 16, y: at('y') + 16 };
  }
  if (last === 'page' && path[n - 3] === 'tabs') {
    return { kind: 'tab', parent: path.slice(0, -3), index: index(path[n - 2]) };
  }
  if (last === 'widget' && path[n - 3] === 'panes') {
    return { kind: 'pane', parent: path.slice(0, -3), index: index(path[n - 2]) };
  }
  if (path[n - 2] === 'widgets' && path[n - 3] === 'status_bar') {
    return { kind: 'status_bar', index: index(last) };
  }
  return undefined;
}

/** まとめて移すときに、部品に一時的に付ける印(移すと位置が変わるので、印で探し直す)。 */
const MARK = '__tokotachi_moving';

function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

/** 印を付けた部品の位置。 */
function findMarked(node: unknown, mark: number, path: JsonPath = []): JsonPath | undefined {
  if (Array.isArray(node)) {
    for (let i = 0; i < node.length; i++) {
      const found = findMarked(node[i], mark, [...path, i]);
      if (found) {
        return found;
      }
    }
    return undefined;
  }
  if (!isRecord(node)) {
    return undefined;
  }
  if (node[MARK] === mark) {
    return path;
  }
  for (const [key, child] of Object.entries(node)) {
    const found = findMarked(child, mark, [...path, key]);
    if (found) {
      return found;
    }
  }
  return undefined;
}

function isInside(path: JsonPath, base: JsonPath): boolean {
  return base.length <= path.length && base.every((key, i) => path[i] === key);
}

/**
 * 選んだもののうち、ほかの選んだものの中に無いもの(親ごと扱う)を、構造の木の順(画面で見える順)に。
 */
export function outermostPaths(doc: TtuiDocument, paths: readonly JsonPath[]): JsonPath[] {
  const order = new Map(flattenOutline(outline(doc)).map((n, i) => [n.id, i]));
  const rank = (p: JsonPath): number => order.get(p.join('/')) ?? Number.MAX_SAFE_INTEGER;
  return paths
    .filter((p) => !paths.some((o) => o !== p && o.length < p.length && isInside(p, o)))
    .sort((a, b) => rank(a) - rank(b));
}

/** 自由な配置の位置(子の x・y か、PlaceLayout の placement の x・y)。無ければ undefined。 */
function freePosition(doc: TtuiDocument, path: JsonPath): { x: number; y: number } | undefined {
  const n = path.length;
  const read = (json: unknown): { x: number; y: number } | undefined => {
    if (!isRecord(json)) {
      return undefined;
    }
    return {
      x: typeof json['x'] === 'number' ? json['x'] : 0,
      y: typeof json['y'] === 'number' ? json['y'] : 0,
    };
  };
  if (path[n - 2] === 'children') {
    return read((valueAt(doc, path) as Json | undefined)?.['properties']);
  }
  if (path[n - 1] === 'widget' && path[n - 3] === 'items') {
    return read((valueAt(doc, path.slice(0, -1)) as Json | undefined)?.['placement']);
  }
  return undefined;
}

/**
 * 部品をまとめて slot に移す。構造の木の順に並べて入れる(最初を slot に、続きはその後ろに)。
 * 自由な配置(子・PlaceLayout)に移すときは、互いの位置関係を保つ(元が自由な配置でなければ 16dp ずつずらす)。
 */
export function moveWidgets(doc: TtuiDocument, targets: readonly JsonPath[], slot: Slot): void {
  const paths = outermostPaths(doc, targets);
  const origins = paths.map((p) => freePosition(doc, p));
  paths.forEach((p, i) => {
    (valueAt(doc, p) as Record<string, unknown>)[MARK] = i;
  });
  try {
    paths.forEach((_, i) => {
      const from = findMarked(doc, i);
      if (!from) {
        return;
      }
      let next = slot;
      if (i > 0) {
        const prev = findMarked(doc, i - 1);
        const free = slot.kind === 'child' || (slot.kind === 'item' && slot.x !== undefined);
        if (free) {
          const o = origins[i];
          const o0 = origins[0];
          const dx = o && o0 ? o.x - o0.x : 16 * i;
          const dy = o && o0 ? o.y - o0.y : 16 * i;
          const x = (slot.kind === 'child' ? slot.x : (slot.x ?? 0)) + dx;
          const y = (slot.kind === 'child' ? slot.y : (slot.y ?? 0)) + dy;
          next =
            slot.kind === 'child'
              ? { ...slot, x, y }
              : { ...slot, x, y, index: prev ? Number(prev[prev.length - 2]) + 1 : slot.index };
        } else {
          next = (prev ? slotAfter(doc, prev) : undefined) ?? slot;
        }
      }
      moveWidget(doc, from, next);
    });
  } finally {
    paths.forEach((_, i) => {
      const at = findMarked(doc, i);
      if (at) {
        Reflect.deleteProperty(valueAt(doc, at) as Record<string, unknown>, MARK);
      }
    });
  }
}
