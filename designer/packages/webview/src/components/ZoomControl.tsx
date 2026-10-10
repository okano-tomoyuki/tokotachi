/** キャンバスの倍率(50%〜200%)。Ctrl+ホイールでも変えられる(CanvasView)。 */
import { l10n } from '@tokotachi-designer/core';
import { useDesigner } from '../store.ts';

export const ZOOM_LEVELS = [0.5, 0.75, 1, 1.25, 1.5, 2] as const;

/** 1 段階大きく(dir = 1)・小さく(-1)した倍率。 */
export function stepZoom(zoom: number, dir: number): number {
  const i = ZOOM_LEVELS.findIndex((z) => z >= zoom - 1e-6);
  const next = Math.max(
    0,
    Math.min(ZOOM_LEVELS.length - 1, (i < 0 ? ZOOM_LEVELS.length - 1 : i) + dir),
  );
  return ZOOM_LEVELS[next] ?? 1;
}

export function ZoomControl() {
  const zoom = useDesigner((s) => s.zoom);
  return (
    <select
      className="zoom"
      title={l10n.t('Zoom (Ctrl+wheel on the canvas)')}
      value={zoom}
      onChange={(e) => {
        useDesigner.setState({ zoom: Number(e.target.value) });
      }}
    >
      {ZOOM_LEVELS.map((z) => (
        <option key={z} value={z}>
          {`${String(Math.round(z * 100))}%`}
        </option>
      ))}
    </select>
  );
}
