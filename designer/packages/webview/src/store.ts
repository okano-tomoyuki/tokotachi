/**
 * デザイナーの画面の状態(Zustand)。ドキュメントは拡張から届いたテキストを読んだもので、編集はコマンドを拡張に送り、
 * TextDocument が変わったら届く新しいテキストで置き換わる(docs/designer/editor-design.md §3)。
 */
import {
  findOutlineNode,
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
  select: (id) => {
    set({ selection: id });
  },
}));

let nextRequestId = 1;

export function sendEdit(command: EditCommand): void {
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
        // 選んでいたノードが無くなったら、ウィンドウを選ぶ
        const selection =
          s.selection !== undefined && findOutlineNode(tree, s.selection) ? s.selection : 'window';
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
