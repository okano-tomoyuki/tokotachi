/**
 * ドラッグした部品を入れる場所(docs/designer/editor-design.md §4.1)。Qt Designer と同じく、点の下のいちばん内側の入れ物の
 * レイアウトに合わせて決める: BoxLayout は項目の間、GridLayout はセル、PlaceLayout とレイアウトの無い入れ物はその点、
 * TabWidget は新しいタブ、Splitter は区画の間、ScrollArea は中身。
 * 位置はキャンバスの描画の結果(部品の位置と大きさ)から求める。座標はウィンドウの dp。
 */
import {
  findClass,
  flattenOutline,
  valueAt,
  type CanvasRect,
  type JsonPath,
  type OutlineNode,
  type Slot,
  type TtuiDocument,
} from '@tokotachi-designer/core';

/** ドラッグしているもの。 */
export type Drag =
  /** パレットの部品(新しく足す)。 */
  | { readonly kind: 'new'; readonly class: string }
  /** パレットの空き(BoxLayout にだけ入れられる)。 */
  | { readonly kind: 'spacer'; readonly spacer: { stretch: number } | { spacing: number } }
  /** キャンバス・構造の木の部品(移す)。 */
  | { readonly kind: 'move'; readonly path: JsonPath; readonly id: string };

/** 入れる場所の印(線か枠)。 */
export interface Indicator {
  readonly shape: 'line' | 'box';
  readonly x: number;
  readonly y: number;
  readonly width: number;
  readonly height: number;
}

export interface DropTarget {
  /** 入れられなければ undefined(印は赤く出す)。 */
  readonly slot: Slot | undefined;
  readonly indicator: Indicator;
  /** 入れ物(枠を出す)。 */
  readonly container: CanvasRect;
}

interface Frame {
  readonly width: number;
  readonly height: number;
  readonly rects: Readonly<Record<string, CanvasRect>>;
}

interface Json {
  readonly [key: string]: unknown;
}

function contains(r: CanvasRect, x: number, y: number): boolean {
  return x >= r.x && y >= r.y && x < r.x + r.width && y < r.y + r.height;
}

const LINE = 3;

export function dropTarget(
  doc: TtuiDocument,
  roots: readonly OutlineNode[],
  frame: Frame,
  x: number,
  y: number,
  drag: Drag,
): DropTarget | undefined {
  const whole: CanvasRect = { x: 0, y: 0, width: frame.width, height: frame.height, showing: true };
  const all = flattenOutline(roots);
  // 入れ物の候補(移す部品とその中は除く)
  const moving = drag.kind === 'move' ? `${drag.id}/` : undefined;
  let best: { node: OutlineNode; rect: CanvasRect } | undefined;
  for (const node of all) {
    if (node.kind !== 'widget' && node.kind !== 'window') {
      continue;
    }
    if (moving !== undefined && `${node.id}/`.startsWith(moving)) {
      continue;
    }
    const info = node.class === undefined ? undefined : findClass(node.class);
    if (node.kind === 'widget' && !info?.accepts_children && !info?.child_keys) {
      continue;
    }
    const rect =
      node.kind === 'window'
        ? whole
        : node.variable === undefined
          ? undefined
          : frame.rects[node.variable];
    if (!rect?.showing || !contains(rect, x, y)) {
      continue;
    }
    if (!best || rect.width * rect.height <= best.rect.width * best.rect.height) {
      best = { node, rect };
    }
  }
  if (!best) {
    return undefined;
  }
  return slotIn(doc, best.node, best.rect, frame, x, y, drag);
}

