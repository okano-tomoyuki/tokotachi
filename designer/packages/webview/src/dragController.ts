/**
 * ドラッグの始まりから離すまで(docs/designer/editor-design.md §4.1)。パレットの部品・空きと、キャンバスの部品を扱う。
 * 4px より動かしたらドラッグとし、キャンバスの上では入れる場所(dropTarget)を出し続け、離したらコマンドを送る。
 * 動かさずに離したら、クリックとして扱う(パレットなら、選んでいる部品の後ろ・入れ物の終わりに足す)。
 */
import {
  endOfLayout,
  findOutlineNode,
  newWidget,
  valueAt,
  type OutlineNode,
  type Slot,
  type TtuiDocument,
  type JsonPath,
} from '@tokotachi-designer/core';
import type { Drag } from './dropTarget.ts';
import { sendEdit, useDesigner, type Ghost } from './store.ts';

const THRESHOLD = 4;

/**
 * マウスのボタンを押したときに呼ぶ。
 * @param onClick 動かさずに離したときに呼ぶ
 */
export function beginDrag(
  drag: Drag,
  event: { clientX: number; clientY: number },
  onClick?: () => void,
  ghost?: Ghost,
): void {
  const startX = event.clientX;
  const startY = event.clientY;
  let active = false;
  const move = (e: MouseEvent): void => {
    if (!active && Math.hypot(e.clientX - startX, e.clientY - startY) > THRESHOLD) {
      active = true;
      useDesigner.setState({ drag, target: undefined, ghost });
    }
    if (active) {
      useDesigner.setState({ pointer: { x: e.clientX, y: e.clientY } });
    }
  };
  const finish = (commit: boolean): void => {
    window.removeEventListener('mousemove', move);
    window.removeEventListener('mouseup', up);
    window.removeEventListener('keydown', key);
    if (active && commit) {
      drop();
    }
    useDesigner.setState({
      drag: undefined,
      target: undefined,
      ghost: undefined,
      pointer: undefined,
    });
    if (!active && commit) {
      onClick?.();
    }
  };
  const up = (): void => {
    finish(true);
  };
  const key = (e: KeyboardEvent): void => {
    if (e.key === 'Escape') {
      finish(false);
    }
  };
  window.addEventListener('mousemove', move);
  window.addEventListener('mouseup', up);
  window.addEventListener('keydown', key);
}

/** ドラッグしていればその中か(キャンバスでクリックの選択を飛ばすため)。 */
export function isDragging(): boolean {
  return useDesigner.getState().drag !== undefined;
}

/** 入れる場所に、ドラッグしているものを入れる。 */
function drop(): void {
  const { drag, target, document } = useDesigner.getState();
  if (!drag || !target?.slot || !document) {
    return;
  }
  insert(document, drag, target.slot);
}

function insert(document: TtuiDocument, drag: Drag, slot: Slot): void {
  switch (drag.kind) {
    case 'new': {
      const widget = newWidget(document, drag.class);
      sendEdit({ kind: 'addWidget', slot, widget }, widget.name);
      return;
    }
    case 'spacer':
      if (slot.kind === 'item') {
        sendEdit({
          kind: 'addSpacer',
          layout: slot.layout,
          index: slot.index,
          spacer: drag.spacer,
        });
      }
      return;
    case 'move': {
      const nameOf = (path: JsonPath): string | undefined =>
        (valueAt(document, path) as { name?: string } | undefined)?.name;
      if (drag.group && drag.group.length > 1) {
        // 移した後も同じ部品を選ぶ(主に選んでいた、つかんだ部品を最初に)
        const names = [drag.path, ...drag.group.map((g) => g.path)].flatMap((p) => {
          const n = nameOf(p);
          return n === undefined ? [] : [n];
        });
        sendEdit({ kind: 'moveWidgets', targets: drag.group.map((g) => g.path), slot }, [
          ...new Set(names),
        ]);
        return;
      }
      sendEdit({ kind: 'moveWidget', target: drag.path, slot }, nameOf(drag.path));
      return;
    }
  }
}

interface Json {
  readonly [key: string]: unknown;
}

/**
 * クリックで足す場所: 選んでいるのがレイアウトの中の部品(BoxLayout)ならその後ろ、レイアウトを持つ入れ物ならその終わり、
 * それ以外はウィンドウのレイアウトの終わり(レイアウトが無ければ左上)。
 */
export function slotForClick(
  document: TtuiDocument,
  roots: readonly OutlineNode[],
  selection: string | undefined,
): Slot | undefined {
  const node = selection === undefined ? undefined : findOutlineNode(roots, selection);
  const path = node?.path ?? ['window'];
  const json = valueAt(document, path) as Json | undefined;
  if (json?.['layout'] !== undefined) {
    return endOfLayout(document, [...path, 'layout']);
  }
  if (node?.kind === 'layout') {
    return endOfLayout(document, path);
  }
  if (path[path.length - 1] === 'widget' && path[path.length - 3] === 'items') {
    const layoutPath = path.slice(0, -3);
    const layout = valueAt(document, layoutPath) as Json | undefined;
    if (layout?.['class'] === 'BoxLayout') {
      return { kind: 'item', layout: layoutPath, index: Number(path[path.length - 2]) + 1 };
    }
    return endOfLayout(document, layoutPath);
  }
  if (document.window.layout) {
    return endOfLayout(document, ['window', 'layout']);
  }
  return { kind: 'child', parent: ['window'], x: 8, y: 8 };
}

/** パレットのクリック: 選んでいる場所に足す。 */
export function clickAdd(drag: Drag): void {
  const { document, outline, selection } = useDesigner.getState();
  if (!document) {
    return;
  }
  const slot = slotForClick(document, outline, selection);
  if (slot) {
    insert(document, drag, slot);
  }
}
