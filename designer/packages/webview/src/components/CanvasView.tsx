/**
 * キャンバス(docs/designer/editor-design.md §4)。拡張が WebAssembly の Tokotachi で描いた画像を表示し、選択の枠を重ねる。
 * クリックで部品を選び、ダブルクリックでその部品の主なハンドラのコードへ移動する。
 */
import {
  defaultEventOf,
  defaultHandlerName,
  findClass,
  findOutlineNode,
  flattenOutline,
  l10n,
  type CanvasRect,
  type OutlineNode,
  valueAt,
} from '@tokotachi-designer/core';
import { useEffect, useMemo, useRef, useState } from 'react';
import { bandContainer, bandSelection, boxOf, outsideParent, type Box } from '../canvasGeometry.ts';
import { beginDrag } from '../dragController.ts';
import { dropTarget } from '../dropTarget.ts';
import { widgetGhost } from '../ghost.ts';
import { ResizeHandles } from './ResizeHandles.tsx';
import { stepZoom } from './ZoomControl.tsx';
import { selectedNode, selectedNodes, sendEdit, useDesigner } from '../store.ts';
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
  const selectedIds = useDesigner((s) => s.selected);
  const select = useDesigner((s) => s.select);
  const toggleSelect = useDesigner((s) => s.toggleSelect);
  const drag = useDesigner((s) => s.drag);
  const target = useDesigner((s) => s.target);
  const ref = useRef<HTMLCanvasElement>(null);
  const scrollRef = useRef<HTMLDivElement>(null);
  const zoom = useDesigner((s) => s.zoom);
  // 範囲で囲んで選んでいる四角(キャンバスの座標)。囲み終えた直後のクリックは、選択を変えない
  const [band, setBand] = useState<Box | undefined>(undefined);
  const bandClicked = useRef(false);

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

  /**
   * 範囲で囲んで選び始める(空いた所か、Ctrl・Shift を押して押したとき)。少し動かすまでは始めない(ただのクリックは onClick で扱う)。
   * additive なら、今の選択に足す。
   */
  const beginBand = (
    e: React.MouseEvent,
    from: OutlineNode | undefined,
    additive: boolean,
  ): void => {
    e.preventDefault();
    const surface = e.currentTarget;
    const start = point(e);
    const startX = e.clientX;
    const startY = e.clientY;
    const container = bandContainer(roots, from);
    const base = additive ? useDesigner.getState().selected.filter((id) => id !== 'window') : [];
    let active = false;
    const toCanvas = (ev: MouseEvent): { x: number; y: number } => {
      const box = surface.getBoundingClientRect();
      const sx = box.width > 0 ? box.width / width : 1;
      const sy = box.height > 0 ? box.height / height : 1;
      return { x: (ev.clientX - box.left) / sx, y: (ev.clientY - box.top) / sy };
    };
    const move = (ev: MouseEvent): void => {
      if (!active && Math.hypot(ev.clientX - startX, ev.clientY - startY) < 4) {
        return;
      }
      active = true;
      const p = toCanvas(ev);
      const b = boxOf(start.x, start.y, p.x, p.y);
      setBand(b);
      // 囲んでいる間も、選ぶものを見せる
      const ids = bandSelection(roots, frame.rects, container, b);
      const selected = [...base, ...ids.filter((id) => !base.includes(id))];
      useDesigner.setState({
        selected: selected.length > 0 ? selected : ['window'],
        selection: selected[0] ?? 'window',
      });
    };
    const up = (): void => {
      window.removeEventListener('mousemove', move);
      window.removeEventListener('mouseup', up);
      if (active) {
        setBand(undefined);
        // この後に届くクリックで選び直さない(届かないこともあるので、すぐ戻す)
        bandClicked.current = true;
        setTimeout(() => {
          bandClicked.current = false;
        }, 0);
      }
    };
    window.addEventListener('mousemove', move);
    window.addEventListener('mouseup', up);
  };

  // 入れ物の外にはみ出した部品(入れ物の外は描かれない)
  const outside = outsideParent(roots, frame.rects, { x: 0, y: 0, width, height });

  const selectionRect = rectOf(selected, roots, frame.rects);
  // 選んでいるものすべての枠(複数選択)
  const selectedRects = selectedIds.flatMap((id) => {
    const r = rectOf(findOutlineNode(roots, id), roots, frame.rects);
    return r ? [r] : [];
  });
  const document = useDesigner.getState().document;
  // 自作部品(docs/adr/0044)のうち、キャンバスが基底(Widget・PaintBox)で描いて中身が見えないものに、枠とクラスの名前を重ねる
  const customs = widgets.flatMap((n) => {
    const base = n.class === undefined ? undefined : findClass(n.class)?.custom?.base;
    const r = n.variable === undefined ? undefined : frame.rects[n.variable];
    return base !== undefined && (base === 'Widget' || base === 'PaintBox') && r?.showing
      ? [{ id: n.id, name: n.class ?? '', r }]
      : [];
  });
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
              // 空いた所・レイアウトの上からか、Ctrl・Shift を押してドラッグしたら、範囲で囲んで選ぶ
              const additive = e.ctrlKey || e.metaKey || e.shiftKey;
              if (additive || node?.kind !== 'widget') {
                beginBand(e, node, additive);
                return;
              }
              const r = node.variable === undefined ? undefined : frame.rects[node.variable];
              const ghost = r ? widgetGhost(frame, r, p.x - r.x, p.y - r.y) : undefined;
              // 選んでいる部品をつかんだら、選んでいる部品をまとめて移す(中に入っているものは親ごと)
              const group = selectedIds.includes(node.id)
                ? selectedNodes(useDesigner.getState())
                    .filter((n) => n.kind === 'widget')
                    .filter((n, _, all) => !all.some((o) => o !== n && n.id.startsWith(`${o.id}/`)))
                : [];
              beginDrag(
                {
                  kind: 'move',
                  path: node.path,
                  id: node.id,
                  ...(group.length > 1
                    ? { group: group.map((n) => ({ path: n.path, id: n.id })) }
                    : {}),
                },
                e,
                undefined,
                ghost,
              );
            }}
            onMouseMove={(e) => {
              if (!drag || !document) {
                return;
              }
              const p = point(e);
              // 部品を移すときは、つかんだ点の分ずらして置く(ドラッグ中の像と同じ位置)
              const ghost = useDesigner.getState().ghost;
              const grab =
                drag.kind === 'move' && ghost?.image !== undefined
                  ? { dx: ghost.dx, dy: ghost.dy, width: ghost.width, height: ghost.height }
                  : undefined;
              useDesigner.setState({
                target: dropTarget(document, roots, frame, p.x, p.y, drag, grab),
              });
            }}
            onMouseLeave={() => {
              if (drag) {
                useDesigner.setState({ target: undefined });
              }
            }}
            onClick={(e) => {
              if (bandClicked.current) {
                return;
              }
              const p = point(e);
              const id = hit(p.x, p.y)?.id ?? 'window';
              // Ctrl・Shift+クリックで選択に足す・外す
              if ((e.ctrlKey || e.metaKey || e.shiftKey) && id !== 'window') {
                toggleSelect(id);
              } else {
                select(id);
              }
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
            {customs.map(({ id, name, r }) => (
              <div
                key={id}
                className="custom-placeholder"
                style={{ left: r.x, top: r.y, width: r.width, height: r.height }}
              >
                {name}
              </div>
            ))}
            {!drag &&
              outside.map(({ id, rect: r }) => (
                <div
                  key={`outside-${id}`}
                  className="outside-parent"
                  style={{ left: r.x, top: r.y, width: r.width, height: r.height }}
                />
              ))}
            {!drag &&
              selectedRects.map((r, i) => (
                <div
                  key={i}
                  className={`selection${selectedRects.length > 1 ? ' multiple' : ''}`}
                  style={{ left: r.x, top: r.y, width: r.width, height: r.height }}
                />
              ))}
            {selectionRect && !drag && selected && selectedIds.length <= 1 && (
              <ResizeHandles node={selected} rect={selectionRect} />
            )}
            {band && (
              <div
                className="rubber-band"
                style={{ left: band.x, top: band.y, width: band.width, height: band.height }}
              />
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
