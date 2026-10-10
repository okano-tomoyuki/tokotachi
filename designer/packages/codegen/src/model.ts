/**
 * 定義ファイルから「部品を組む操作の並び」を作る(docs/designer/codegen-design.md §4 の順)。
 * C++ の書き出し(cpp/emit.ts)はこれを C++ の文にし、キャンバス(WebAssembly の Tokotachi)は同じ並びを実行する。
 * 同じ並びを使うので、キャンバスに描いた画面と、生成したコードが組む画面は同じになる(docs/adr/0039)。
 */
import {
  findClass,
  layoutVariable,
  MENU_BAR_VARIABLE,
  menuVariable,
  namedNodes,
  paneVariable,
  STATUS_BAR_VARIABLE,
  tabVariable,
  TOOL_BAR_VARIABLE,
  type Events,
  type LayoutItem,
  type LayoutNode,
  type MenuNode,
  type NamedNode,
  type PaneNode,
  type Properties,
  type PropertyType,
  type PropertyValue,
  type TabNode,
  type TtuiDocument,
  type WidgetNode,
} from '@tokotachi-designer/core';

/** 呼び出しの引数。 */
export type Arg =
  /** 変数(メンバ・一時的な変数)。`this` はウィンドウ自身。 */
  | { readonly kind: 'ref'; readonly name: string }
  | { readonly kind: 'int'; readonly value: number }
  | { readonly kind: 'float'; readonly value: number }
  | { readonly kind: 'string'; readonly value: string }
  | { readonly kind: 'enum'; readonly enum: string; readonly value: string };

/** 呼び出しの結果を受け取る変数。 */
export interface Result {
  readonly variable: string;
  readonly class: string;
  /** 一時的な変数を宣言する(false ならメンバへの代入)。 */
  readonly declare: boolean;
}

/** target が undefined の操作は、ウィンドウ自身に対するもの。 */
export type Op =
  /** `variable = new tokotachi::Class(parent)`。 */
  | {
      readonly op: 'new';
      readonly variable: string;
      readonly class: string;
      readonly declare: boolean;
      readonly parent?: string | undefined;
    }
  /** `target->property = value`(値は定義ファイルの書き方のまま。参照なら相手の変数の名前)。 */
  | {
      readonly op: 'set';
      readonly target?: string | undefined;
      readonly property: string;
      readonly type: PropertyType;
      readonly value: PropertyValue;
    }
  /** `target->property = [this](...) { handler(...); }`。 */
  | {
      readonly op: 'connect';
      readonly target?: string | undefined;
      readonly property: string;
      readonly handler: string;
      readonly handlerType: string;
    }
  /** `result = target->method(args)`。後ろの既定値の引数は省いてある。 */
  | {
      readonly op: 'call';
      readonly target?: string | undefined;
      readonly method: string;
      readonly args: readonly Arg[];
      readonly result?: Result | undefined;
    }
  /** PlaceLayout に置く(Placement のメンバを設定して add する)。 */
  | {
      readonly op: 'place';
      readonly layout: string;
      readonly widget: string;
      readonly placement: Readonly<Record<string, PropertyValue>>;
    }
  /** C++ の書き出しの区切りの空行(実行には関係しない)。 */
  | { readonly op: 'blank' };

export interface Handler {
  readonly name: string;
  /** ハンドラの型(NotifyHandler 等)。 */
  readonly type: string;
}

export interface SetupModel {
  readonly ops: readonly Op[];
  /** つなぐハンドラ(最初に現れた順。同じ名前は 1 つ)。 */
  readonly handlers: readonly Handler[];
}

const ref = (name: string): Arg => ({ kind: 'ref', name });
const int = (value: number): Arg => ({ kind: 'int', value });
const float = (value: number): Arg => ({ kind: 'float', value });
const str = (value: string): Arg => ({ kind: 'string', value });
const align = (value: string): Arg => ({ kind: 'enum', enum: 'Align', value });

function sameArg(a: Arg, b: Arg): boolean {
  return JSON.stringify(a) === JSON.stringify(b);
}

