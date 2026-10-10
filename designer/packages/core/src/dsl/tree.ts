/**
 * 定義ファイルの木をたどる(検証・コード生成・キャンバスで共通)。
 * 生成するコードの一時的な変数の名前(名前の無いレイアウト・メニューバー等。docs/designer/codegen-design.md N3)もここで決める。
 */
import type { JsonPath } from './diagnostics.ts';
import type {
  ColumnNode,
  LayoutNode,
  MenuNode,
  PaneNode,
  TabNode,
  TtuiDocument,
  VariableType,
  WidgetNode,
} from './schema.ts';

/** 部品がどこに置かれているか。 */
export type Placement =
  | { readonly kind: 'layout'; readonly layout: LayoutNode }
  | { readonly kind: 'free' }
  | { readonly kind: 'tab'; readonly tab: TabNode }
  | { readonly kind: 'pane'; readonly pane: PaneNode }
  | { readonly kind: 'content' }
  | { readonly kind: 'tool_bar' }
  | { readonly kind: 'status_bar' };

export interface WidgetEntry {
  readonly node: WidgetNode;
  readonly path: JsonPath;
  readonly placement: Placement;
  /** 親の部品の名前(ウィンドウ・ツールバー・ステータスバーの直下なら undefined)。 */
  readonly parent: string | undefined;
}

/** 部品を木の順(親が先)にすべて並べる。 */
export function widgets(doc: TtuiDocument): WidgetEntry[] {
  const out: WidgetEntry[] = [];
  const visit = (
    node: WidgetNode,
    path: JsonPath,
    placement: Placement,
    parent: string | undefined,
  ): void => {
    out.push({ node, path, placement, parent });
    visitChildren(node, path, node.name);
  };
  const visitLayout = (layout: LayoutNode, path: JsonPath, owner: string | undefined): void => {
    layout.items?.forEach((item, i) => {
      if (item.widget) {
        visit(item.widget, [...path, 'items', i, 'widget'], { kind: 'layout', layout }, owner);
      }
    });
  };
  const visitChildren = (
    node: {
      readonly layout?: LayoutNode | undefined;
      readonly children?: readonly WidgetNode[] | undefined;
      readonly tabs?: readonly TabNode[] | undefined;
      readonly panes?: readonly PaneNode[] | undefined;
      readonly content?: WidgetNode | undefined;
    },
    path: JsonPath,
    owner: string | undefined,
  ): void => {
    if (node.layout) {
      visitLayout(node.layout, [...path, 'layout'], owner);
    }
    node.children?.forEach((c, i) => {
      visit(c, [...path, 'children', i], { kind: 'free' }, owner);
    });
    node.tabs?.forEach((tab, i) => {
      visit(tab.page, [...path, 'tabs', i, 'page'], { kind: 'tab', tab }, owner);
    });
    node.panes?.forEach((pane, i) => {
      visit(pane.widget, [...path, 'panes', i, 'widget'], { kind: 'pane', pane }, owner);
    });
    if (node.content) {
      visit(node.content, [...path, 'content'], { kind: 'content' }, owner);
    }
  };

  const w = doc.window;
  w.tool_bar?.items?.forEach((item, i) => {
    if (item.widget) {
      visit(
        item.widget,
        ['window', 'tool_bar', 'items', i, 'widget'],
        { kind: 'tool_bar' },
        undefined,
      );
    }
  });
  w.status_bar?.widgets?.forEach((node, i) => {
    visit(node, ['window', 'status_bar', 'widgets', i], { kind: 'status_bar' }, undefined);
  });
  visitChildren(w, ['window'], undefined);
  return out;
}

export interface LayoutEntry {
  readonly node: LayoutNode;
  readonly path: JsonPath;
  /** レイアウトが付く部品の名前(ウィンドウなら undefined)。 */
  readonly owner: string | undefined;
}

