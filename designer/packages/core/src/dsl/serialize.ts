/**
 * ドキュメントを決まった形のテキストに書き出す(docs/designer/dsl-spec.md §8。Bethany から流用)。
 * 同じ内容なら常に同じテキストになるようにし、1 回の操作による差分を小さくする。
 *
 * - キーの順: ノードごとに決まった順。properties はカタログの順、events もカタログの順、値の型(Margins 等)はメンバの順。
 *   カタログに無いキーは、元の順のまま後ろに置く。レイアウトの項目は widget を最後に置く(設定が先に読めるように)。
 * - properties は 1 行に 1 つのプロパティ(1 つの変更が 1 行の差分になる)。ノード(name を持つオブジェクト)も 1 行にしない。
 * - それ以外で、値が単純なもの(文字列・数値・真偽・それらの配列・そのようなオブジェクト)だけのオブジェクト・配列は、
 *   1 行に収まれば 1 行で書く(events・Margins・メニューの項目等)。
 * - インデントは 2 文字、改行は LF、末尾に改行。
 */
import { findClass, getCatalog } from '../catalog/catalog.ts';
import type { ClassInfo } from '../catalog/types.ts';
import type {
  ActionNode,
  LayoutItem,
  LayoutNode,
  MenuItem,
  MenuNode,
  ObjectNode,
  PaneNode,
  Properties,
  PropertyValue,
  TabNode,
  ToolItem,
  TtuiDocument,
  WidgetNode,
  WindowNode,
} from './schema.ts';

const INDENT = '  ';
/** 1 行で書くときの行の長さの上限(Prettier の printWidth と同じ)。 */
const MAX_WIDTH = 100;

export function serializeDocument(doc: TtuiDocument): string {
  return `${stringify(ordered(doc), '', '')}\n`;
}

// ---- キーの順 ----

type Transform = Readonly<Record<string, (value: unknown) => unknown>>;

function ordered(doc: TtuiDocument): Json {
  return pick(doc, ['$schema', 'format_version', 'window', 'actions', 'objects'], {
    window: (w) => windowNode(w as WindowNode),
    actions: (list) => (list as ActionNode[]).map((a) => classLike(a, 'Action')),
    objects: (list) => (list as ObjectNode[]).map((o) => classLike(o, o.class)),
  });
}

function classLike(node: object, cls: string, keys: readonly string[] = []): Json {
  return pick(node, ['name', 'class', 'properties', 'events', ...keys], members(cls));
}

/** properties・events をカタログの順にする変換。 */
function members(cls: string, extra: Transform = {}): Transform {
  const info = findClass(cls);
  return {
    properties: (p) => properties(p as Properties, info),
    events: (e) => byKeys(e as Json, Object.keys(info?.handlers ?? {})),
    ...extra,
  };
}

const CHILD_TRANSFORMS: Transform = {
  layout: (l) => layoutNode(l as LayoutNode),
  children: (list) => (list as WidgetNode[]).map(widgetNode),
  tabs: (list) =>
    (list as TabNode[]).map((t) =>
      pick(t, ['name', 'properties', 'page'], {
        ...members('Tab'),
        page: (p) => widgetNode(p as WidgetNode),
      }),
    ),
  panes: (list) =>
    (list as PaneNode[]).map((p) =>
      pick(p, ['name', 'properties', 'widget'], {
        ...members('SplitterPane'),
        widget: (w) => widgetNode(w as WidgetNode),
      }),
    ),
  content: (c) => widgetNode(c as WidgetNode),
};

function windowNode(w: WindowNode): Json {
  return pick(
    w,
    [
      'name',
      'class',
      'properties',
      'events',
      'menu_bar',
      'tool_bar',
      'status_bar',
      'layout',
      'children',
      'buttons',
    ],
    members(w.class, {
      ...CHILD_TRANSFORMS,
      menu_bar: (m) =>
        pick(m as object, ['name', 'properties', 'menus'], {
          ...members('MenuBar'),
          menus: (list) => (list as MenuNode[]).map(menuNode),
        }),
      tool_bar: (t) =>
        pick(t as object, ['name', 'properties', 'items'], {
          ...members('ToolBar'),
          items: (list) =>
            (list as ToolItem[]).map((item) =>
              pick(item, ['action', 'separator', 'widget'], {
                widget: (x) => widgetNode(x as WidgetNode),
              }),
            ),
        }),
      status_bar: (s) =>
        pick(s as object, ['name', 'properties', 'widgets'], {
          ...members('StatusBar'),
          widgets: (list) => (list as WidgetNode[]).map(widgetNode),
        }),
      buttons: (list) => (list as object[]).map((b) => pick(b, ['button', 'text'])),
    }),
  );
}

function widgetNode(node: WidgetNode): Json {
  return pick(
    node,
    [
      'name',
      'class',
      'properties',
      'events',
      'actions',
      'layout',
      'children',
      'tabs',
      'panes',
      'content',
    ],
    members(node.class, CHILD_TRANSFORMS),
  );
}

function layoutNode(layout: LayoutNode): Json {
  const placementMembers = Object.keys(getCatalog().values['Placement']?.members ?? {});
  return pick(layout, ['name', 'class', 'properties', 'rows', 'columns', 'items'], {
    ...members(layout.class),
    rows: (r) => tracks(r as Json),
    columns: (c) => tracks(c as Json),
    items: (list) =>
      (list as LayoutItem[]).map((item) =>
        pick(
          item,
          [
            'stretch',
            'alignment',
            'spacing',
            'row',
            'column',
            'row_span',
            'column_span',
            'horizontal',
            'vertical',
            'placement',
            'widget',
          ],
          {
            placement: (p) => byKeys(p as Json, placementMembers),
            widget: (w) => widgetNode(w as WidgetNode),
          },
        ),
      ),
  });
}