/** 既定値と同じ後ろの引数を省く(codegen-design.md N6)。defaults の undefined は省けない引数。 */
function trimDefaults(args: readonly Arg[], defaults: readonly (Arg | undefined)[]): Arg[] {
  let n = args.length;
  while (n > 0) {
    const d = defaults[n - 1];
    const a = args[n - 1];
    if (d === undefined || a === undefined || !sameArg(a, d)) {
      break;
    }
    n--;
  }
  return args.slice(0, n);
}

export function buildSetup(doc: TtuiDocument): SetupModel {
  return new SetupBuilder(doc).build();
}

class SetupBuilder {
  private readonly doc: TtuiDocument;
  private readonly ops: Op[] = [];
  private readonly handlers: Handler[] = [];
  /** 名前 → ノード(参照の相手が部品か)。 */
  private readonly named: ReadonlyMap<string, NamedNode>;
  /** ほかの部品への参照(すべての部品を作った後に設定する)。 */
  private readonly deferred: Op[] = [];

  constructor(doc: TtuiDocument) {
    this.doc = doc;
    this.named = new Map(namedNodes(doc).map((n) => [n.name, n]));
  }

  build(): SetupModel {
    const { doc } = this;
    const w = doc.window;
    for (const a of doc.actions ?? []) {
      this.add({ op: 'new', variable: a.name, class: 'Action', declare: false, parent: 'this' });
      this.properties(a.name, 'Action', a.properties);
      this.events(a.name, 'Action', a.events);
      this.blank();
    }
    for (const o of doc.objects ?? []) {
      this.add({ op: 'new', variable: o.name, class: o.class, declare: false, parent: 'this' });
      this.properties(o.name, o.class, o.properties);
      this.events(o.name, o.class, o.events);
      this.blank();
    }
    this.properties(undefined, w.class, w.properties);
    this.events(undefined, w.class, w.events);
    for (const b of w.buttons ?? []) {
      const button: Arg = { kind: 'enum', enum: 'DialogButton', value: b.button };
      this.call(undefined, 'add_button', b.text === undefined ? [button] : [button, str(b.text)]);
    }
    this.blank();
    this.menuBar();
    this.toolBar();
    this.statusBar();
    this.children(w, 'this', undefined);
    if (this.deferred.length > 0) {
      this.blank();
      this.ops.push(...this.deferred);
    }
    while (this.ops.length > 0 && this.ops[this.ops.length - 1]?.op === 'blank') {
      this.ops.pop();
    }
    return { ops: this.ops, handlers: this.handlers };
  }

  private add(op: Op): void {
    this.ops.push(op);
  }

  /** 空行(続けて入れない)。 */
  private blank(): void {
    const last = this.ops[this.ops.length - 1];
    if (last !== undefined && last.op !== 'blank') {
      this.ops.push({ op: 'blank' });
    }
  }

  private call(
    target: string | undefined,
    method: string,
    args: readonly Arg[],
    result?: Result,
  ): void {
    this.add({ op: 'call', target, method, args, result });
  }

  // ---- プロパティ・ハンドラ ----

  /**
   * プロパティを設定する。カタログの順に並べる。
   * @param late true なら after_children の印が付いたものだけ、false ならそれ以外
   */
  private properties(
    target: string | undefined,
    cls: string,
    props: Properties | undefined,
    late = false,
    skip: readonly string[] = [],
  ): void {
    const info = findClass(cls);
    if (!info || !props) {
      return;
    }
    for (const [name, prop] of Object.entries(info.properties)) {
      const value = props[name];
      if (value === undefined || skip.includes(name) || (prop.after_children === true) !== late) {
        continue;
      }
      const op: Op = { op: 'set', target, property: name, type: prop.type, value };
      const isRef = typeof prop.type === 'object' && 'ref' in prop.type;
      if (isRef && typeof value === 'string' && this.named.get(value)?.kind === 'widget') {
        this.deferred.push(op);
      } else {
        this.add(op);
      }
    }
  }

  private events(target: string | undefined, cls: string, events: Events | undefined): void {
    const info = findClass(cls);
    if (!info || !events) {
      return;
    }
    for (const [name, h] of Object.entries(info.handlers)) {
      const handler = events[name];
      if (handler === undefined) {
        continue;
      }
      if (!this.handlers.some((x) => x.name === handler)) {
        this.handlers.push({ name: handler, type: h.type });
      }
      this.add({ op: 'connect', target, property: name, handler, handlerType: h.type });
    }
  }

