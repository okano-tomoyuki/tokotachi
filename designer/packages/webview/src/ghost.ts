/** ドラッグ中の像(docs/designer/editor-design.md §4.1)。 */
import type { CanvasFrame, CanvasRect } from '@tokotachi-designer/core';
import type { Ghost } from './store.ts';

/** キャンバスの画像から、部品の範囲を切り出した像。dx・dy はつかんだ点(部品の左上から。dp)。 */
export function widgetGhost(
  frame: CanvasFrame,
  rect: CanvasRect,
  dx: number,
  dy: number,
): Ghost | undefined {
  const s = frame.scale;
  const x0 = Math.max(0, Math.floor(rect.x * s));
  const y0 = Math.max(0, Math.floor(rect.y * s));
  const w = Math.min(frame.width - x0, Math.ceil(rect.width * s));
  const h = Math.min(frame.height - y0, Math.ceil(rect.height * s));
  if (w <= 0 || h <= 0) {
    return undefined;
  }
  const pixels = new Uint8ClampedArray(w * h * 4);
  for (let y = 0; y < h; y++) {
    const from = ((y0 + y) * frame.width + x0) * 4;
    pixels.set(frame.pixels.subarray(from, from + w * 4), y * w * 4);
  }
  const canvas = document.createElement('canvas');
  canvas.width = w;
  canvas.height = h;
  canvas.getContext('2d')?.putImageData(new ImageData(pixels, w, h), 0, 0);
  return { image: canvas.toDataURL(), width: w / s, height: h / s, dx, dy };
}

/** パレットの部品の像(名前だけ)。 */
export function labelGhost(label: string): Ghost {
  return { label, width: 0, height: 0, dx: -12, dy: -12 };
}
