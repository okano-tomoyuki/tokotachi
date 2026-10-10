/**
 * 画面の定義ファイル(*.ttui.json)の構造のスキーマ(docs/designer/dsl-spec.md)。
 * 実装後はこのスキーマを正とし、dsl-spec.md と食い違う場合は dsl-spec.md を直す。
 *
 * ここでは形だけを検証する。クラス・プロパティ・ハンドラがカタログにあるか、値が型に合うか、名前の重複・参照・
 * レイアウトの項目の組み合わせは validate.ts(意味の検証)で調べる。
 */
import * as z from 'zod';

export const FORMAT_VERSION = 1;
export const DOCUMENT_EXTENSION = '.ttui.json';

/**
 * プロパティの値(型に合うかは意味の検証で調べる)。
 * `{ "bind": "Variable の名前" }` は、その Variable との結びつけ(dsl-spec.md §5.2)。
 */
export const PropertyValue = z.json();
export type PropertyValue = z.infer<typeof PropertyValue>;

/** プロパティの名前 → 値。 */
export const Properties = z.record(z.string(), PropertyValue);
export type Properties = z.infer<typeof Properties>;

/** ハンドラのプロパティの名前 → メンバ関数の名前。 */
export const Events = z.record(z.string(), z.string());
export type Events = z.infer<typeof Events>;

// ---- 部品とレイアウト(互いに入れ子になる) ----

export interface WidgetNode {
  readonly name: string;
  readonly class: string;
  readonly properties?: Properties | undefined;
  readonly events?: Events | undefined;
  /** add_action する Action の名前。 */
  readonly actions?: readonly string[] | undefined;
  readonly layout?: LayoutNode | undefined;
  /** レイアウトを付けない自由な配置の子。 */
  readonly children?: readonly WidgetNode[] | undefined;
  /** TabWidget のタブ。 */
  readonly tabs?: readonly TabNode[] | undefined;
  /** Splitter の区画。 */
  readonly panes?: readonly PaneNode[] | undefined;
  /** ScrollArea の中身。 */
  readonly content?: WidgetNode | undefined;
  /** ListView・TreeView の列。 */
  readonly columns?: readonly ColumnNode[] | undefined;
}

/** レイアウトの項目。どのキーを書けるかはレイアウトのクラスで決まる(意味の検証で調べる)。 */
export interface LayoutItem {
  readonly widget?: WidgetNode | undefined;
  // BoxLayout
  readonly stretch?: number | undefined;
  readonly alignment?: string | undefined;
  readonly spacing?: number | undefined;
  // GridLayout
  readonly row?: number | undefined;
  readonly column?: number | undefined;
  readonly row_span?: number | undefined;
  readonly column_span?: number | undefined;
  readonly horizontal?: string | undefined;
  readonly vertical?: string | undefined;
  // PlaceLayout
  readonly placement?: Properties | undefined;
}

/** GridLayout の行・列の設定。 */
export const TrackSettings = z.strictObject({
  stretch: z.int().min(0).optional(),
  minimum: z.number().min(0).optional(),
});
export type TrackSettings = z.infer<typeof TrackSettings>;

export interface LayoutNode {
  readonly name?: string | undefined;
  readonly class: string;
  readonly properties?: Properties | undefined;
  readonly items?: readonly LayoutItem[] | undefined;
  /** GridLayout の行・列の設定(番号 → 設定)。 */
  readonly rows?: Readonly<Record<string, TrackSettings>> | undefined;
  readonly columns?: Readonly<Record<string, TrackSettings>> | undefined;
}

export interface TabNode {
  readonly name?: string | undefined;
  readonly properties?: Properties | undefined;
  readonly page: WidgetNode;
}

/** 一覧の列(ItemView::add_column)。properties は Column のプロパティ(title・width・alignment)。 */
export interface ColumnNode {
  readonly name?: string | undefined;
  readonly properties?: Properties | undefined;
}

export interface PaneNode {
  readonly name?: string | undefined;
  readonly properties?: Properties | undefined;
  readonly widget: WidgetNode;
}

export const WidgetNode: z.ZodType<WidgetNode> = z.strictObject({
  name: z.string(),
  class: z.string(),
  properties: Properties.optional(),
  events: Events.optional(),
  actions: z.array(z.string()).optional(),
  get layout() {
    return LayoutNode.optional();
  },
  get children() {
    return z.array(WidgetNode).optional();
  },
  get tabs() {
    return z.array(TabNode).optional();
  },
  get panes() {
    return z.array(PaneNode).optional();
  },
  get content() {
    return WidgetNode.optional();
  },
  get columns() {
    return z.array(ColumnNode).optional();
  },
});

export const LayoutItem: z.ZodType<LayoutItem> = z.strictObject({
  get widget() {
    return WidgetNode.optional();
  },
  stretch: z.int().min(0).optional(),
  alignment: z.string().optional(),
  spacing: z.number().min(0).optional(),
  row: z.int().min(0).optional(),
  column: z.int().min(0).optional(),
  row_span: z.int().min(1).optional(),
  column_span: z.int().min(1).optional(),
  horizontal: z.string().optional(),
  vertical: z.string().optional(),
  placement: Properties.optional(),
});

