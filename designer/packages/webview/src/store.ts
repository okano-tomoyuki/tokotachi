/**
 * デザイナーの画面の状態(Zustand)。ドキュメントは拡張から届いたテキストを読んだもので、編集はコマンドを拡張に送り、
 * TextDocument が変わったら届く新しいテキストで置き換わる(docs/designer/editor-design.md §3)。
 */
import {
  findOutlineNode,
  flattenOutline,
  outline,
  parseDocument,
  type CanvasFrame,
  type Diagnostic,
  type EditCommand,
  type ExtensionToWebviewMessage,
  type OutlineNode,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import { create } from 'zustand';
import type { Drag, DropTarget } from './dropTarget.ts';
import { postMessage } from './vscode.ts';

export interface DesignerState {
  readonly version: number;
  readonly fileName: string;
  /** 構造の検証を通ったドキュメント(通らなければ undefined)。 */
  readonly document: TtuiDocument | undefined;
  readonly diagnostics: readonly Diagnostic[];
  readonly outline: readonly OutlineNode[];
  readonly frame: CanvasFrame | undefined;
  readonly canvasError: string | undefined;
  /** 選んでいる木のノードの id。 */
  readonly selection: string | undefined;
  /** 最後の編集の失敗(しばらく表示する)。 */
  readonly editError: string | undefined;
  /** ドラッグしているもの(パレットの部品・空き、移す部品)と、入れる場所。 */
  readonly drag: Drag | undefined;
  readonly target: DropTarget | undefined;
  readonly select: (id: string | undefined) => void;
}

export const useDesigner = create<DesignerState>()((set) => ({
  version: -1,
  fileName: '',
  document: undefined,
  diagnostics: [],
  outline: [],
  frame: undefined,
  canvasError: undefined,
  selection: 'window',
  editError: undefined,
  drag: undefined,
  target: undefined,
  select: (id) => {
    set({ selection: id });
  },
}));

let nextRequestId = 1;
/** 足した・移した部品の名前。ドキュメントが届いたら、その部品を選ぶ。 */
let pendingSelection: string | undefined;

export function sendEdit(command: EditCommand, select?: string): void {
  pendingSelection = select;
  postMessage({ type: 'edit', requestId: nextRequestId++, command });
}

/** 選んでいるノード(無くなっていれば undefined)。 */
export function selectedNode(state: DesignerState): OutlineNode | undefined {
  return state.selection === undefined
    ? undefined
    : findOutlineNode(state.outline, state.selection);
}

let errorTimer: ReturnType<typeof setTimeout> | undefined;

export function handleMessage(message: ExtensionToWebviewMessage): void {
  switch (message.type) {
    case 'document': {
      const { document, diagnostics } = parseDocument(message.text);
      useDesigner.setState((s) => {
        const tree = document ? outline(document) : [];
        // 足した・移した部品を選ぶ。選んでいたノードが無くなったら、ウィンドウを選ぶ
        const added =
          pendingSelection === undefined
            ? undefined
            : flattenOutline(tree).find(
                (n) => (n.kind === 'widget' || n.kind === 'object') && n.label === pendingSelection,
              );
        if (added) {
          pendingSelection = undefined;
        }
        const selection =
          added?.id ??
          (s.selection !== undefined && findOutlineNode(tree, s.selection)
            ? s.selection
            : 'window');
        return {
          version: message.version,
          fileName: message.fileName,
          document,
          diagnostics,
          outline: tree,
          selection,
        };
      });
      break;
    }
    case 'canvas':
      useDesigner.setState({ frame: message.frame, canvasError: undefined });
      break;
    case 'canvasError':
      useDesigner.setState({ canvasError: message.message });
      break;
    case 'editResult':
      if (!message.ok) {
        useDesigner.setState({ editError: message.error });
        if (errorTimer !== undefined) {
          clearTimeout(errorTimer);
        }
        errorTimer = setTimeout(() => {
          useDesigner.setState({ editError: undefined });
        }, 6000);
      }
      break;
  }
}
