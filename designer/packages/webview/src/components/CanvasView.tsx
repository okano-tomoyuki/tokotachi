/**
 * キャンバス(docs/designer/editor-design.md §4)。拡張が WebAssembly の Tokotachi で描いた画像を表示し、選択の枠を重ねる。
 * クリックで部品を選び、ダブルクリックでその部品の主なハンドラのコードへ移動する。
 */
import {
  defaultEventOf,
  defaultHandlerName,
  findOutlineNode,
  flattenOutline,
  l10n,
  type CanvasRect,
  type OutlineNode,
  valueAt,
} from '@tokotachi-designer/core';
import { useEffect, useMemo, useRef } from 'react';
import { beginDrag } from '../dragController.ts';
import { dropTarget } from '../dropTarget.ts';
import { widgetGhost } from '../ghost.ts';
import { ResizeHandles } from './ResizeHandles.tsx';
import { stepZoom } from './ZoomControl.tsx';
import { selectedNode, sendEdit, useDesigner } from '../store.ts';
import { postMessage } from '../vscode.ts';

/** ノードの枠に使う部品の位置(レイアウト・タブ・区画は、付いている部品・中の部品の位置)。 */
function rectOf(
  node: OutlineNode | undefined,
  roots: readonly OutlineNode[],
  rects: Readonly<Record<string, CanvasRect>>,
): CanvasRect | undefined {
  if (!node) {
    return undefined;
  }
  if (node.variable !== undefined && node.variable in rects) {
    return rects[node.variable];
  }
  if (node.kind === 'tab' || node.kind === 'pane') {
    return rectOf(node.children[0], roots, rects);
  }
  if (node.kind === 'column') {
    // 列は一覧の部品の枠を出す
    return rectOf(findOutlineNode(roots, node.path.slice(0, -2).join('/')), roots, rects);
  }
  if (node.kind === 'layout') {
    // 付いている部品(パスの親)。ウィンドウなら undefined(全体)
    const owner = findOutlineNode(roots, node.path.slice(0, -1).join('/'));
    return owner?.kind === 'window' ? undefined : rectOf(owner, roots, rects);
  }
  return undefined;
}

