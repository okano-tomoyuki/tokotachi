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
  setCustomClasses,
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
  /** 選んでいる木のノードの id(インスペクタに出す、主に選んでいるもの)。 */
  readonly selection: string | undefined;
  /** 選んでいるノードすべての id(複数選択。selection を含む)。 */
  readonly selected: readonly string[];
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
  /** パレットに出す部品ファイル(パスは定義ファイルのフォルダからの相対パス)。 */
  readonly components: readonly { readonly path: string; readonly className: string }[];
  /** 自作部品が変わるたびに増える(パレットを作り直す)。 */
  readonly customVersion: number;
  readonly sidebar: SidebarLayout;
  /** 開いている右クリックのメニュー(画面の座標と項目)。 */
  readonly contextMenu:
    { readonly x: number; readonly y: number; readonly items: readonly MenuItem[] } | undefined;
  readonly select: (id: string | undefined) => void;
  /** 選択に足す・外す(Ctrl+クリック)。 */
  readonly toggleSelect: (id: string) => void;
  /** 主に選んでいるものから id までを選ぶ(構造の木の Shift+クリック。ids は木の見えている行の順)。 */
  readonly selectRange: (id: string, ids: readonly string[]) => void;
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
  selected: ['window'],
  editError: undefined,
  drag: undefined,
  target: undefined,
  ghost: undefined,
  pointer: undefined,
  zoom: 1,
  themePresets: [],
  components: [],
  customVersion: 0,
  sidebar: DEFAULT_SIDEBAR,
  contextMenu: undefined,
  select: (id) => {
    set({ selection: id, selected: id === undefined ? [] : [id] });
  },
  toggleSelect: (id) => {
    set((s) => {
      if (s.selected.includes(id)) {
        const rest = s.selected.filter((x) => x !== id);
        // 主に選んでいたものを外したら、残りの最後を主にする(何も残らなければウィンドウ)
        const selection = s.selection === id ? (rest[rest.length - 1] ?? 'window') : s.selection;
        return { selection, selected: rest.length > 0 ? rest : ['window'] };
      }
      // ウィンドウは部品と一緒に選ばない
      const others = s.selected.filter((x) => x !== 'window');
      return { selection: id, selected: id === 'window' ? [id] : [...others, id] };
    });
  },
  selectRange: (id, ids) => {
    set((s) => {
      const from = s.selection === undefined ? -1 : ids.indexOf(s.selection);
      const to = ids.indexOf(id);
      if (from < 0 || to < 0) {
        return { selection: id, selected: [id] };
      }
      const range = ids.slice(Math.min(from, to), Math.max(from, to) + 1);
      const selected = range.filter((x) => x !== 'window' || range.length === 1);
      // 主に選んでいるもの(範囲の起点)はそのままにする(ウィンドウを外したときは id を主にする)
      return {
        selection: s.selection !== undefined && selected.includes(s.selection) ? s.selection : id,
        selected,
      };
    });
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
/** 足した・移した部品の名前。ドキュメントが届いたら、その部品を選ぶ(複数なら最初を主にする)。 */
let pendingSelection: readonly string[] | undefined;

/** 並べ替えた後に選ぶノードの id(名前の無いノードも選び直せるように、位置で渡す)。最初を主にする。 */
let pendingIds: readonly string[] | undefined;

export function sendEdit(command: EditCommand, select?: string | readonly string[]): void {
  pendingSelection =
    select === undefined ? undefined : typeof select === 'string' ? [select] : select;
  pendingIds = undefined;
  postMessage({ type: 'edit', requestId: nextRequestId++, command });
}

/** 編集して、届いたドキュメントで ids のノードを選ぶ(ids の最初を主にする)。 */
export function sendEditSelectingIds(command: EditCommand, ids: readonly string[]): void {
  sendEdit(command);
  pendingIds = ids;
}

/** 選んでいるノードすべて(構造の木の順)。 */
export function selectedNodes(state: DesignerState): OutlineNode[] {
  const ids = new Set(state.selected);
  return flattenOutline(state.outline).filter((n) => ids.has(n.id));
}

/** 選んでいるノード(無くなっていれば undefined)。 */
export function selectedNode(state: DesignerState): OutlineNode | undefined {
  return state.selection === undefined
    ? undefined
    : findOutlineNode(state.outline, state.selection);
}

let errorTimer: ReturnType<typeof setTimeout> | undefined;

/** 最後に受け取った内容(自作部品が変わったら、検証し直す)。 */
let lastDocument: Extract<ExtensionToWebviewMessage, { type: 'document' }> | undefined;

export function handleMessage(message: ExtensionToWebviewMessage): void {
  switch (message.type) {
    case 'customClasses':
      // 自作部品(docs/adr/0044)をカタログに足し、今の内容を検証し直す
      setCustomClasses(message.classes);
      useDesigner.setState((s) => ({ customVersion: s.customVersion + 1 }));
      if (lastDocument) {
        handleMessage(lastDocument);
      }
      break;
    case 'document': {
      lastDocument = message;
      const { document, diagnostics } = parseDocument(message.text);
      useDesigner.setState((s) => {
        const tree = document ? outline(document) : [];
        // 足した・移した部品を選ぶ。選んでいたノードが無くなったら、ウィンドウを選ぶ
        const pending = pendingSelection;
        const added =
          pending === undefined
            ? []
            : flattenOutline(tree).filter(
                (n) =>
                  ['widget', 'object', 'menu_item', 'tool_item', 'action', 'variable'].includes(
                    n.kind,
                  ) && pending.includes(n.label),
              );
        if (added.length > 0) {
          pendingSelection = undefined;
        }
        // 主に選ぶのは、名前を渡した順の最初
        const first = pending
          ?.map((name) => added.find((n) => n.label === name))
          .find((n) => n !== undefined);
        const selection =
          first?.id ??
          (s.selection !== undefined && findOutlineNode(tree, s.selection)
            ? s.selection
            : 'window');
        // 並べ替えたものを、新しい位置で選び直す
        const ids = pendingIds?.filter((id) => findOutlineNode(tree, id) !== undefined) ?? [];
        pendingIds = undefined;
        if (ids.length > 0 && ids[0] !== undefined) {
          return {
            version: message.version,
            fileName: message.fileName,
            document,
            diagnostics,
            outline: tree,
            selected: ids,
            hidden: document
              ? hiddenOutlineNodes(document, tree)
              : new Map<string, OutlineHidden>(),
            selection: ids[0],
          };
        }
        // 足した・移したものを選んだら、ほかの選択は解く。無くなったノードは選択から外す
        const kept = s.selected.filter((id) => findOutlineNode(tree, id) !== undefined);
        const selected =
          added.length > 0
            ? added.map((n) => n.id)
            : kept.includes(selection)
              ? kept
              : [...kept.filter((id) => id !== 'window'), selection];
        return {
          version: message.version,
          fileName: message.fileName,
          document,
          diagnostics,
          outline: tree,
          selected,
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
    case 'components':
      useDesigner.setState({ components: message.items });
      break;
    case 'editResult':
      if (!message.ok) {
        pendingIds = undefined;
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
