/**
 * 大きさを変えるつまみ(docs/designer/editor-design.md §4.1)。位置と大きさを書ける部品(PlaceLayout の項目・自由な配置の子)を
 * 選んだときだけ、選択の枠の 8 か所に出す。ドラッグの間は新しい枠を出し、離したら書く。
 * - 自由な配置: プロパティの x・y・width・height。
 * - PlaceLayout: placement の x・y・width・height(anchor で合わせる点が動いた分だけ x・y を変え、width・height は親の割合の分を除く)。
 */
import {
  valueAt,
  type CanvasRect,
  type OutlineNode,
  type PropertyValue,
} from '@tokotachi-designer/core';
import { useState } from 'react';
import { placementAfterResize, resized, type Edge } from '../resize.ts';
import { sendEdit, useDesigner } from '../store.ts';

interface Json {
  readonly [key: string]: unknown;
}

const EDGES: readonly Edge[] = ['n', 's', 'e', 'w', 'ne', 'nw', 'se', 'sw'];

/** つまみで大きさを変えられるか(自由な配置の子か、PlaceLayout の項目)。 */
export function resizeMode(node: OutlineNode | undefined): 'free' | 'place' | undefined {
  if (node?.kind !== 'widget') {
    return undefined;
  }
  const path = node.path;
  if (path[path.length - 2] === 'children') {
    return 'free';
  }
  if (path[path.length - 1] === 'widget' && path[path.length - 3] === 'items') {
    const layout = valueAt(useDesigner.getState().document, path.slice(0, -3)) as Json | undefined;
    return layout?.['class'] === 'PlaceLayout' ? 'place' : undefined;
  }
  return undefined;
}

/** 新しい枠を定義ファイルに書く。 */
function commit(node: OutlineNode, mode: 'free' | 'place', from: CanvasRect, to: CanvasRect): void {
  const { document, frame } = useDesigner.getState();
  const round = (v: number): number => Math.round(v);
  if (mode === 'free') {
    const props = ((valueAt(document, node.path) as Json | undefined)?.['properties'] ??
      {}) as Record<string, PropertyValue>;
    const x = typeof props['x'] === 'number' ? props['x'] : 0;
    const y = typeof props['y'] === 'number' ? props['y'] : 0;
    sendEdit({
      kind: 'setProperties',
      target: node.path,
      properties: {
        x: round(x + to.x - from.x),
        y: round(y + to.y - from.y),
        width: round(to.width),
        height: round(to.height),
      },
    });
    return;
  }
  const itemPath = node.path.slice(0, -1);
  const item = valueAt(document, itemPath) as Json | undefined;
  const p = (item?.['placement'] ?? {}) as Record<string, PropertyValue>;
  // 親(レイアウトが付いている部品。ウィンドウなら全体)の大きさ。割合の分を除くのに使う
  const ownerPath = node.path.slice(0, -4);
  const owner =
    ownerPath.length === 1
      ? {
          width: (frame?.width ?? 0) / (frame?.scale ?? 1),
          height: (frame?.height ?? 0) / (frame?.scale ?? 1),
        }
      : frame?.rects[String((valueAt(document, ownerPath) as Json | undefined)?.['name'])];
  const parentWidth = owner?.width ?? 0;
  const parentHeight = owner?.height ?? 0;
  const placement = placementAfterResize(p, from, to, parentWidth, parentHeight);
  sendEdit({ kind: 'setItem', target: itemPath, values: { placement } });
}

export function ResizeHandles({ node, rect }: { node: OutlineNode; rect: CanvasRect }) {
  const [preview, setPreview] = useState<CanvasRect | undefined>();
  const mode = resizeMode(node);
  if (!mode) {
    return null;
  }
  const shown = preview ?? rect;
  const start = (edge: Edge, e: React.MouseEvent): void => {
    e.preventDefault();
    e.stopPropagation();
    const sx = e.clientX;
    const sy = e.clientY;
    let last = rect;
    const zoom = useDesigner.getState().zoom;
    const move = (m: MouseEvent): void => {
      last = resized(rect, edge, (m.clientX - sx) / zoom, (m.clientY - sy) / zoom);
      setPreview(last);
    };
    const up = (): void => {
      window.removeEventListener('mousemove', move);
      window.removeEventListener('mouseup', up);
      setPreview(undefined);
      if (last !== rect) {
        commit(node, mode, rect, last);
      }
    };
    window.addEventListener('mousemove', move);
    window.addEventListener('mouseup', up);
  };
  const handle = (edge: Edge): React.CSSProperties => {
    const x = edge.includes('w') ? 0 : edge.includes('e') ? shown.width : shown.width / 2;
    const y = edge.includes('n') ? 0 : edge.includes('s') ? shown.height : shown.height / 2;
    return { left: shown.x + x - 4, top: shown.y + y - 4 };
  };
  return (
    <>
      {preview && (
        <div
          className="resize-preview"
          style={{ left: preview.x, top: preview.y, width: preview.width, height: preview.height }}
        />
      )}
      {EDGES.map((edge) => (
        <div
          key={edge}
          className={`resize-handle ${edge}`}
          style={handle(edge)}
          onMouseDown={(e) => {
            start(edge, e);
          }}
          onClick={(e) => {
            e.stopPropagation();
          }}
        />
      ))}
    </>
  );
}
