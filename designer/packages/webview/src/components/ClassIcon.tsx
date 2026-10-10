/**
 * クラスと木のノードのアイコン(docs/designer/editor-design.md §4.1・§5)。パレットと構造の木で使う。
 * 16×16 の線画の SVG(Bethany のデザイナーのものを土台に、Tokotachi の部品に合わせて描いた)。
 * 線は currentColor で VS Code のテーマに合わせ、要所だけ強調色(--tt-icon-accent)で描く。線が画素にそろうよう、線の座標は .5 にする。
 *
 * クラス名 a: 強調色の線、af: 強調色の塗り、sf: 薄い塗り
 */
import type { OutlineNode } from '@tokotachi-designer/core';
import type { ReactNode } from 'react';

/** カタログのクラスの名前、またはクラスを持たない木のノードの種類(spacer は stretch・spacing に分ける)。 */
const ICONS: Readonly<Record<string, ReactNode>> = {
  // ---- ウィンドウ ----
  Window: (
    <>
      <rect x="1.5" y="2.5" width="13" height="11" rx="1" />
      <rect className="af" x="2" y="3" width="12" height="2.5" />
      <path d="M1.5 5.5h13" />
    </>
  ),
  Dialog: (
    <>
      <rect x="1.5" y="2.5" width="13" height="11" rx="1" />
      <rect className="af" x="2" y="3" width="12" height="2.5" />
      <path d="M1.5 5.5h13" />
      <rect x="6.5" y="9.5" width="3" height="2" />
      <rect className="sf" x="10.5" y="9.5" width="3" height="2" />
      <rect className="a" x="10.5" y="9.5" width="3" height="2" />
    </>
  ),

  // ---- 入力 ----
  Button: (
    <>
      <rect x="1.5" y="4.5" width="13" height="7" rx="2" />
      <path className="a" d="M5 8h6" />
    </>
  ),
  CheckBox: (
    <>
      <rect x="1.5" y="4.5" width="7" height="7" rx="1" />
      <path className="a" d="M3.2 8l1.6 1.6 2.5-3.3" />
      <path d="M10.5 8h4" />
    </>
  ),
  RadioButton: (
    <>
      <circle cx="5" cy="8" r="3.5" />
      <circle className="af" cx="5" cy="8" r="1.6" />
      <path d="M10.5 8h4" />
    </>
  ),
  LineEdit: (
    <>
      <rect x="1.5" y="4.5" width="13" height="7" />
      <path className="a" d="M4.5 6.5v3" />
    </>
  ),
  TextEdit: (
    <>
      <rect x="1.5" y="1.5" width="13" height="13" />
      <path d="M11.5 1.5v13M3.5 4.5h6M3.5 7.5h6M3.5 10.5h4" />
      <path className="a" d="M13 3.5v3" />
    </>
  ),
  SpinBox: (
    <>
      <rect x="1.5" y="4.5" width="13" height="7" />
      <path d="M10.5 4.5v7M10.5 8h4M3.5 8h4.5" />
      <path className="a" d="M11.7 6.9l.8-.9.8.9M11.7 9.1l.8.9.8-.9" />
    </>
  ),
  Slider: (
    <>
      <path d="M1.5 8.5h13" />
      <path className="a" d="M1.5 8.5h6" />
      <circle className="af" cx="8.5" cy="8.5" r="2.5" />
    </>
  ),
  ComboBox: (
    <>
      <rect x="1.5" y="4.5" width="13" height="7" />
      <path d="M10.5 4.5v7M3.5 8h4.5" />
      <path className="a" d="M11.6 7.3l.9 1.2.9-1.2" />
    </>
  ),

  // ---- 表示 ----
  Label: <path d="M3.5 13.5L8 2.5l4.5 11M5.3 9.5h5.4" />,
  ProgressBar: (
    <>
      <rect x="1.5" y="5.5" width="13" height="5" />
      <rect className="af" x="2.5" y="6.5" width="7" height="3" />
    </>
  ),

  // ---- 一覧 ----
  ListView: (
    <>
      <rect x="1.5" y="1.5" width="13" height="13" />
      <path d="M1.5 4.5h13M7.5 1.5v3" />
      <rect className="sf" x="2" y="7" width="12" height="3" />
      <path className="a" d="M3.5 8.5h3M9 8.5h3.5" />
      <path d="M3.5 12h3M9 12h3.5" />
    </>
  ),
  TreeView: (
    <>
      <rect className="af" x="2" y="1.5" width="4" height="3" />
      <path d="M4 4.5v8M4 8h3.5M4 12.5h3.5" />
      <rect x="7.5" y="6.5" width="6" height="3" />
      <rect x="7.5" y="11" width="6" height="3" />
    </>
  ),
  Column: (
    <>
      <path d="M1.5 2.5h13M1.5 5.5h13M1.5 2.5v12M14.5 2.5v12" />
      <rect className="sf" x="5.5" y="2.5" width="5" height="12" />
      <path className="a" d="M5.5 2.5v12M10.5 2.5v12" />
    </>
  ),

  // ---- 入れ物 ----
  Widget: (
    <>
      <rect className="sf" x="1.5" y="3.5" width="13" height="9" />
      <rect x="1.5" y="3.5" width="13" height="9" />
      <path className="a" d="M3.5 11V5.5h9" />
    </>
  ),
  TabWidget: (
    <>
      <rect x="1.5" y="5.5" width="13" height="9" />
      <path className="a" d="M1.5 5.5v-3h5v3" />
      <path d="M6.5 3.5h4.5v2M3.5 8.5h7M3.5 11.5h5" />
    </>
  ),
  Tab: (
    <>
      <rect x="1.5" y="4.5" width="13" height="10" />
      <path className="a" d="M2.5 4.5v-2h5v2" />
      <path d="M3.5 7.5h7M3.5 10.5h5" />
    </>
  ),
  Splitter: (
    <>
      <rect x="1.5" y="2.5" width="13" height="11" />
      <rect className="af" x="7" y="3" width="2" height="10" />
      <path d="M5.5 8h-2M4.5 7l-1 1 1 1M10.5 8h2M11.5 7l1 1-1 1" />
    </>
  ),
  SplitterPane: (
    <>
      <rect x="1.5" y="2.5" width="13" height="11" />
      <path d="M8.5 2.5v11" />
      <rect className="sf" x="2" y="3" width="6" height="10" />
      <path className="a" d="M3.5 5.5h3M3.5 8h3M3.5 10.5h2" />
    </>
  ),
  ScrollArea: (
    <>
      <rect x="1.5" y="1.5" width="13" height="13" />
      <path d="M11.5 1.5v10M1.5 11.5h13" />
      <path className="a" d="M13 3.5v4M3.5 13h4" />
    </>
  ),

  // ---- 描画 ----
  PaintBox: (
    <>
      <rect x="1.5" y="1.5" width="13" height="13" strokeDasharray="2 1.5" />
      <path className="a" d="M4 11.5c1.5-5 3.5 1 5-3.5s1.5-2 3-3" />
    </>
  ),

  // ---- レイアウト・空き ----
  BoxLayout: (
    <>
      <rect x="1.5" y="3.5" width="13" height="9" strokeDasharray="2 1.5" />
      <rect className="af" x="3" y="5" width="3" height="6" />
      <rect x="6.5" y="5.5" width="3" height="5" />
      <rect x="10.5" y="5.5" width="2.5" height="5" />
    </>
  ),
  GridLayout: (
    <>
      <rect x="1.5" y="1.5" width="13" height="13" strokeDasharray="2 1.5" />
      <rect className="af" x="3" y="3" width="4.5" height="4.5" />
      <rect x="9" y="3.5" width="4" height="4" />
      <rect x="3.5" y="9" width="4" height="4" />
      <rect x="9" y="9" width="4" height="4" />
    </>
  ),
  PlaceLayout: (
    <>
      <rect x="1.5" y="1.5" width="13" height="13" strokeDasharray="2 1.5" />
      <rect className="af" x="3" y="3" width="5" height="3.5" />
      <rect x="6.5" y="8.5" width="6" height="4" />
      <path className="a" d="M3.5 13V9M3.5 13h2.5" />
    </>
  ),
  stretch: (
    <>
      <path d="M1.5 4.5v7M14.5 4.5v7" />
      <path className="a" d="M3 8l1.5-2 2 4 2-4 2 4 2-4L13 8" />
    </>
  ),
  spacing: (
    <>
      <path d="M1.5 4.5v7M14.5 4.5v7" />
      <path className="a" d="M3 8h10M5 6.5L3.5 8 5 9.5M11 6.5l1.5 1.5-1.5 1.5" />
    </>
  ),

  // ---- メニュー・バー・Action ----
  MenuBar: (
    <>
      <rect x="1.5" y="4.5" width="13" height="7" />
      <path className="a" d="M3.5 8h2.5M8 8h2.5" />
    </>
  ),
  Menu: (
    <>
      <path d="M1.5 3.5h7" />
      <rect className="sf" x="3.5" y="5.5" width="10" height="8.5" />
      <rect x="3.5" y="5.5" width="10" height="8.5" />
      <path className="a" d="M5.5 8h6" />
      <path d="M5.5 11h6" />
    </>
  ),
  menu_item: (
    <>
      <rect className="sf" x="1.5" y="5.5" width="13" height="5" />
      <rect x="1.5" y="5.5" width="13" height="5" />
      <path d="M3.5 8h6" />
      <path className="a" d="M11.8 6.8l1.2 1.2-1.2 1.2" />
    </>
  ),
  separator: <path className="a" d="M1.5 8h13" />,
  ToolBar: (
    <>
      <rect x="1.5" y="4.5" width="13" height="7" />
      <rect className="af" x="3" y="6" width="3" height="4" />
      <rect x="7.5" y="6.5" width="2.5" height="3" />
      <path d="M12 6v4" />
    </>
  ),
  tool_item: (
    <>
      <rect x="3.5" y="3.5" width="9" height="9" rx="1.5" />
      <rect className="af" x="6" y="6" width="4" height="4" />
    </>
  ),
  StatusBar: (
    <>
      <rect x="1.5" y="2.5" width="13" height="11" />
      <rect className="af" x="2" y="10.5" width="12" height="2.5" />
      <path d="M1.5 10.5h13" />
    </>
  ),
  Action: <path className="af" d="M9.5 1.5L3.5 9h4l-1 5.5 6-7.5h-4z" />,
  actions: (
    <>
      <rect x="1.5" y="2.5" width="10" height="11" rx="1" />
      <path d="M3.5 5.5h4M3.5 8h4M3.5 10.5h4" />
      <path className="af" d="M13 6.5l-3 4h2l-.5 3 3-4h-2z" />
    </>
  ),

  // ---- オブジェクト・Variable ----
  Timer: (
    <>
      <circle cx="8" cy="9" r="5.5" />
      <path d="M6.5 1.5h3M8 1.5v2" />
      <path className="a" d="M8 9V6M8 9h2.5" />
    </>
  ),
  objects: (
    <>
      <path d="M8 1.5l6 3v7l-6 3-6-3v-7z" />
      <path d="M2 4.5l6 3 6-3M8 7.5v7" />
      <path className="af" d="M8 1.5l6 3-6 3-6-3z" />
    </>
  ),
  Variable: (
    <>
      <rect x="1.5" y="3.5" width="13" height="9" rx="2" />
      <path className="a" d="M4.5 6l3 4M7.5 6l-3 4" />
      <path d="M9.5 7h3M9.5 9h3" />
    </>
  ),
  variables: (
    <>
      <rect x="3.5" y="1.5" width="11" height="7" rx="1.5" />
      <rect x="1.5" y="6.5" width="11" height="8" rx="1.5" />
      <path className="a" d="M3.5 8.5l3 4M6.5 8.5l-3 4" />
      <path d="M8 10h3M8 12h3" />
    </>
  ),
};