export function CanvasView() {
  const frame = useDesigner((s) => s.frame);
  const canvasError = useDesigner((s) => s.canvasError);
  const roots = useDesigner((s) => s.outline);
  const selected = useDesigner(selectedNode);
  const select = useDesigner((s) => s.select);
  const drag = useDesigner((s) => s.drag);
  const target = useDesigner((s) => s.target);
  const ref = useRef<HTMLCanvasElement>(null);
  const scrollRef = useRef<HTMLDivElement>(null);
  const zoom = useDesigner((s) => s.zoom);

  useEffect(() => {
    // Ctrl+ホイールで倍率を変える(React の wheel は passive なので、既定の動作を止めるために直接つなぐ)
    const element = scrollRef.current;
    if (!element) {
      return;
    }
    const wheel = (e: WheelEvent): void => {
      if (!e.ctrlKey) {
        return;
      }
      e.preventDefault();
      const current = useDesigner.getState().zoom;
      useDesigner.setState({ zoom: stepZoom(current, e.deltaY < 0 ? 1 : -1) });
    };
    element.addEventListener('wheel', wheel, { passive: false });
    return () => {
      element.removeEventListener('wheel', wheel);
    };
  });

  useEffect(() => {
    const canvas = ref.current;
    if (!canvas || !frame) {
      return;
    }
    canvas.width = frame.width;
    canvas.height = frame.height;
    const image = new ImageData(new Uint8ClampedArray(frame.pixels), frame.width, frame.height);
    canvas.getContext('2d')?.putImageData(image, 0, 0);
  }, [frame]);

  // 部品(変数の位置があるもの)を、親が先の順に
  const widgets = useMemo(
    () => flattenOutline(roots).filter((n) => n.variable !== undefined),
    [roots],
  );

  if (!frame) {
    return <div className="canvas-message">{canvasError ?? l10n.t('Drawing...')}</div>;
  }
  const scale = frame.scale;
  const width = frame.width / scale;
  const height = frame.height / scale;

  /** 点の下の部品(いちばん小さいもの。同じ大きさなら木の後ろ(子)のもの)。 */
  const hit = (x: number, y: number): OutlineNode | undefined => {
    let best: OutlineNode | undefined;
    let bestArea = Infinity;
    for (const n of widgets) {
      const r = n.variable === undefined ? undefined : frame.rects[n.variable];
      if (!r?.showing || x < r.x || y < r.y || x >= r.x + r.width || y >= r.y + r.height) {
        continue;
      }
      const area = r.width * r.height;
      if (area <= bestArea) {
        best = n;
        bestArea = area;
      }
    }
    return best;
  };

  // 倍率(CSS の zoom)を除いた、キャンバスの座標(dp)
  const point = (e: React.MouseEvent): { x: number; y: number } => {
    // 画面上の大きさと論理の大きさの比で割る(倍率の掛け方によらず正しい位置になる)
    const box = e.currentTarget.getBoundingClientRect();
    const sx = box.width > 0 ? box.width / width : 1;
    const sy = box.height > 0 ? box.height / height : 1;
    return { x: (e.clientX - box.left) / sx, y: (e.clientY - box.top) / sy };
  };

  const selectionRect = rectOf(selected, roots, frame.rects);
  const document = useDesigner.getState().document;
  return (
    <div className="canvas-scroll" ref={scrollRef}>
      <div className="window-frame" style={{ width: width * zoom }}>
        <div className="window-title">{findOutlineNode(roots, 'window')?.label}</div>
        {/* 倍率は transform で掛け、スクロールの範囲は外側の大きさで合わせる */}
        <div className="zoom-box" style={{ width: width * zoom, height: height * zoom }}>
          <div
            className="canvas-surface"
            style={{ width, height, transform: `scale(${String(zoom)})`, transformOrigin: '0 0' }}
            onMouseDown={(e) => {
              // 部品を押したまま動かしたら、移すドラッグにする
              if (e.button !== 0) {
                return;
              }
              const p = point(e);
              const node = hit(p.x, p.y);
              if (node?.kind === 'widget') {
                const r = node.variable === undefined ? undefined : frame.rects[node.variable];
                const ghost = r ? widgetGhost(frame, r, p.x - r.x, p.y - r.y) : undefined;
                beginDrag({ kind: 'move', path: node.path, id: node.id }, e, undefined, ghost);
              }
            }}
            onMouseMove={(e) => {
              if (!drag || !document) {
                return;
              }
              const p = point(e);
              useDesigner.setState({ target: dropTarget(document, roots, frame, p.x, p.y, drag) });
            }}
            onMouseLeave={() => {
              if (drag) {
                useDesigner.setState({ target: undefined });
              }
            }}
            onClick={(e) => {
              const p = point(e);
              select(hit(p.x, p.y)?.id ?? 'window');
            }}
            onDoubleClick={(e) => {
              const p = point(e);
              const node = hit(p.x, p.y) ?? findOutlineNode(roots, 'window');
              if (!node?.class) {
                return;
              }
              const event = defaultEventOf(node.class);
              if (event === undefined) {
                return;
              }
              // ハンドラが無ければ既定の名前で設定してから移動する
              const events = (
                valueAt(document, node.path) as { events?: Record<string, string> } | undefined
              )?.events;
              const handler =
                events?.[event] ??
                defaultHandlerName(node.kind === 'window' ? undefined : node.label, event);
              if (events?.[event] === undefined) {
                sendEdit({ kind: 'setEvent', target: node.path, event, handler });
              }
              postMessage({ type: 'goToHandler', handler });
            }}
          >
            <canvas ref={ref} style={{ width, height }} />
            {selectionRect && !drag && (
              <div
                className="selection"
                style={{
                  left: selectionRect.x,
                  top: selectionRect.y,
                  width: selectionRect.width,
                  height: selectionRect.height,
                }}
              />
            )}
            {selectionRect && !drag && selected && (
              <ResizeHandles node={selected} rect={selectionRect} />
            )}
            {target && (
              <>
                <div
                  className="drop-container"
                  style={{
                    left: target.container.x,
                    top: target.container.y,
                    width: target.container.width,
                    height: target.container.height,
                  }}
                />
                <div
                  className={`drop-indicator ${target.indicator.shape}${target.slot ? '' : ' invalid'}`}
                  style={{
                    left: target.indicator.x,
                    top: target.indicator.y,
                    width: target.indicator.width,
                    height: target.indicator.height,
                  }}
                />
              </>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}
