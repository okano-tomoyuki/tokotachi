/**
 * 右のペイン(docs/designer/editor-design.md §5)。インスペクタを入れ、左の端で幅を変えられる。ツールバーのボタンで隠せる
 * (幅と隠したかは、左のペインと一緒に Webview の状態に保存する)。
 */
import { l10n } from '@tokotachi-designer/core';
import { useRef } from 'react';
import { dragWith } from '../pointerDrag.ts';
import { useDesigner } from '../store.ts';
import { Inspector } from './Inspector.tsx';

const MIN_WIDTH = 200;
const MAX_WIDTH = 720;

export function InspectorPane() {
  const width = useDesigner((s) => s.sidebar.inspectorWidth);
  const hidden = useDesigner((s) => s.sidebar.inspectorHidden);
  const setSidebar = useDesigner((s) => s.setSidebar);
  const ref = useRef<HTMLDivElement>(null);
  if (hidden) {
    return null;
  }
  return (
    <div ref={ref} className="pane inspector-pane" style={{ width }}>
      <div
        className="inspector-resize"
        title={l10n.t('Drag to resize')}
        onPointerDown={(e) => {
          dragWith(e, (x) => {
            const r = ref.current?.getBoundingClientRect();
            if (r) {
              setSidebar({
                inspectorWidth: Math.round(Math.min(MAX_WIDTH, Math.max(MIN_WIDTH, r.right - x))),
              });
            }
          });
        }}
      />
      <div className="inspector-scroll">
        <Inspector />
      </div>
    </div>
  );
}
