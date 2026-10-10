/** つまみで大きさを変える計算(docs/designer/editor-design.md §4.1)。座標は dp。 */
import type { CanvasRect, PropertyValue } from '@tokotachi-designer/core';

export type Edge = 'n' | 's' | 'e' | 'w' | 'ne' | 'nw' | 'se' | 'sw';

const MIN = 8;

/** anchor の、合わせる点の位置(部品の左上からの割合)。 */
const ANCHORS: Readonly<Record<string, readonly [number, number]>> = {
  TOP_LEFT: [0, 0],
  TOP: [0.5, 0],
  TOP_RIGHT: [1, 0],
  LEFT: [0, 0.5],
  CENTER: [0.5, 0.5],
  RIGHT: [1, 0.5],
  BOTTOM_LEFT: [0, 1],
  BOTTOM: [0.5, 1],
  BOTTOM_RIGHT: [1, 1],
};

/** つまみを (dx, dy) 動かした後の枠(最小 8dp)。 */
export function resized(r: CanvasRect, edge: Edge, dx: number, dy: number): CanvasRect {
  let { x, y, width, height } = r;
  if (edge.includes('e')) {
    width = Math.max(MIN, width + dx);
  }
  if (edge.includes('s')) {
    height = Math.max(MIN, height + dy);
  }
  if (edge.includes('w')) {
    const w = Math.max(MIN, width - dx);
    x += width - w;
    width = w;
  }
  if (edge.includes('n')) {
    const h = Math.max(MIN, height - dy);
    y += height - h;
    height = h;
  }
  return { ...r, x, y, width, height };
}

/**
 * PlaceLayout の置き方を、枠を from から to に変えたものにする。
 * anchor で合わせる点が動いた分だけ x・y を変え、width・height は親の大きさの割合(relative_width 等)の分を除いて書く。
 */
export function placementAfterResize(
  p: Readonly<Record<string, PropertyValue>>,
  from: CanvasRect,
  to: CanvasRect,
  parentWidth: number,
  parentHeight: number,
): Record<string, PropertyValue> {
  const num = (k: string): number => {
    const v = p[k];
    return typeof v === 'number' ? v : 0;
  };
  const anchor = p['anchor'];
  const [ax, ay] = ANCHORS[typeof anchor === 'string' ? anchor : 'TOP_LEFT'] ?? [0, 0];
  return {
    ...p,
    x: Math.round(num('x') + to.x + ax * to.width - (from.x + ax * from.width)),
    y: Math.round(num('y') + to.y + ay * to.height - (from.y + ay * from.height)),
    width: Math.round(to.width - num('relative_width') * parentWidth),
    height: Math.round(to.height - num('relative_height') * parentHeight),
  };
}
