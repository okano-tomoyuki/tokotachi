/** 仕切りのドラッグ(左のペインの幅・高さの割合を変える)。 */
import type { PointerEvent } from 'react';

/** つかんだ要素の上でポインタを動かしている間、move を呼ぶ(離すまでポインタを捕まえておく)。 */
export function dragWith(e: PointerEvent<HTMLElement>, move: (x: number, y: number) => void): void {
  if (e.button !== 0) {
    return;
  }
  e.preventDefault();
  const target = e.currentTarget;
  target.setPointerCapture(e.pointerId);
  const onMove = (ev: globalThis.PointerEvent): void => {
    move(ev.clientX, ev.clientY);
  };
  const onUp = (): void => {
    target.removeEventListener('pointermove', onMove);
    target.removeEventListener('pointerup', onUp);
    target.removeEventListener('pointercancel', onUp);
  };
  target.addEventListener('pointermove', onMove);
  target.addEventListener('pointerup', onUp);
  target.addEventListener('pointercancel', onUp);
}
