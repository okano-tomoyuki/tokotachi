/**
 * C++ の書き出し(docs/designer/codegen-design.md §2〜§5)。
 *
 * - ヘッダ: 区間 tokotachi_header_begin(インクルードガードと <tokotachi/tokotachi.hpp>)、tokotachi_namespace_begin、
 *   生成するクラス(区間 declarations にメンバとハンドラの宣言)、tokotachi_namespace_end、tokotachi_header_end。
 * - ソース: 区間 tokotachi_source_begin(ヘッダの include)、tokotachi_namespace_begin、コンストラクタ、区間 tokotachi_setup_ui、
 *   ハンドラの雛形、tokotachi_namespace_end。
 * - 生成したコードは tokotachi:: を省かずに書き、using namespace を書かない(N2)。
 */
import {
  findClass,
  getCatalog,
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
  type TabNode,
  type TtuiDocument,
  type WidgetNode,
} from '@tokotachi-designer/core';
import { generatedComments } from '../comments.ts';
import type { CppTarget, CommentLocale } from '../names.ts';
import type { GeneratedCode, HandlerStub, Region } from '../region.ts';
import { cppString, cppValue, enumValue, placementStatements, scalarText } from './values.ts';

const INDENT = '    ';

export interface CppFiles {
  readonly header: GeneratedCode;
  readonly source: GeneratedCode;
}

interface Handler {
  readonly name: string;
  /** ハンドラの型(NotifyHandler 等)。 */
  readonly type: string;
}

/** ハンドラの型の引数の並び(`tokotachi::Object* sender, bool& can_close`)。 */
function paramList(type: string): string {
  return (getCatalog().handler_types[type] ?? [])
    .map((p) => `${/^[A-Z]/.test(p.type) ? `tokotachi::${p.type}` : p.type} ${p.name}`)
    .join(', ');
}

function argList(type: string): string {
  return (getCatalog().handler_types[type] ?? []).map((p) => p.name).join(', ');
}

/** 空行が続かないように行を集める。 */
class Lines {
  readonly lines: string[] = [];

  add(...lines: string[]): void {
    this.lines.push(...lines);
  }

  blank(): void {
    if (this.lines.length > 0 && this.lines[this.lines.length - 1] !== '') {
      this.lines.push('');
    }
  }

  /** 末尾の空行を除いた行。 */
  done(): string[] {
    const out = [...this.lines];
    while (out.length > 0 && out[out.length - 1] === '') {
      out.pop();
    }
    return out;
  }
}

/** tokotachi_setup_ui の中身を作る(§4 の順)。 */
class SetupEmitter {
  private readonly doc: TtuiDocument;
  private readonly out = new Lines();
  readonly handlers: Handler[] = [];
  /** 名前 → ノード(参照の相手が部品か)。 */
  private readonly named: ReadonlyMap<string, NamedNode>;
  /** ほかの部品への参照(すべての部品を作った後に設定する)。 */
  private readonly deferred: string[] = [];

  constructor(doc: TtuiDocument) {
    this.doc = doc;
    this.named = new Map(namedNodes(doc).map((n) => [n.name, n]));
  }

  emit(): string[] {
    const { doc, out } = this;
    const w = doc.window;
    for (const a of doc.actions ?? []) {
      out.add(`${a.name} = new tokotachi::Action(this);`);
      this.properties(a.name, 'Action', a.properties);
      this.events(a.name, 'Action', a.events);
      out.blank();
    }
    for (const o of doc.objects ?? []) {
      out.add(`${o.name} = new tokotachi::${o.class}(this);`);
      this.properties(o.name, o.class, o.properties);
      this.events(o.name, o.class, o.events);
      out.blank();
    }
    this.properties(undefined, w.class, w.properties);
    this.events(undefined, w.class, w.events);
    for (const b of w.buttons ?? []) {
      const text = b.text === undefined ? '' : `, ${cppString(b.text)}`;
      out.add(`add_button(${enumValue('DialogButton', b.button)}${text});`);
    }
    out.blank();
    this.menuBar();
    this.toolBar();
    this.statusBar();
    this.children(w, 'this', undefined);
    if (this.deferred.length > 0) {
      out.blank();
      out.add(...this.deferred);
    }
    return out.done();
  }

  // ---- プロパティ・ハンドラ ----

  /** target が undefined ならウィンドウ自身。 */
  private access(target: string | undefined, member: string): string {
    return target === undefined ? member : `${target}->${member}`;
  }