/** 行・列の番号(0 以上の整数の文字列)。 */
const TrackIndex = z.string().regex(/^(0|[1-9][0-9]*)$/);

export const LayoutNode: z.ZodType<LayoutNode> = z.strictObject({
  name: z.string().optional(),
  class: z.string(),
  properties: Properties.optional(),
  get items() {
    return z.array(LayoutItem).optional();
  },
  rows: z.record(TrackIndex, TrackSettings).optional(),
  columns: z.record(TrackIndex, TrackSettings).optional(),
});

export const TabNode: z.ZodType<TabNode> = z.strictObject({
  name: z.string().optional(),
  properties: Properties.optional(),
  get page() {
    return WidgetNode;
  },
});

export const ColumnNode: z.ZodType<ColumnNode> = z.strictObject({
  name: z.string().optional(),
  properties: Properties.optional(),
});

export const PaneNode: z.ZodType<PaneNode> = z.strictObject({
  name: z.string().optional(),
  properties: Properties.optional(),
  get widget() {
    return WidgetNode;
  },
});

// ---- メニュー・ツールバー・ステータスバー ----

export interface MenuNode {
  readonly name?: string | undefined;
  readonly title: string;
  readonly items?: readonly MenuItem[] | undefined;
}

/** メニューの項目。action・separator・menu のどれか 1 つ(意味の検証で調べる)。 */
export interface MenuItem {
  readonly action?: string | undefined;
  readonly separator?: true | undefined;
  readonly menu?: MenuNode | undefined;
}

export const MenuNode: z.ZodType<MenuNode> = z.strictObject({
  name: z.string().optional(),
  title: z.string(),
  get items() {
    return z.array(MenuItem).optional();
  },
});

export const MenuItem: z.ZodType<MenuItem> = z.strictObject({
  action: z.string().optional(),
  separator: z.literal(true).optional(),
  get menu() {
    return MenuNode.optional();
  },
});

export const MenuBarNode = z.strictObject({
  name: z.string().optional(),
  properties: Properties.optional(),
  menus: z.array(MenuNode).optional(),
});
export type MenuBarNode = z.infer<typeof MenuBarNode>;

/** ツールバーの項目。action・separator・widget のどれか 1 つ(意味の検証で調べる)。 */
export const ToolItem = z.strictObject({
  action: z.string().optional(),
  separator: z.literal(true).optional(),
  widget: WidgetNode.optional(),
});
export type ToolItem = z.infer<typeof ToolItem>;

export const ToolBarNode = z.strictObject({
  name: z.string().optional(),
  properties: Properties.optional(),
  items: z.array(ToolItem).optional(),
});
export type ToolBarNode = z.infer<typeof ToolBarNode>;

export const StatusBarNode = z.strictObject({
  name: z.string().optional(),
  properties: Properties.optional(),
  widgets: z.array(WidgetNode).optional(),
});
export type StatusBarNode = z.infer<typeof StatusBarNode>;

// ---- ウィンドウとドキュメント ----

/** Dialog のボタン(Dialog::add_button)。 */
export const DialogButtonNode = z.strictObject({
  button: z.string(),
  text: z.string().optional(),
});
export type DialogButtonNode = z.infer<typeof DialogButtonNode>;

export const WindowNode = z.strictObject({
  /** 生成するクラスの名前。 */
  name: z.string(),
  class: z.enum(['Window', 'Dialog']),
  properties: Properties.optional(),
  events: Events.optional(),
  menu_bar: MenuBarNode.optional(),
  tool_bar: ToolBarNode.optional(),
  status_bar: StatusBarNode.optional(),
  layout: LayoutNode.optional(),
  children: z.array(WidgetNode).optional(),
  buttons: z.array(DialogButtonNode).optional(),
});
export type WindowNode = z.infer<typeof WindowNode>;

export const ActionNode = z.strictObject({
  name: z.string(),
  properties: Properties.optional(),
  events: Events.optional(),
});
export type ActionNode = z.infer<typeof ActionNode>;

/** Variable の値の型(C++ の `tokotachi::Variable<T>` の T)。 */
export const VariableType = z.enum(['bool', 'int', 'float', 'double', 'string']);
export type VariableType = z.infer<typeof VariableType>;

/**
 * Variable(値を 1 つ持つオブジェクト。docs/adr/0016)。プロパティに `{ "bind": "名前" }` と書いて結びつける。
 * value は初期値(型に合うかは意味の検証で調べる。省略すると T())。
 */
export const VariableNode = z.strictObject({
  name: z.string(),
  type: VariableType,
  value: PropertyValue.optional(),
  events: Events.optional(),
});
export type VariableNode = z.infer<typeof VariableNode>;

/** 画面に出ないオブジェクト(Timer 等)。 */
export const ObjectNode = z.strictObject({
  name: z.string(),
  class: z.string(),
  properties: Properties.optional(),
  events: Events.optional(),
});
export type ObjectNode = z.infer<typeof ObjectNode>;

export const TtuiDocument = z.strictObject({
  $schema: z.string().optional(),
  format_version: z.literal(FORMAT_VERSION),
  window: WindowNode,
  actions: z.array(ActionNode).optional(),
  objects: z.array(ObjectNode).optional(),
  variables: z.array(VariableNode).optional(),
});
export type TtuiDocument = z.infer<typeof TtuiDocument>;