/** 描けないもの(カタログに後から加えたクラス等)は、点線の四角 */
const FALLBACK = <rect x="2.5" y="2.5" width="11" height="11" rx="1" strokeDasharray="2 1.5" />;

export function hasClassIcon(key: string): boolean {
  return Object.hasOwn(ICONS, key);
}

/** 木のノードのアイコンの名前。 */
export function nodeIconKey(node: OutlineNode): string {
  if (node.class !== undefined) {
    return node.class;
  }
  switch (node.kind) {
    case 'group':
      return node.label; // actions・objects・variables
    case 'spacer':
      // 固定の空きは "spacing 12"、伸びる空きは "stretch 1"(outline.ts)
      return node.label.startsWith('spacing') ? 'spacing' : 'stretch';
    default:
      return node.kind;
  }
}

export function ClassIcon({ name, size = 16 }: { readonly name: string; readonly size?: number }) {
  return (
    <svg
      className="class-icon"
      width={size}
      height={size}
      viewBox="0 0 16 16"
      fill="none"
      stroke="currentColor"
      strokeWidth="1"
      strokeLinecap="round"
      strokeLinejoin="round"
      aria-hidden="true"
      focusable="false"
    >
      {hasClassIcon(name) ? ICONS[name] : FALLBACK}
    </svg>
  );
}