/** 行・列の設定は番号の順。 */
function tracks(t: Json): Json {
  const sorted = Object.entries(t).sort(([a], [b]) => Number(a) - Number(b));
  return Object.fromEntries(
    sorted.map(([k, v]) => [k, pick(v as object, ['stretch', 'minimum']) as PropertyValue]),
  );
}

function menuNode(menu: MenuNode): Json {
  return pick(menu, ['name', 'title', 'items'], {
    items: (list) =>
      (list as MenuItem[]).map((item) =>
        pick(item, ['action', 'separator', 'menu'], {
          menu: (m) => menuNode(m as MenuNode),
        }),
      ),
  });
}

/** properties をカタログの順に。値の型(Margins 等)のオブジェクトの中もメンバの順にする。 */
function properties(props: Properties, info: ClassInfo | undefined): Json {
  const known = info?.properties ?? {};
  const sorted = byKeys(props, Object.keys(known));
  const catalog = getCatalog();
  for (const [name, value] of Object.entries(sorted)) {
    const type = Object.hasOwn(known, name) ? known[name]?.type : undefined;
    if (typeof type === 'object' && 'value' in type && isPlainObject(value)) {
      sorted[name] = byKeys(value, Object.keys(catalog.values[type.value]?.members ?? {}));
    }
  }
  return sorted;
}

/** order の順に並べ、order に無いキーは元の順で後ろに置く。 */
function byKeys(object: Readonly<Record<string, PropertyValue>>, order: readonly string[]): Json {
  const rank = new Map(order.map((key, i) => [key, i]));
  const entries = Object.entries(object).map(([k, v], i) => ({ k, v, i }));
  entries.sort(
    (a, b) => (rank.get(a.k) ?? order.length + a.i) - (rank.get(b.k) ?? order.length + b.i),
  );
  return Object.fromEntries(entries.map(({ k, v }) => [k, v]));
}

/** keys の順に並べ(undefined は除く)、transform があれば値を変換する。keys に無いキーは後ろに置く。 */
function pick(object: object, keys: readonly string[], transform: Transform = {}): Json {
  const source = object as Record<string, unknown>;
  const result: Record<string, unknown> = {};
  const all = [...keys, ...Object.keys(source).filter((k) => !keys.includes(k))];
  for (const key of all) {
    const value = source[key];
    if (value === undefined) {
      continue;
    }
    result[key] = Object.hasOwn(transform, key) ? transform[key]?.(value) : value;
  }
  return result as Json;
}

// ---- テキスト ----

type Json = Record<string, PropertyValue>;

function stringify(value: unknown, indent: string, key: string): string {
  const inner = indent + INDENT;
  if (Array.isArray(value)) {
    const items: readonly unknown[] = value;
    if (items.length === 0) {
      return '[]';
    }
    const flat = oneLine(value, indent, key);
    if (flat !== undefined) {
      return flat;
    }
    return `[\n${items.map((v) => inner + stringify(v, inner, '')).join(',\n')}\n${indent}]`;
  }
  if (isPlainObject(value)) {
    const entries = Object.entries(value);
    if (entries.length === 0) {
      return '{}';
    }
    // properties と、ノード(name を持つもの)は 1 行にしない
    if (key !== 'properties' && !Object.hasOwn(value, 'name')) {
      const flat = oneLine(value, indent, key);
      if (flat !== undefined) {
        return flat;
      }
    }
    const body = entries
      .map(([k, v]) => `${inner}${JSON.stringify(k)}: ${stringify(v, inner, k)}`)
      .join(',\n');
    return `{\n${body}\n${indent}}`;
  }
  return JSON.stringify(value);
}

/** 単純な値だけで、`"key": ` の後に 1 行で収まるなら、その 1 行。 */
function oneLine(value: unknown, indent: string, key: string): string | undefined {
  if (!isSimple(value, 0)) {
    return undefined;
  }
  const text = flat(value);
  const prefix = key === '' ? 0 : JSON.stringify(key).length + 2;
  // 後ろの "," の分も数える
  return indent.length + prefix + text.length + 1 <= MAX_WIDTH ? text : undefined;
}

/**
 * 1 行で書ける値: 単純な値、単純な値の配列、単純な値を値に持つオブジェクト、そのようなオブジェクトの配列
 * (メニューの項目の並び等。入れ子は 1 段まで)。
 */
function isSimple(value: unknown, depth: number): boolean {
  if (isScalar(value)) {
    return true;
  }
  if (Array.isArray(value)) {
    return value.every((v) => isScalar(v) || (depth === 0 && isFlatObject(v)));
  }
  if (isPlainObject(value)) {
    return (
      depth === 0 &&
      !Object.hasOwn(value, 'name') &&
      Object.values(value).every((v) => isScalar(v) || (Array.isArray(v) && v.every(isScalar)))
    );
  }
  return false;
}

function isFlatObject(value: unknown): boolean {
  return (
    isPlainObject(value) && !Object.hasOwn(value, 'name') && Object.values(value).every(isScalar)
  );
}

function flat(value: unknown): string {
  if (Array.isArray(value)) {
    return `[${value.map(flat).join(', ')}]`;
  }
  if (isPlainObject(value)) {
    const entries = Object.entries(value);
    return entries.length === 0
      ? '{}'
      : `{ ${entries.map(([k, v]) => `${JSON.stringify(k)}: ${flat(v)}`).join(', ')} }`;
  }
  return JSON.stringify(value);
}

function isScalar(value: unknown): boolean {
  return value === null || ['string', 'number', 'boolean'].includes(typeof value);
}

function isPlainObject(value: unknown): value is Record<string, PropertyValue> {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}