/** 入れ物の中の場所。 */
function slotIn(
  doc: TtuiDocument,
  container: OutlineNode,
  rect: CanvasRect,
  frame: Frame,
  x: number,
  y: number,
  drag: Drag,
): DropTarget {
  const json = valueAt(doc, container.path) as Json;
  const none = (indicator: Indicator): DropTarget => ({
    slot: undefined,
    indicator,
    container: rect,
  });
  const point: Indicator = { shape: 'box', x: x - 4, y: y - 4, width: 8, height: 8 };
  const layout = json['layout'] as Json | undefined;
  const isSpacer = drag.kind === 'spacer';
  if (layout) {
    const layoutPath = [...container.path, 'layout'];
    const items = (layout['items'] as Json[] | undefined) ?? [];
    const rectOf = (item: Json): CanvasRect | undefined => {
      const w = item['widget'] as Json | undefined;
      return w ? frame.rects[String(w['name'])] : undefined;
    };
    const cls = layout['class'];
    if (cls === 'BoxLayout') {
      const props = (layout['properties'] as Json | undefined) ?? {};
      const horizontal = props['orientation'] === 'HORIZONTAL';
      let index = items.length;
      let edge: number | undefined;
      let last: number | undefined;
      for (const [i, item] of items.entries()) {
        const r = rectOf(item);
        if (!r) {
          continue;
        }
        const mid = horizontal ? r.x + r.width / 2 : r.y + r.height / 2;
        if ((horizontal ? x : y) < mid) {
          index = i;
          edge = horizontal ? r.x : r.y;
          break;
        }
        last = horizontal ? r.x + r.width : r.y + r.height;
      }
      const at = edge ?? last ?? (horizontal ? rect.x + 4 : rect.y + 4);
      const indicator: Indicator = horizontal
        ? {
            shape: 'line',
            x: at - LINE / 2 - (edge === undefined ? -2 : 2),
            y: rect.y + 2,
            width: LINE,
            height: rect.height - 4,
          }
        : {
            shape: 'line',
            x: rect.x + 2,
            y: at - LINE / 2 - (edge === undefined ? -2 : 2),
            width: rect.width - 4,
            height: LINE,
          };
      return { slot: { kind: 'item', layout: layoutPath, index }, indicator, container: rect };
    }
    if (isSpacer) {
      return none(point);
    }
    if (cls === 'GridLayout') {
      return gridSlot(layoutPath, items, rectOf, rect, x, y, drag);
    }
    if (cls === 'PlaceLayout') {
      return {
        slot: {
          kind: 'item',
          layout: layoutPath,
          index: items.length,
          x: x - rect.x,
          y: y - rect.y,
        },
        indicator: point,
        container: rect,
      };
    }
    return none(point);
  }
  if (isSpacer) {
    return none(point);
  }
  const info = container.class === undefined ? undefined : findClass(container.class);
  const keys = info?.child_keys ?? [];
  if (keys.includes('tabs')) {
    const tabs = (json['tabs'] as unknown[] | undefined) ?? [];
    return {
      slot: { kind: 'tab', parent: container.path, index: tabs.length },
      indicator: { shape: 'box', ...rect },
      container: rect,
    };
  }
  if (keys.includes('panes')) {
    const panes = (json['panes'] as Json[] | undefined) ?? [];
    const props = (json['properties'] as Json | undefined) ?? {};
    const vertical = props['orientation'] === 'VERTICAL';
    let index = panes.length;
    let at = vertical ? rect.y + rect.height - 4 : rect.x + rect.width - 4;
    for (const [i, pane] of panes.entries()) {
      const w = pane['widget'] as Json | undefined;
      const r = w ? frame.rects[String(w['name'])] : undefined;
      if (r && (vertical ? y < r.y + r.height / 2 : x < r.x + r.width / 2)) {
        index = i;
        at = vertical ? r.y : r.x;
        break;
      }
    }
    const indicator: Indicator = vertical
      ? { shape: 'line', x: rect.x + 2, y: at - LINE / 2, width: rect.width - 4, height: LINE }
      : { shape: 'line', x: at - LINE / 2, y: rect.y + 2, width: LINE, height: rect.height - 4 };
    return { slot: { kind: 'pane', parent: container.path, index }, indicator, container: rect };
  }
  if (keys.includes('content')) {
    const indicator: Indicator = { shape: 'box', ...rect };
    return json['content'] === undefined
      ? { slot: { kind: 'content', parent: container.path }, indicator, container: rect }
      : none(indicator);
  }
  if (info?.accepts_children || container.kind === 'window') {
    return {
      slot: { kind: 'child', parent: container.path, x: x - rect.x, y: y - rect.y },
      indicator: point,
      container: rect,
    };
  }
  return none(point);
}

/** GridLayout のセル。項目の位置から行・列の範囲を求め、点の下のセル(下・右の外なら新しい行・列)にする。 */
function gridSlot(
  layoutPath: JsonPath,
  items: readonly Json[],
  rectOf: (item: Json) => CanvasRect | undefined,
  area: CanvasRect,
  x: number,
  y: number,
  drag: Drag,
): DropTarget {
  const rows = new Map<number, { start: number; end: number }>();
  const columns = new Map<number, { start: number; end: number }>();
  const widen = (
    m: Map<number, { start: number; end: number }>,
    k: number,
    start: number,
    end: number,
  ): void => {
    const old = m.get(k);
    m.set(
      k,
      old ? { start: Math.min(old.start, start), end: Math.max(old.end, end) } : { start, end },
    );
  };
  for (const item of items) {
    const r = rectOf(item);
    if (!r) {
      continue;
    }
    // またがる項目は、行・列の範囲を決めるのに使わない
    if ((item['row_span'] ?? 1) === 1) {
      widen(rows, Number(item['row'] ?? 0), r.y, r.y + r.height);
    }
    if ((item['column_span'] ?? 1) === 1) {
      widen(columns, Number(item['column'] ?? 0), r.x, r.x + r.width);
    }
  }
  const pick = (
    m: Map<number, { start: number; end: number }>,
    v: number,
    areaStart: number,
    areaEnd: number,
  ): { index: number; start: number; end: number } => {
    const sorted = [...m.entries()].sort(([a], [b]) => a - b);
    if (sorted.length === 0) {
      return { index: 0, start: areaStart + 4, end: areaEnd - 4 };
    }
    for (const [k, r] of sorted) {
      if (v < r.end + 4) {
        return { index: k, start: r.start, end: r.end };
      }
    }
    const [lastIndex, lastRange] = sorted[sorted.length - 1] ?? [0, { start: 0, end: 0 }];
    return {
      index: lastIndex + 1,
      start: lastRange.end + 4,
      end: Math.max(lastRange.end + 28, areaEnd - 4),
    };
  };
  const row = pick(rows, y, area.y, area.y + area.height);
  const column = pick(columns, x, area.x, area.x + area.width);
  const indicator: Indicator = {
    shape: 'box',
    x: column.start,
    y: row.start,
    width: Math.max(8, column.end - column.start),
    height: Math.max(8, row.end - row.start),
  };
  // 空いているか(またがる項目も含めて。移す部品自身のセルは空きとみなす)
  const occupied = items.some((item, i) => {
    if (
      drag.kind === 'move' &&
      drag.path.join('/') === [...layoutPath, 'items', i, 'widget'].join('/')
    ) {
      return false;
    }
    const r0 = Number(item['row'] ?? 0);
    const c0 = Number(item['column'] ?? 0);
    const rs = Number(item['row_span'] ?? 1);
    const cs = Number(item['column_span'] ?? 1);
    return row.index >= r0 && row.index < r0 + rs && column.index >= c0 && column.index < c0 + cs;
  });
  return {
    slot: occupied
      ? undefined
      : {
          kind: 'item',
          layout: layoutPath,
          index: items.length,
          row: row.index,
          column: column.index,
        },
    indicator,
    container: area,
  };
}
