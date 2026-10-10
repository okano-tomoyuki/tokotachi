/**
 * デザイナーの構造の木(docs/designer/editor-design.md §5)。定義ファイルのノードを、画面の入れ子の順に並べる。
 * キャンバスの部品の位置(キャンバスの描画の結果。変数の名前ごと)と、木のノードを結び付ける変数の名前も持つ。
 */
import type { JsonPath } from './diagnostics.ts';
import type { LayoutNode, MenuNode, TtuiDocument, WidgetNode } from './schema.ts';
import {
  layoutVariable,
  MENU_BAR_VARIABLE,
  menuVariable,
  STATUS_BAR_VARIABLE,
  TOOL_BAR_VARIABLE,
} from './tree.ts';

export type OutlineKind =
  | 'window'
  | 'widget'
  | 'layout'
  | 'spacer'
  | 'tab'
  | 'pane'
  | 'menu_bar'
  | 'menu'
  | 'menu_item'
  | 'separator'
  | 'tool_bar'
  | 'tool_item'
  | 'status_bar'
  | 'group'
  | 'action'
  | 'object'
  | 'variable';

export interface OutlineNode {
  /** パスを `/` でつないだもの(木の中で一意)。 */
  readonly id: string;
  /** ノードの位置。group(Action・オブジェクトのまとまり)は配列の位置。 */
  readonly path: JsonPath;
  readonly kind: OutlineKind;
  /** 表示する名前(名前・タイトル・参照する Action の名前など)。 */
  readonly label: string;
  /** クラスの名前(あれば)。 */
  readonly class?: string;
  /** プロパティ・ハンドラを編集できるノードか。 */
  readonly editable: boolean;
  /** キャンバスの描画の結果の変数の名前(部品・レイアウト・バー)。 */
  readonly variable?: string;
  readonly children: readonly OutlineNode[];
}

const id = (path: JsonPath): string => path.join('/');

function node(
  path: JsonPath,
  kind: OutlineKind,
  label: string,
  rest: Partial<Pick<OutlineNode, 'class' | 'variable' | 'editable'>> = {},
  children: readonly OutlineNode[] = [],
): OutlineNode {
  return { id: id(path), path, kind, label, editable: rest.editable ?? false, children, ...rest };
}

function widget(w: WidgetNode, path: JsonPath): OutlineNode {
  const children: OutlineNode[] = [];
  if (w.layout) {
    children.push(layout(w.layout, [...path, 'layout'], w.name));
  }
  w.children?.forEach((c, i) => {
    children.push(widget(c, [...path, 'children', i]));
  });
  w.tabs?.forEach((t, i) => {
    const p = [...path, 'tabs', i];
    const title = t.properties?.['title'];
    children.push(
      node(
        p,
        'tab',
        t.name ?? (typeof title === 'string' ? title : ''),
        { class: 'Tab', editable: true },
        [widget(t.page, [...p, 'page'])],
      ),
    );
  });
  w.panes?.forEach((pane, i) => {
    const p = [...path, 'panes', i];
    children.push(
      node(p, 'pane', pane.name ?? '', { class: 'SplitterPane', editable: true }, [
        widget(pane.widget, [...p, 'widget']),
      ]),
    );
  });
  if (w.content) {
    children.push(widget(w.content, [...path, 'content']));
  }
  return node(
    path,
    'widget',
    w.name,
    { class: w.class, variable: w.name, editable: true },
    children,
  );
}

function layout(l: LayoutNode, path: JsonPath, owner: string | undefined): OutlineNode {
  const items = (l.items ?? []).map((item, i) => {
    const p = [...path, 'items', i];
    if (item.widget) {
      return widget(item.widget, [...p, 'widget']);
    }
    const label =
      item.spacing !== undefined
        ? `spacing ${String(item.spacing)}`
        : `stretch ${String(item.stretch ?? 1)}`;
    return node(p, 'spacer', label);
  });
  return node(
    path,
    'layout',
    l.name ?? '',
    { class: l.class, variable: layoutVariable(l, owner), editable: true },
    items,
  );
}