  /** ウィンドウのメニューバー等に設定する(`menu_bar = window_menu_bar`)。 */
  private attach(property: string, cls: string, variable: string): void {
    this.add({ op: 'set', property, type: { ref: cls }, value: variable });
  }

  // ---- メニューバー・ツールバー・ステータスバー ----

  private menuBar(): void {
    const bar = this.doc.window.menu_bar;
    if (!bar) {
      return;
    }
    const variable = bar.name ?? MENU_BAR_VARIABLE;
    this.add({ op: 'new', variable, class: 'MenuBar', declare: bar.name === undefined });
    this.properties(variable, 'MenuBar', bar.properties);
    bar.menus?.forEach((m, i) => {
      this.menu(m, menuVariable(m, variable, i), variable, 'add_menu');
    });
    this.attach('menu_bar', 'MenuBar', variable);
    this.blank();
  }

  private menu(menu: MenuNode, variable: string, parent: string, method: string): void {
    this.call(parent, method, [str(menu.title)], {
      variable,
      class: 'Menu',
      declare: menu.name === undefined,
    });
    menu.items?.forEach((item, j) => {
      if (item.action !== undefined) {
        this.call(variable, 'add_action', [ref(item.action)]);
      } else if (item.separator) {
        this.call(variable, 'add_separator', []);
      } else if (item.menu) {
        this.menu(item.menu, menuVariable(item.menu, variable, j), variable, 'add_submenu');
      }
    });
  }

  private toolBar(): void {
    const bar = this.doc.window.tool_bar;
    if (!bar) {
      return;
    }
    const variable = bar.name ?? TOOL_BAR_VARIABLE;
    this.add({ op: 'new', variable, class: 'ToolBar', declare: bar.name === undefined });
    this.properties(variable, 'ToolBar', bar.properties);
    for (const item of bar.items ?? []) {
      if (item.action !== undefined) {
        this.call(variable, 'add_action', [ref(item.action)]);
      } else if (item.separator) {
        this.call(variable, 'add_separator', []);
      } else if (item.widget) {
        this.widget(item.widget, variable);
        this.call(variable, 'add_widget', [ref(item.widget.name)]);
      }
    }
    this.attach('tool_bar', 'ToolBar', variable);
    this.blank();
  }

  private statusBar(): void {
    const bar = this.doc.window.status_bar;
    if (!bar) {
      return;
    }
    const variable = bar.name ?? STATUS_BAR_VARIABLE;
    this.add({ op: 'new', variable, class: 'StatusBar', declare: bar.name === undefined });
    this.properties(variable, 'StatusBar', bar.properties);
    for (const w of bar.widgets ?? []) {
      this.widget(w, variable);
      this.call(variable, 'add_widget', [ref(w.name)]);
    }
    this.attach('status_bar', 'StatusBar', variable);
    this.blank();
  }

  // ---- 部品とレイアウト ----

  /**
   * 部品を作り、プロパティ・ハンドラ・子を設定する(親のレイアウトに入れるのは呼び出し側)。
   * @param parent 親の部品の変数(`this`・メンバ・一時的な変数)
   */
  private widget(node: WidgetNode, parent: string): void {
    const name = node.name;
    this.add({ op: 'new', variable: name, class: node.class, declare: false, parent });
    this.properties(name, node.class, node.properties);
    this.events(name, node.class, node.events);
    for (const a of node.actions ?? []) {
      this.call(name, 'add_action', [ref(a)]);
    }
    const hasChildren =
      node.layout !== undefined ||
      node.children !== undefined ||
      node.tabs !== undefined ||
      node.panes !== undefined ||
      node.content !== undefined;
    if (!hasChildren) {
      return;
    }
    this.children(node, name, name);
    this.blank();
    this.properties(name, node.class, node.properties, true);
  }

