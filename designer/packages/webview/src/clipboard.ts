/**
 * デザイナーのコピー・切り取り・貼り付け・複製(docs/designer/editor-design.md §3.6)。中身は core の clipboard.ts で作る。
 * Ctrl+C・X・V は VS Code が Webview の copy・cut・paste のイベントにするので、それを受ける。右クリックのメニューからは
 * Clipboard API で読み書きし、使えなければこの画面で最後にコピーしたものを使う。
 */
import {
  canCopy,
  copyNodes,
  parseClipboard,
  preparePaste,
  serializeClipboard,
  type JsonPath,
} from '@tokotachi-designer/core';
import { slotAfter, slotForClick } from './dragController.ts';
import { selectedNode, sendEdit, useDesigner } from './store.ts';
import { removeSelected } from './treeOps.ts';

/** この画面で最後にコピーしたもの(Clipboard API が使えないときの代わり)。 */
let lastCopied: string | undefined;

/** 選んでいるノードの位置(コピーできるものだけ)。 */
function copyTarget(): JsonPath | undefined {
  const { document } = useDesigner.getState();
  const node = selectedNode(useDesigner.getState());
  return document && node && canCopy(document, node.path) ? node.path : undefined;
}

export function canCopySelection(): boolean {
  return copyTarget() !== undefined;
}

/** 選んでいるノードをコピーした文字(コピーできなければ undefined)。 */
function copyText(): string | undefined {
  const { document } = useDesigner.getState();
  const path = copyTarget();
  const data = document && path ? copyNodes(document, [path]) : undefined;
  if (!data) {
    return undefined;
  }
  lastCopied = serializeClipboard(data);
  return lastCopied;
}

/**
 * コピーした文字を貼り付ける。部品は選んでいる場所(パレットのクリックと同じ)か slot に入れ、貼り付けたものを選ぶ。
 * @returns デザイナーがコピーしたものでなければ false
 */
function pasteText(text: string, after?: JsonPath): boolean {
  const data = parseClipboard(text);
  const { document, outline, selection } = useDesigner.getState();
  if (!data || !document) {
    return false;
  }
  const items = preparePaste(document, data.items);
  const hasWidgets = items.some((i) => i.kind === 'widget');
  const slot = !hasWidgets
    ? undefined
    : ((after ? slotAfter(document, after) : undefined) ??
      slotForClick(document, outline, selection));
  sendEdit({ kind: 'paste', items, slot }, items[0]?.node.name);
  return true;
}

/** 入力欄の中では、文字のコピー・貼り付けにする。 */
function inTextInput(e: Event): boolean {
  const t = e.target as HTMLElement | null;
  return t !== null && ['INPUT', 'TEXTAREA', 'SELECT'].includes(t.tagName);
}

/** Ctrl+C・X・V(VS Code が copy・cut・paste のイベントにする)を受ける。 */
export function installClipboard(): () => void {
  const onCopy = (e: ClipboardEvent): void => {
    if (inTextInput(e)) {
      return;
    }
    const text = copyText();
    if (text !== undefined) {
      e.clipboardData?.setData('text/plain', text);
      e.preventDefault();
      if (e.type === 'cut') {
        removeSelected();
      }
    }
  };
  const onPaste = (e: ClipboardEvent): void => {
    if (inTextInput(e)) {
      return;
    }
    if (pasteText(e.clipboardData?.getData('text/plain') ?? '')) {
      e.preventDefault();
    }
  };
  window.addEventListener('copy', onCopy);
  window.addEventListener('cut', onCopy);
  window.addEventListener('paste', onPaste);
  return () => {
    window.removeEventListener('copy', onCopy);
    window.removeEventListener('cut', onCopy);
    window.removeEventListener('paste', onPaste);
  };
}

/** 右クリックのメニューからコピーする(cut なら消す)。 */
export function copySelection(cut: boolean): void {
  const text = copyText();
  if (text === undefined) {
    return;
  }
  void navigator.clipboard.writeText(text).catch(() => undefined);
  if (cut) {
    removeSelected();
  }
}

/** 右クリックのメニューから貼り付ける。 */
export function pasteFromClipboard(): void {
  void navigator.clipboard
    .readText()
    .catch(() => lastCopied ?? '')
    .then((text) => {
      if (!pasteText(text) && lastCopied !== undefined) {
        pasteText(lastCopied);
      }
    });
}

/** 複製(Ctrl+D): コピーして、すぐ後ろに貼り付ける。クリップボードは変えない。 */
export function duplicateSelection(): void {
  const { document } = useDesigner.getState();
  const path = copyTarget();
  const data = document && path ? copyNodes(document, [path]) : undefined;
  if (data && path) {
    pasteText(serializeClipboard(data), path);
  }
}
