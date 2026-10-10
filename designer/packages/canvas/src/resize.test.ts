/** つまみで大きさを変える計算(webview の resize.ts)。 */
import { describe, expect, it } from 'vitest';
import { placementAfterResize, resized } from '../../webview/src/resize.ts';

const R = { x: 100, y: 50, width: 80, height: 40, showing: true };

describe('resize', () => {
  it('つまみの向きで枠を変え、最小の大きさで止める', () => {
    expect(resized(R, 'se', 20, 10)).toMatchObject({ x: 100, y: 50, width: 100, height: 50 });
    expect(resized(R, 'nw', 10, 5)).toMatchObject({ x: 110, y: 55, width: 70, height: 35 });
    expect(resized(R, 'w', 200, 0)).toMatchObject({ x: 172, width: 8 });
  });

  it('左上で合わせるなら、左上が動いた分だけ x・y を変える', () => {
    const to = resized(R, 'nw', -10, -5);
    expect(placementAfterResize({ x: 20, y: 30 }, R, to, 400, 300)).toEqual({
      x: 10,
      y: 25,
      width: 90,
      height: 45,
    });
  });

  it('中央で合わせるなら、中央が動いた分だけ変え、親の割合の分は大きさから除く', () => {
    // 右へ 20 広げると中央は 10 右へ動く
    const to = resized(R, 'e', 20, 0);
    expect(
      placementAfterResize(
        { relative_x: 0.5, anchor: 'CENTER', relative_width: 0.1 },
        R,
        to,
        400,
        300,
      ),
    ).toEqual({
      relative_x: 0.5,
      anchor: 'CENTER',
      relative_width: 0.1,
      x: 10,
      y: 0,
      width: 60,
      height: 40,
    });
  });
});
