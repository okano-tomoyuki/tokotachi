/**
 * 左のペイン(docs/designer/editor-design.md §4)。上にパレット、下に構造の木を置き、間の仕切りで高さの割合を、
 * 右の端で幅を変えられる(変えたものは Webview の状態に保存する)。
 */
import { l10n } from '@tokotachi-designer/core';
import { useRef } from 'react';
import { dragWith } from '../pointerDrag.ts';
import { useDesigner } from '../store.ts';
import { Palette } from './Palette.tsx';
import { Tree } from './Tree.tsx';

const MIN_WIDTH = 160;
const MAX_WIDTH = 480;
const MIN_RATIO = 0.1;
const MAX_RATIO = 0.9;

export function Sidebar() {
  const sidebar = useDesigner((s) => s.sidebar);
  const setSidebar = useDesigner((s) => s.setSidebar);
  const ref = useRef<HTMLDivElement>(null);
  if (sidebar.hidden) {
    return null;
  }
  return (
    <div ref={ref} className="pane sidebar" style={{ width: sidebar.width }}>
      <div
        className="sidebar-palette"
        style={{ flexBasis: `${String(sidebar.paletteRatio * 100)}%` }}
      >
        <Palette />
      </div>
      <div
        className="sidebar-split"
        title={l10n.t('Drag to resize')}
        onPointerDown={(e) => {
          dragWith(e, (_x, y) => {
            const r = ref.current?.getBoundingClientRect();
            if (r && r.height > 0) {
              const ratio = (y - r.top) / r.height;
              setSidebar({ paletteRatio: Math.min(MAX_RATIO, Math.max(MIN_RATIO, ratio)) });
            }
          });
        }}
      />
      <div className="sidebar-structure">
        <div className="section-head">{l10n.t('Structure')}</div>
        <Tree />
      </div>
      <div
        className="sidebar-resize"
        title={l10n.t('Drag to resize')}
        onPointerDown={(e) => {
          dragWith(e, (x) => {
            const r = ref.current?.getBoundingClientRect();
            if (r) {
              setSidebar({
                width: Math.round(Math.min(MAX_WIDTH, Math.max(MIN_WIDTH, x - r.left))),
              });
            }
          });
        }}
      />
    </div>
  );
}
