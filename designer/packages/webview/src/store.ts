/**
 * デザイナーの画面の状態(Zustand)。ドキュメントは拡張から届いたテキストを読んだもので、編集はコマンドを拡張に送り、
 * TextDocument が変わったら届く新しいテキストで置き換わる(docs/designer/editor-design.md §3)。
 */
import {
  findOutlineNode,
  flattenOutline,
  hiddenOutlineNodes,
  outline,
  parseDocument,
  type CanvasFrame,
  type Diagnostic,
  type EditCommand,
  type ExtensionToWebviewMessage,
  type OutlineHidden,
  type OutlineNode,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import { create } from 'zustand';
import type { Drag, DropTarget } from './dropTarget.ts';
import { postMessage } from './vscode.ts';

/** ドラッグ中の像。dx・dy はつかんだ点(像の左上から)。 */
export interface Ghost {
  /** 部品の画像(data URL)。無ければ label を出す。 */
  readonly image?: string;
  readonly label?: string;
  readonly width: number;
  readonly height: number;
  readonly dx: number;
  readonly dy: number;
}

/** 右クリックのメニューの項目。 */
export interface MenuItem {
  readonly label: string;
  /** 説明(ツールチップ)。 */
  readonly title?: string;
  /** 右に添えるキーの操作。 */
  readonly shortcut?: string;
  readonly disabled?: boolean;
  /** 前に区切りの線を引く。 */
  readonly separator?: boolean;
  readonly run: () => void;
}

/** 左のペイン(パレットと構造の木)の配置。Webview の状態に保存し、開き直したときに戻す。 */
export interface SidebarLayout {
  /** 幅(px)。 */
  readonly width: number;
  /** パレットの高さ(ペインの高さに対する割合)。 */
  readonly paletteRatio: number;
  /** 構造の木の下の Action・オブジェクト・Variable の欄の高さ(構造の欄の高さに対する割合)。 */
  readonly groupsRatio: number;
  /** ペインを隠しているか(キャンバスを広く使う)。 */
  readonly hidden: boolean;
  /** パレットで開いているタブ(画面に出る部品か、非ビジュアルか)。 */
  readonly paletteTab: 'visual' | 'non_visual';
  /** 畳んでいるパレットの分類。 */
  readonly closedCategories: readonly string[];
}

export const DEFAULT_SIDEBAR: SidebarLayout = {
  width: 220,
  paletteRatio: 0.45,
  groupsRatio: 0.35,
  hidden: false,
  paletteTab: 'visual',
  closedCategories: [],
};

export interface DesignerState {
  readonly version: number;
  readonly fileName: string;
  /** 構造の検証を通ったドキュメント(通らなければ undefined)。 */
  readonly document: TtuiDocument | undefined;
  readonly diagnostics: readonly Diagnostic[];
  readonly outline: readonly OutlineNode[];
  /** 非表示のノード(visible を false と書いた部品と、その子孫)。 */
  readonly hidden: ReadonlyMap<string, OutlineHidden>;
  readonly frame: CanvasFrame | undefined;
  readonly canvasError: string | undefined;
  /** 選んでいる木のノードの id。 */
  readonly selection: string | undefined;
  /** 最後の編集の失敗(しばらく表示する)。 */
  readonly editError: string | undefined;
  /** ドラッグしているもの(パレットの部品・空き、移す部品)と、入れる場所。 */
  readonly drag: Drag | undefined;
  readonly target: DropTarget | undefined;
  /** ドラッグ中にカーソルについて動かす像(移す部品の画像か、パレットの部品の名前)と、カーソルの位置(画面の座標)。 */
  readonly ghost: Ghost | undefined;
  readonly pointer: { readonly x: number; readonly y: number } | undefined;
  /** キャンバスの倍率(1 = 100%)。 */
  readonly zoom: number;
  /** プロジェクトに登録したテーマの名前(theme_override の preset で選べるもの)。 */
  readonly themePresets: readonly string[];
  readonly sidebar: SidebarLayout;
  /** 開いている右クリックのメニュー(画面の座標と項目)。 */
  readonly contextMenu:
    { readonly x: number; readonly y: number; readonly items: readonly MenuItem[] } | undefined;
  readonly select: (id: string | undefined) => void;
  readonly setSidebar: (change: Partial<SidebarLayout>) => void;
}

export const useDesigner = create<DesignerState>()((set) => ({
  version: -1,
  fileName: '',
  document: undefined,
  diagnostics: [],
  outline: [],
  hidden: new Map(),
  frame: undefined,
  canvasError: undefined,
  selection: 'window',
  editError: undefined,
  drag: undefined,
  target: undefined,
  ghost: undefined,
  pointer: undefined,
  zoom: 1,
  themePresets: [],
  sidebar: DEFAULT_SIDEBAR,
  contextMenu: undefined,
  select: (id) => {
    set({ selection: id });
  },
  setSidebar: (change) => {
    set((s) => ({ sidebar: { ...s.sidebar, ...change } }));
  },
}));

/** 右クリックのメニューを開く(項目が無ければ開かない)。 */
export function openContextMenu(x: number, y: number, items: readonly MenuItem[]): void {
  useDesigner.setState({ contextMenu: items.length > 0 ? { x, y, items } : undefined });
}

export function closeContextMenu(): void {
  useDesigner.setState({ contextMenu: undefined });
}

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
                (n) =>
                  ['widget', 'object', 'menu_item', 'tool_item', 'action', 'variable'].includes(
                    n.kind,
                  ) && n.label === pendingSelection,
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
          hidden: document ? hiddenOutlineNodes(document, tree) : new Map<string, OutlineHidden>(),
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
    case 'themePresets':
      useDesigner.setState({ themePresets: message.names });
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