  /**
   * プロパティを設定する文。カタログの順に書く。
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
      const statement = `${this.access(target, name)} = ${cppValue(prop.type, value)};`;
      const ref = typeof prop.type === 'object' && 'ref' in prop.type;
      if (ref && this.named.get(scalarText(value))?.kind === 'widget') {
        this.deferred.push(statement);
      } else {
        this.out.add(statement);
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
      this.out.add(
        `${this.access(target, name)} = [this](${paramList(h.type)}) { ${handler}(${argList(h.type)}); };`,
      );
    }
  }

  // ---- メニューバー・ツールバー・ステータスバー ----

  private menuBar(): void {
    const bar = this.doc.window.menu_bar;
    if (!bar) {
      return;
    }
    const variable = bar.name ?? MENU_BAR_VARIABLE;
    this.out.add(`${this.declaration('MenuBar', bar.name, variable)} = new tokotachi::MenuBar();`);
    this.properties(variable, 'MenuBar', bar.properties);
    bar.menus?.forEach((m, i) => {
      this.menu(m, menuVariable(m, variable, i), `${variable}->add_menu`);
    });
    this.out.add(`menu_bar = ${variable};`);
    this.out.blank();
  }

  private menu(menu: MenuNode, variable: string, create: string): void {
    this.out.add(
      `${this.declaration('Menu', menu.name, variable)} = ${create}(${cppString(menu.title)});`,
    );
    menu.items?.forEach((item, j) => {
      if (item.action !== undefined) {
        this.out.add(`${variable}->add_action(${item.action});`);
      } else if (item.separator) {
        this.out.add(`${variable}->add_separator();`);
      } else if (item.menu) {
        this.menu(item.menu, menuVariable(item.menu, variable, j), `${variable}->add_submenu`);
      }
    });
  }

  private toolBar(): void {
    const bar = this.doc.window.tool_bar;
    if (!bar) {
      return;
    }
    const variable = bar.name ?? TOOL_BAR_VARIABLE;
    this.out.add(`${this.declaration('ToolBar', bar.name, variable)} = new tokotachi::ToolBar();`);
    this.properties(variable, 'ToolBar', bar.properties);
    for (const item of bar.items ?? []) {
      if (item.action !== undefined) {
        this.out.add(`${variable}->add_action(${item.action});`);
      } else if (item.separator) {
        this.out.add(`${variable}->add_separator();`);
      } else if (item.widget) {
        this.widget(item.widget, variable);
        this.out.add(`${variable}->add_widget(${item.widget.name});`);
      }
    }
    this.out.add(`tool_bar = ${variable};`);
    this.out.blank();
  }

  private statusBar(): void {
    const bar = this.doc.window.status_bar;
    if (!bar) {
      return;
    }
    const variable = bar.name ?? STATUS_BAR_VARIABLE;
    this.out.add(
      `${this.declaration('StatusBar', bar.name, variable)} = new tokotachi::StatusBar();`,
    );
    this.properties(variable, 'StatusBar', bar.properties);
    for (const w of bar.widgets ?? []) {
      this.widget(w, variable);
      this.out.add(`${variable}->add_widget(${w.name});`);
    }
    this.out.add(`status_bar = ${variable};`);
    this.out.blank();
  }

  /** 名前があればメンバへの代入、無ければ一時的な変数の宣言。 */
  private declaration(cls: string, name: string | undefined, variable: string): string {
    return name === undefined ? `tokotachi::${cls}* ${variable}` : variable;
  }

  // ---- 部品とレイアウト ----