/** レイアウトを木の順に並べる。 */
export function layouts(doc: TtuiDocument): LayoutEntry[] {
  const out: LayoutEntry[] = [];
  if (doc.window.layout) {
    out.push({ node: doc.window.layout, path: ['window', 'layout'], owner: undefined });
  }
  for (const w of widgets(doc)) {
    if (w.node.layout) {
      out.push({ node: w.node.layout, path: [...w.path, 'layout'], owner: w.node.name });
    }
  }
  return out;
}

export interface MenuEntry {
  readonly node: MenuNode;
  readonly path: JsonPath;
  /** サブメニューなら親のメニュー。 */
  readonly parent: MenuNode | undefined;
}

/** メニューバーのメニューを木の順に並べる。 */
export function menus(doc: TtuiDocument): MenuEntry[] {
  const out: MenuEntry[] = [];
  const visit = (node: MenuNode, path: JsonPath, parent: MenuNode | undefined): void => {
    out.push({ node, path, parent });
    node.items?.forEach((item, i) => {
      if (item.menu) {
        visit(item.menu, [...path, 'items', i, 'menu'], node);
      }
    });
  };
  doc.window.menu_bar?.menus?.forEach((m, i) => {
    visit(m, ['window', 'menu_bar', 'menus', i], undefined);
  });
  return out;
}

// ---- 名前 ----

export type NamedKind =
  | 'widget'
  | 'layout'
  | 'tab'
  | 'pane'
  | 'column'
  | 'menu_bar'
  | 'tool_bar'
  | 'status_bar'
  | 'menu'
  | 'action'
  | 'object'
  | 'variable';

export interface NamedNode {
  readonly name: string;
  readonly kind: NamedKind;
  /** C++ のクラス(Tokotachi の名前空間を除く)。 */
  readonly class: string;
  /** name の位置。 */
  readonly path: JsonPath;
}

/** 生成するクラスのメンバになる、名前を持つノード(定義ファイルの順)。 */
export function namedNodes(doc: TtuiDocument): NamedNode[] {
  const out: NamedNode[] = [];
  const add = (name: string | undefined, kind: NamedKind, cls: string, path: JsonPath): void => {
    if (name !== undefined) {
      out.push({ name, kind, class: cls, path: [...path, 'name'] });
    }
  };
  for (const [i, a] of (doc.actions ?? []).entries()) {
    add(a.name, 'action', 'Action', ['actions', i]);
  }
  for (const [i, o] of (doc.objects ?? []).entries()) {
    add(o.name, 'object', o.class, ['objects', i]);
  }
  for (const [i, v] of (doc.variables ?? []).entries()) {
    add(v.name, 'variable', variableClass(v.type), ['variables', i]);
  }
  const w = doc.window;
  add(w.menu_bar?.name, 'menu_bar', 'MenuBar', ['window', 'menu_bar']);
  for (const m of menus(doc)) {
    add(m.node.name, 'menu', 'Menu', m.path);
  }
  add(w.tool_bar?.name, 'tool_bar', 'ToolBar', ['window', 'tool_bar']);
  add(w.status_bar?.name, 'status_bar', 'StatusBar', ['window', 'status_bar']);
  if (w.layout) {
    add(w.layout.name, 'layout', w.layout.class, ['window', 'layout']);
  }
  for (const e of widgets(doc)) {
    add(e.node.name, 'widget', e.node.class, e.path);
    if (e.node.layout) {
      add(e.node.layout.name, 'layout', e.node.layout.class, [...e.path, 'layout']);
    }
    e.node.tabs?.forEach((t, i) => {
      add(t.name, 'tab', 'Tab', [...e.path, 'tabs', i]);
    });
    e.node.panes?.forEach((p, i) => {
      add(p.name, 'pane', 'SplitterPane', [...e.path, 'panes', i]);
    });
    e.node.columns?.forEach((c, i) => {
      add(c.name, 'column', 'Column', [...e.path, 'columns', i]);
    });
  }
  return out;
}

/** Variable の C++ のクラス(`Variable<std::string>`。Tokotachi の名前空間を除く)。 */
export function variableClass(type: VariableType): string {
  return `Variable<${type === 'string' ? 'std::string' : type}>`;
}

