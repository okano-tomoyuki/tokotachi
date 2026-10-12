/**
 * キャンバスの部品の位置から求めるもの(docs/adr/0052)。範囲で囲んで選ぶ部品と、入れ物の外にはみ出した部品。
 */
import {
  flattenOutline,
  isSubclassOf,
  type CanvasRect,
  type OutlineNode,
} from '@tokotachi-designer/core';

export interface Box {
  readonly x: number;
  readonly y: number;
  readonly width: number;
  readonly height: number;
}

/** 2 点を角とする四角。 */
export function boxOf(x0: number, y0: number, x1: number, y1: number): Box {
  return {
    x: Math.min(x0, x1),
    y: Math.min(y0, y1),
    width: Math.abs(x1 - x0),
    height: Math.abs(y1 - y0),
  };
}

function intersects(a: Box, b: Box): boolean {
  return a.x < b.x + b.width && b.x < a.x + a.width && a.y < b.y + b.height && b.y < a.y + a.height;
}

/** 部品(とウィンドウ)を id で引く表。 */
function widgetIndex(roots: readonly OutlineNode[]): Map<string, OutlineNode> {
  return new Map(
    flattenOutline(roots)
      .filter((n) => n.kind === 'widget' || n.kind === 'window')
      .map((n) => [n.id, n]),
  );
}

/** いちばん近い入れ物(祖先の部品かウィンドウ。間のレイアウト・タブ・区画は飛ばす)。 */
function containerOf(
  node: OutlineNode,
  index: ReadonlyMap<string, OutlineNode>,
): OutlineNode | undefined {
  for (let k = node.path.length - 1; k > 0; k--) {
    const found = index.get(node.path.slice(0, k).join('/'));
    if (found) {
      return found;
    }
  }
  return undefined;
}

/**
 * 範囲で囲んで選ぶ入れ物(点の下のノードから、いちばん近い部品かウィンドウ)。
 * from が部品なら、その部品を入れ物にする(Ctrl を押して部品の中から囲み始めたとき)。
 */
export function bandContainer(
  roots: readonly OutlineNode[],
  from: OutlineNode | undefined,
): string {
  if (!from) {
    return 'window';
  }
  if (from.kind === 'window') {
    return from.id;
  }
  const index = widgetIndex(roots);
  // 中に部品を入れた部品なら、それを入れ物にする
  if (
    from.kind === 'widget' &&
    [...index.values()].some((n) => n.kind === 'widget' && containerOf(n, index)?.id === from.id)
  ) {
    return from.id;
  }
  return containerOf(from, index)?.id ?? 'window';
}

/** 入れ物 container の直接の部品のうち、四角 band に掛かるもの(木の順)。 */
export function bandSelection(
  roots: readonly OutlineNode[],
  rects: Readonly<Record<string, CanvasRect>>,
  container: string,
  band: Box,
): string[] {
  const index = widgetIndex(roots);
  return [...index.values()].flatMap((n) => {
    const r = n.variable === undefined ? undefined : rects[n.variable];
    return n.kind === 'widget' &&
      r?.showing === true &&
      containerOf(n, index)?.id === container &&
      intersects(r, band)
      ? [n.id]
      : [];
  });
}

/** 入れ物の外にはみ出した部品と、その入れ物の名前(ScrollArea の中身のように、はみ出すのが普通のものは除く)。 */
export function outsideParent(
  roots: readonly OutlineNode[],
  rects: Readonly<Record<string, CanvasRect>>,
  window: Box,
): { id: string; parent: string; rect: CanvasRect }[] {
  const index = widgetIndex(roots);
  // 測り方の端数で出ないように、少しの差は許す
  const slack = 0.5;
  return [...index.values()].flatMap((n) => {
    const r = n.variable === undefined ? undefined : rects[n.variable];
    if (n.kind !== 'widget' || r?.showing !== true || r.width <= 0 || r.height <= 0) {
      return [];
    }
    const container = containerOf(n, index);
    if (
      !container ||
      (container.class !== undefined && isSubclassOf(container.class, 'ScrollArea'))
    ) {
      return [];
    }
    const outer =
      container.kind === 'window'
        ? window
        : container.variable === undefined
          ? undefined
          : rects[container.variable];
    if (!outer) {
      return [];
    }
    const inside =
      r.x >= outer.x - slack &&
      r.y >= outer.y - slack &&
      r.x + r.width <= outer.x + outer.width + slack &&
      r.y + r.height <= outer.y + outer.height + slack;
    return inside ? [] : [{ id: n.id, parent: container.label, rect: r }];
  });
}