  /**
   * 部品を作り、プロパティ・ハンドラ・子を設定する(親のレイアウトに入れる文は呼び出し側が書く)。
   * @param parent 親の部品の式(`this`・メンバ・一時的な変数)
   */
  private widget(node: WidgetNode, parent: string): void {
    const { out } = this;
    const name = node.name;
    out.add(`${name} = new tokotachi::${node.class}(${parent});`);
    this.properties(name, node.class, node.properties);
    this.events(name, node.class, node.events);
    for (const a of node.actions ?? []) {
      out.add(`${name}->add_action(${a});`);
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
    out.blank();
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
    const { out } = this;
    if (node.layout) {
      this.layout(node.layout, target, owner);
    }
    for (const c of node.children ?? []) {
      out.blank();
      this.widget(c, target);
    }
    node.tabs?.forEach((tab, i) => {
      out.blank();
      this.widget(tab.page, target);
      const title = tab.properties?.['title'];
      const call = `${target}->add_tab(${tab.page.name}, ${cppString(typeof title === 'string' ? title : '')})`;
      this.withPart('Tab', tab.name, tabVariable(tab, owner ?? 'window', i), call, tab.properties, [
        'title',
      ]);
    });
    node.panes?.forEach((pane, i) => {
      out.blank();
      this.widget(pane.widget, target);
      const stretch = pane.properties?.['stretch'];
      const args = typeof stretch === 'number' && stretch !== 1 ? `, ${String(stretch)}` : '';
      const call = `${target}->add_widget(${pane.widget.name}${args})`;
      this.withPart(
        'SplitterPane',
        pane.name,
        paneVariable(pane, owner ?? 'window', i),
        call,
        pane.properties,
        ['stretch'],
      );
    });
    if (node.content) {
      out.blank();
      this.widget(node.content, target);
      out.add(`${target}->content = ${node.content.name};`);
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
    call: string,
    props: Properties | undefined,
    passed: readonly string[],
  ): void {
    const rest = Object.keys(props ?? {}).filter((k) => !passed.includes(k));
    if (name === undefined && rest.length === 0) {
      this.out.add(`${call};`);
      return;
    }
    this.out.add(`${this.declaration(cls, name, variable)} = ${call};`);
    this.properties(variable, cls, props, false, passed);
  }

  private layout(layout: LayoutNode, target: string, owner: string | undefined): void {
    const { out } = this;
    const variable = layoutVariable(layout, owner);
    out.add(
      `${this.declaration(layout.class, layout.name, variable)} = new tokotachi::${layout.class}(${target});`,
    );
    this.properties(variable, layout.class, layout.properties);
    for (const [kind, tracks] of [
      ['row', layout.rows],
      ['column', layout.columns],
    ] as const) {
      for (const [index, t] of Object.entries(tracks ?? {}).sort(
        ([a], [b]) => Number(a) - Number(b),
      )) {
        if (t.stretch !== undefined) {
          out.add(`${variable}->set_${kind}_stretch(${index}, ${String(t.stretch)});`);
        }
        if (t.minimum !== undefined) {
          out.add(`${variable}->set_${kind}_minimum(${index}, ${cppValue('float', t.minimum)});`);
        }
      }
    }
    for (const item of layout.items ?? []) {
      this.item(layout.class, variable, item, target);
    }
  }

  private item(cls: string, variable: string, item: LayoutItem, target: string): void {
    const { out } = this;
    const w = item.widget;
    if (!w) {
      out.blank();
      if (item.spacing !== undefined) {
        out.add(`${variable}->add_spacing(${cppValue('float', item.spacing)});`);
      } else {
        const stretch = item.stretch ?? 1;
        out.add(`${variable}->add_stretch(${stretch === 1 ? '' : String(stretch)});`);
      }
      return;
    }
    out.blank();
    this.widget(w, target);
    if (cls === 'PlaceLayout') {
      out.add('{');
      out.add(`${INDENT}tokotachi::Placement placement;`);
      out.add(...placementStatements('placement', item.placement ?? {}).map((s) => INDENT + s));
      out.add(`${INDENT}${variable}->add(${w.name}, placement);`);
      out.add('}');
      return;
    }
    const args =
      cls === 'GridLayout'
        ? trimDefaults(
            [
              String(item.row ?? 0),
              String(item.column ?? 0),
              String(item.row_span ?? 1),
              String(item.column_span ?? 1),
              enumValue('Align', item.horizontal ?? 'FILL'),
              enumValue('Align', item.vertical ?? 'CENTER'),
            ],
            ['', '', '1', '1', enumValue('Align', 'FILL'), enumValue('Align', 'CENTER')],
          )
        : trimDefaults(
            [String(item.stretch ?? 0), enumValue('Align', item.alignment ?? 'FILL')],
            ['0', enumValue('Align', 'FILL')],
          );
    out.add(`${variable}->add(${[w.name, ...args].join(', ')});`);
  }
}

/** 既定値と同じ後ろの引数を省く(N6)。defaults の '' は省けない引数。 */
function trimDefaults(args: readonly string[], defaults: readonly string[]): string[] {
  let n = args.length;
  while (n > 0 && defaults[n - 1] !== '' && args[n - 1] === defaults[n - 1]) {
    n--;
  }
  return args.slice(0, n);
}

/** 名前空間の始まり(C++11 でも通るよう、入れ子は 1 つずつ書く)。 */
function namespaceBegin(namespace: readonly string[]): string {
  return namespace.map((n) => `namespace ${n}\n{`).join('\n');
}

function namespaceEnd(namespace: readonly string[]): string {
  return [...namespace]
    .reverse()
    .map((n) => `}  // namespace ${n}`)
    .join('\n');
}

/**
 * @param sourceName 定義ファイルの名前(生成物の説明に使う)
 */
export function emitCpp(
  doc: TtuiDocument,
  sourceName: string,
  target: CppTarget,
  locale: CommentLocale,
): CppFiles {
  const comments = generatedComments(locale);
  const { className, includeGuard, namespace } = target;
  const base = doc.window.class;
  const setup = new SetupEmitter(doc);
  const body = setup.emit();
  const members = namedNodes(doc);

  const declarations: Region = {
    id: 'declarations',
    indent: 1,
    content: [
      ...members.map((m) => `${INDENT}tokotachi::${m.class}* ${m.name};`),
      ...(members.length > 0 && setup.handlers.length > 0 ? [''] : []),
      ...setup.handlers.map((h) => `${INDENT}void ${h.name}(${paramList(h.type)});`),
    ].join('\n'),
  };
  const headerBegin: Region = {
    id: 'tokotachi_header_begin',
    indent: 0,
    content: [
      `#ifndef ${includeGuard}`,
      `#define ${includeGuard}`,
      '',
      '#include <tokotachi/tokotachi.hpp>',
    ].join('\n'),
  };
  const nsBegin: Region = {
    id: 'tokotachi_namespace_begin',
    indent: 0,
    content: namespaceBegin(namespace),
  };
  const nsEnd: Region = {
    id: 'tokotachi_namespace_end',
    indent: 0,
    content: namespaceEnd(namespace),
  };
  const headerEnd: Region = {
    id: 'tokotachi_header_end',
    indent: 0,
    content: `#endif  // ${includeGuard}`,
  };
  const constructor =
    base === 'Dialog' ? `explicit ${className}(tokotachi::Window* parent);` : `${className}();`;

  const header: GeneratedCode = {
    regions: [headerBegin, nsBegin, declarations, nsEnd, headerEnd],
    stubs: [],
    scaffold: (rendered) =>
      [
        rendered('tokotachi_header_begin'),
        '',
        rendered('tokotachi_namespace_begin'),
        '',
        `/// ${comments.classDoc(sourceName)}`,
        `class ${className} : public tokotachi::${base}`,
        '{',
        'public:',
        `${INDENT}${constructor}`,
        '',
        rendered('declarations'),
        '',
        'private:',
        `${INDENT}void tokotachi_setup_ui();`,
        '};',
        '',
        rendered('tokotachi_namespace_end'),
        '',
        rendered('tokotachi_header_end'),
        '',
      ].join('\n'),
  };

  const setupRegion: Region = {
    id: 'tokotachi_setup_ui',
    indent: 0,
    content: [
      `// ${comments.setupUi}`,
      `void ${className}::tokotachi_setup_ui()`,
      '{',
      ...body.map((l) => (l === '' ? '' : INDENT + l)),
      '}',
    ].join('\n'),
  };
  const stubs: HandlerStub[] = setup.handlers.map((h) => ({
    name: h.name,
    code: [`void ${className}::${h.name}(${paramList(h.type)})`, '{', '}'].join('\n'),
  }));
  const sourceBegin: Region = {
    id: 'tokotachi_source_begin',
    indent: 0,
    content: `#include "${target.headerInclude}"`,
  };
  const init =
    base === 'Dialog'
      ? `${INDENT}: tokotachi::Dialog(parent, std::string())`
      : `${INDENT}: tokotachi::Window(std::string())`;
  const source: GeneratedCode = {
    regions: [sourceBegin, nsBegin, setupRegion, nsEnd],
    stubs,
    scaffold: (rendered) =>
      [
        rendered('tokotachi_source_begin'),
        '',
        rendered('tokotachi_namespace_begin'),
        '',
        `${className}::${base === 'Dialog' ? `${className}(tokotachi::Window* parent)` : `${className}()`}`,
        init,
        '{',
        `${INDENT}tokotachi_setup_ui();`,
        '}',
        '',
        rendered('tokotachi_setup_ui'),
        '',
        '// <tokotachi-designer:handler-stubs>',
        ...stubs.flatMap((s) => [s.code, '']),
        rendered('tokotachi_namespace_end'),
        '',
      ].join('\n'),
  };
  return { header, source };
}