  /** 子(レイアウト・自由な配置・タブ・区画・中身)。owner は部品の名前(ウィンドウなら undefined)。 */
  private children(
    node: {
      readonly layout?: LayoutNode | undefined;
      readonly children?: readonly WidgetNode[] | undefined;
      readonly tabs?: readonly TabNode[] | undefined;
      readonly panes?: readonly PaneNode[] | undefined;
      readonly content?: WidgetNode | undefined;
    },
    target: string,
    owner: string | undefined,
  ): void {
    if (node.layout) {
      this.layout(node.layout, target, owner);
    }
    for (const c of node.children ?? []) {
      this.blank();
      this.widget(c, target);
    }
    node.tabs?.forEach((tab, i) => {
      this.blank();
      this.widget(tab.page, target);
      const title = tab.properties?.['title'];
      this.withPart(
        'Tab',
        tab.name,
        tabVariable(tab, owner ?? 'window', i),
        target,
        'add_tab',
        [ref(tab.page.name), str(typeof title === 'string' ? title : '')],
        tab.properties,
        ['title'],
      );
    });
    node.panes?.forEach((pane, i) => {
      this.blank();
      this.widget(pane.widget, target);
      const stretch = pane.properties?.['stretch'];
      this.withPart(
        'SplitterPane',
        pane.name,
        paneVariable(pane, owner ?? 'window', i),
        target,
        'add_widget',
        trimDefaults(
          [ref(pane.widget.name), int(typeof stretch === 'number' ? stretch : 1)],
          [undefined, int(1)],
        ),
        pane.properties,
        ['stretch'],
      );
    });
    if (node.content) {
      this.blank();
      this.widget(node.content, target);
      this.add({
        op: 'set',
        target,
        property: 'content',
        type: { ref: 'Widget' },
        value: node.content.name,
      });
    }
  }

  /**
   * add_tab・add_widget の結果(Tab・SplitterPane)に、引数で渡さないプロパティを設定する。
   * 設定するものが無く名前も無ければ、結果を受け取らない。
   */
  private withPart(
    cls: string,
    name: string | undefined,
    variable: string,
    target: string,
    method: string,
    args: readonly Arg[],
    props: Properties | undefined,
    passed: readonly string[],
  ): void {
    const rest = Object.keys(props ?? {}).filter((k) => !passed.includes(k));
    if (name === undefined && rest.length === 0) {
      this.call(target, method, args);
      return;
    }
    this.call(target, method, args, { variable, class: cls, declare: name === undefined });
    this.properties(variable, cls, props, false, passed);
  }

  private layout(layout: LayoutNode, target: string, owner: string | undefined): void {
    const variable = layoutVariable(layout, owner);
    this.add({
      op: 'new',
      variable,
      class: layout.class,
      declare: layout.name === undefined,
      parent: target,
    });
    this.properties(variable, layout.class, layout.properties);
    for (const [kind, tracks] of [
      ['row', layout.rows],
      ['column', layout.columns],
    ] as const) {
      for (const [index, t] of Object.entries(tracks ?? {}).sort(
        ([a], [b]) => Number(a) - Number(b),
      )) {
        if (t.stretch !== undefined) {
          this.call(variable, `set_${kind}_stretch`, [int(Number(index)), int(t.stretch)]);
        }
        if (t.minimum !== undefined) {
          this.call(variable, `set_${kind}_minimum`, [int(Number(index)), float(t.minimum)]);
        }
      }
    }
    for (const item of layout.items ?? []) {
      this.item(layout.class, variable, item, target);
    }
  }

  private item(cls: string, variable: string, item: LayoutItem, target: string): void {
    const w = item.widget;
    this.blank();
    if (!w) {
      if (item.spacing !== undefined) {
        this.call(variable, 'add_spacing', [float(item.spacing)]);
      } else {
        this.call(variable, 'add_stretch', trimDefaults([int(item.stretch ?? 1)], [int(1)]));
      }
      return;
    }
    this.widget(w, target);
    if (cls === 'PlaceLayout') {
      this.add({ op: 'place', layout: variable, widget: w.name, placement: item.placement ?? {} });
      return;
    }
    const args =
      cls === 'GridLayout'
        ? trimDefaults(
            [
              ref(w.name),
              int(item.row ?? 0),
              int(item.column ?? 0),
              int(item.row_span ?? 1),
              int(item.column_span ?? 1),
              align(item.horizontal ?? 'FILL'),
              align(item.vertical ?? 'CENTER'),
            ],
            [undefined, undefined, undefined, int(1), int(1), align('FILL'), align('CENTER')],
          )
        : trimDefaults(
            [ref(w.name), int(item.stretch ?? 0), align(item.alignment ?? 'FILL')],
            [undefined, int(0), align('FILL')],
          );
    this.call(variable, 'add', args);
  }
}