/** 名前の無いレイアウトの一時的な変数の名前(付く部品の名前 + `_layout`、ウィンドウは `window_layout`)。 */
export function layoutVariable(layout: LayoutNode, owner: string | undefined): string {
  return layout.name ?? `${owner ?? 'window'}_layout`;
}

export const MENU_BAR_VARIABLE = 'window_menu_bar';
export const TOOL_BAR_VARIABLE = 'window_tool_bar';
export const STATUS_BAR_VARIABLE = 'window_status_bar';

/** 名前の無いメニューの一時的な変数の名前(親 + `_menu_` + 1 から数えた番号)。 */
export function menuVariable(menu: MenuNode, parentVariable: string, index: number): string {
  return menu.name ?? `${parentVariable}_menu_${String(index + 1)}`;
}

/** 名前の無いタブ・区画の一時的な変数の名前(部品の名前 + `_tab_`・`_pane_` + 1 から数えた番号)。 */
export function tabVariable(tab: TabNode, owner: string, index: number): string {
  return tab.name ?? `${owner}_tab_${String(index + 1)}`;
}

export function paneVariable(pane: PaneNode, owner: string, index: number): string {
  return pane.name ?? `${owner}_pane_${String(index + 1)}`;
}

/** 名前の無い列の一時的な変数の名前(部品の名前 + `_column_` + 1 から数えた番号)。 */
export function columnVariable(column: ColumnNode, owner: string, index: number): string {
  return column.name ?? `${owner}_column_${String(index + 1)}`;
}

/**
 * 生成するコードが使う一時的な変数の名前(名前を持つノードのメンバと衝突してはならない)。
 * 使わないかもしれないもの(プロパティの無いタブ等)も含める(定義ファイルを少し変えただけで衝突が現れないように)。
 */
export function temporaryNames(doc: TtuiDocument): { name: string; path: JsonPath }[] {
  const out: { name: string; path: JsonPath }[] = [];
  const w = doc.window;
  if (w.menu_bar && w.menu_bar.name === undefined) {
    out.push({ name: MENU_BAR_VARIABLE, path: ['window', 'menu_bar'] });
  }
  if (w.tool_bar && w.tool_bar.name === undefined) {
    out.push({ name: TOOL_BAR_VARIABLE, path: ['window', 'tool_bar'] });
  }
  if (w.status_bar && w.status_bar.name === undefined) {
    out.push({ name: STATUS_BAR_VARIABLE, path: ['window', 'status_bar'] });
  }
  // サブメニューの番号は、親のメニューの items の中の位置
  const visitMenu = (m: MenuNode, variable: string, path: JsonPath): void => {
    if (m.name === undefined) {
      out.push({ name: variable, path });
    }
    m.items?.forEach((item, j) => {
      if (item.menu) {
        visitMenu(item.menu, menuVariable(item.menu, variable, j), [...path, 'items', j, 'menu']);
      }
    });
  };
  const barVariable = w.menu_bar?.name ?? MENU_BAR_VARIABLE;
  w.menu_bar?.menus?.forEach((m, i) => {
    visitMenu(m, menuVariable(m, barVariable, i), ['window', 'menu_bar', 'menus', i]);
  });
  for (const l of layouts(doc)) {
    if (l.node.name === undefined) {
      out.push({ name: layoutVariable(l.node, l.owner), path: l.path });
    }
  }
  for (const e of widgets(doc)) {
    e.node.tabs?.forEach((t, i) => {
      if (t.name === undefined) {
        out.push({ name: tabVariable(t, e.node.name, i), path: [...e.path, 'tabs', i] });
      }
    });
    e.node.panes?.forEach((p, i) => {
      if (p.name === undefined) {
        out.push({ name: paneVariable(p, e.node.name, i), path: [...e.path, 'panes', i] });
      }
    });
    e.node.columns?.forEach((c, i) => {
      if (c.name === undefined) {
        out.push({ name: columnVariable(c, e.node.name, i), path: [...e.path, 'columns', i] });
      }
    });
  }
  return out;
}