function menu(m: MenuNode, path: JsonPath, variable: string): OutlineNode {
  const items = (m.items ?? []).map((item, j) => {
    const p = [...path, 'items', j];
    if (item.menu) {
      return menu(item.menu, [...p, 'menu'], menuVariable(item.menu, variable, j));
    }
    if (item.separator) {
      return node(p, 'separator', '');
    }
    return node(p, 'menu_item', item.action ?? '');
  });
  return node(path, 'menu', m.name ?? m.title, { class: 'Menu' }, items);
}

/** 構造の木の根(ウィンドウ・Action のまとまり・オブジェクトのまとまり)。 */
export function outline(doc: TtuiDocument): OutlineNode[] {
  const w = doc.window;
  const children: OutlineNode[] = [];
  if (w.menu_bar) {
    const p = ['window', 'menu_bar'];
    const variable = w.menu_bar.name ?? MENU_BAR_VARIABLE;
    children.push(
      node(
        p,
        'menu_bar',
        w.menu_bar.name ?? '',
        { class: 'MenuBar', variable, editable: true },
        (w.menu_bar.menus ?? []).map((m, i) =>
          menu(m, [...p, 'menus', i], menuVariable(m, variable, i)),
        ),
      ),
    );
  }
  if (w.tool_bar) {
    const p = ['window', 'tool_bar'];
    children.push(
      node(
        p,
        'tool_bar',
        w.tool_bar.name ?? '',
        { class: 'ToolBar', variable: w.tool_bar.name ?? TOOL_BAR_VARIABLE, editable: true },
        (w.tool_bar.items ?? []).map((item, i) => {
          const ip = [...p, 'items', i];
          if (item.widget) {
            return widget(item.widget, [...ip, 'widget']);
          }
          return item.separator
            ? node(ip, 'separator', '')
            : node(ip, 'tool_item', item.action ?? '');
        }),
      ),
    );
  }
  if (w.status_bar) {
    const p = ['window', 'status_bar'];
    children.push(
      node(
        p,
        'status_bar',
        w.status_bar.name ?? '',
        { class: 'StatusBar', variable: w.status_bar.name ?? STATUS_BAR_VARIABLE, editable: true },
        (w.status_bar.widgets ?? []).map((x, i) => widget(x, [...p, 'widgets', i])),
      ),
    );
  }
  if (w.layout) {
    children.push(layout(w.layout, ['window', 'layout'], undefined));
  }
  w.children?.forEach((c, i) => {
    children.push(widget(c, ['window', 'children', i]));
  });
  const roots: OutlineNode[] = [
    node(['window'], 'window', w.name, { class: w.class, editable: true }, children),
  ];
  if (doc.actions && doc.actions.length > 0) {
    roots.push(
      node(
        ['actions'],
        'group',
        'actions',
        {},
        doc.actions.map((a, i) =>
          node(['actions', i], 'action', a.name, { class: 'Action', editable: true }),
        ),
      ),
    );
  }
  if (doc.objects && doc.objects.length > 0) {
    roots.push(
      node(
        ['objects'],
        'group',
        'objects',
        {},
        doc.objects.map((o, i) =>
          node(['objects', i], 'object', o.name, { class: o.class, editable: true }),
        ),
      ),
    );
  }
  if (doc.variables && doc.variables.length > 0) {
    roots.push(
      node(
        ['variables'],
        'group',
        'variables',
        {},
        doc.variables.map((v, i) =>
          node(['variables', i], 'variable', v.name, { class: 'Variable' }),
        ),
      ),
    );
  }
  return roots;
}

/** 木のノードを id で探す。 */
export function findOutlineNode(
  roots: readonly OutlineNode[],
  target: string,
): OutlineNode | undefined {
  for (const r of roots) {
    if (r.id === target) {
      return r;
    }
    const found = findOutlineNode(r.children, target);
    if (found) {
      return found;
    }
  }
  return undefined;
}

/** 木のノードを、親が先の順にすべて並べる。 */
export function flattenOutline(roots: readonly OutlineNode[]): OutlineNode[] {
  return roots.flatMap((r) => [r, ...flattenOutline(r.children)]);
}
