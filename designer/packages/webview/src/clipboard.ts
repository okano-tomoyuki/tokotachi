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
  slotAfter,
  type JsonPath,
} from '@tokotachi-designer/core';
import { slotForClick } from './dragController.ts';
import { sendEdit, useDesigner } from './store.ts';
import { removeSelected, selectedPaths } from './treeOps.ts';

/** この画面で最後にコピーしたもの(Clipboard API が使えないときの代わり)。 */
let lastCopied: string | undefined;

/** 選んでいるノードのうちコピーできるものの位置(複数選択ならすべて。中に入っているものは親ごと)。 */
function copyTargets(): JsonPath[] {
  const { document } = useDesigner.getState();
  return document ? selectedPaths((n) => canCopy(document, n.path)) : [];
}

export function canCopySelection(): boolean {
  return copyTargets().length > 0;
}

/** 選んでいるノードをコピーした文字(コピーできなければ undefined)。 */
function copyText(): string | undefined {
  const { document } = useDesigner.getState();
  const data = document ? copyNodes(document, copyTargets()) : undefined;
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
  // 貼り付けたものすべてを選ぶ
  sendEdit(
    { kind: 'paste', items, slot },
    items.map((i) => i.node.name),
  );
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

/** 複製(Ctrl+D): コピーして、すぐ後ろ(複数なら最後のものの後ろ)に貼り付ける。クリップボードは変えない。 */
export function duplicateSelection(): void {
  const { document } = useDesigner.getState();
  const paths = copyTargets();
  const data = document ? copyNodes(document, paths) : undefined;
  if (data) {
    pasteText(serializeClipboard(data), paths[paths.length - 1]);
  }
}
