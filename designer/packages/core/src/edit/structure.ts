/**
 * 部品の追加・移動・削除と、レイアウトの編集(docs/designer/editor-design.md §3.2)。
 *
 * 部品を入れる場所は「スロット」で表す。レイアウトの項目の位置・自由な配置・タブ・区画・ScrollArea の中身・ツールバー・ステータスバー。
 * 移動は、入れる先の配列を先に(オブジェクトとして)見つけてから元の場所から外す(外したことで位置がずれても、正しい場所に入る)。
 */
import { findClass, getCatalog } from '../catalog/catalog.ts';
import { l10n } from '../l10n.ts';
import type { JsonPath } from '../dsl/diagnostics.ts';
import type {
  LayoutItem,
  Properties,
  PropertyValue,
  TtuiDocument,
  WidgetNode,
} from '../dsl/schema.ts';
import { namedNodes } from '../dsl/tree.ts';

export class CommandError extends Error {}

type Json = Record<string, unknown>;

/** 部品を入れる場所。 */
export type Slot =
  /** レイアウトの項目の位置(GridLayout は行と列、PlaceLayout は置く位置も)。 */
  | {
      readonly kind: 'item';
      readonly layout: JsonPath;
      readonly index: number;
      readonly row?: number;
      readonly column?: number;
      readonly x?: number;
      readonly y?: number;
    }
  /** 自由な配置(レイアウトの無い部品・ウィンドウ)。位置は親の左上から(dp)。 */
  | { readonly kind: 'child'; readonly parent: JsonPath; readonly x: number; readonly y: number }
  /** TabWidget のタブ・Splitter の区画の位置。 */
  | { readonly kind: 'tab' | 'pane'; readonly parent: JsonPath; readonly index: number }
  /** ScrollArea の中身。 */
  | { readonly kind: 'content'; readonly parent: JsonPath }
  /** ツールバー・ステータスバーの位置。 */
  | { readonly kind: 'tool_bar' | 'status_bar'; readonly index: number };

/** キーを消す(JSON のオブジェクトの編集)。 */
function removeKey(object: Json, key: string): void {
  Reflect.deleteProperty(object, key);
}

function isObject(value: unknown): value is Json {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

/** path が指す値(無ければ undefined)。 */
export function valueAt(root: unknown, path: JsonPath): unknown {
  let node = root;
  for (const key of path) {
    node = Array.isArray(node)
      ? (node as unknown[])[Number(key)]
      : isObject(node)
        ? node[String(key)]
        : undefined;
  }
  return node;
}

function objectAt(root: unknown, path: JsonPath): Json {
  const node = valueAt(root, path);
  if (!isObject(node)) {
    throw new CommandError(l10n.t('The target was not found: {0}', path.join('.')));
  }
  return node;
}

/** 配列(無ければ作る)。 */
function arrayOf(owner: Json, key: string): unknown[] {
  const value = owner[key];
  if (Array.isArray(value)) {
    return value as unknown[];
  }
  const created: unknown[] = [];
  owner[key] = created;
  return created;
}

// ---- 名前と既定の内容 ----

/** クラスの名前の snake_case(LineEdit → line_edit)。 */
export function snakeCase(name: string): string {
  return name.replace(/([a-z0-9])([A-Z])/g, '$1_$2').toLowerCase();
}

/** 種類を先頭に付けた、空いている名前(button_1、button_2…)。 */
export function newName(
  doc: TtuiDocument,
  prefix: string,
  taken: ReadonlySet<string> = new Set(),
): string {
  const used = new Set([...namedNodes(doc).map((n) => n.name), ...taken]);
  for (let i = 1; ; i++) {
    const name = `${prefix}_${String(i)}`;
    if (!used.has(name)) {
      return name;
    }
  }
}

/** パレットから足した部品の既定の内容。文字を持つ部品は名前を文字にする(C++Builder と同じ)。入れ物はレイアウトを付ける。 */
export function newWidget(doc: TtuiDocument, cls: string): WidgetNode {
  const name = newName(doc, snakeCase(cls));
  const info = findClass(cls);
  const props: Properties = {};
  if (
    info &&
    'text' in info.properties &&
    ['Button', 'CheckBox', 'RadioButton', 'Label'].includes(cls)
  ) {
    props['text'] = name;
  }
  const node: WidgetNode = {
    name,
    class: cls,
    ...(Object.keys(props).length > 0 ? { properties: props } : {}),
    ...(info?.accepts_children ? { layout: { class: 'BoxLayout' } } : {}),
  };
  return node;
}

// ---- 部品の場所 ----

/** 部品(パスが指すノード)を、入っている場所から外す。外した部品と、元の場所の項目(レイアウトの項目の設定を引き継ぐため)。 */
function detach(doc: Json, path: JsonPath): { widget: Json; item: Json | undefined } {
  const widget = objectAt(doc, path);
  const key = path[path.length - 1];
  const parentPath = path.slice(0, -1);
  if (key === 'widget' || key === 'page') {
    // レイアウトの項目・ツールバーの項目・タブ・区画: その項目ごと外す
    const entryPath = parentPath;
    const list = valueAt(doc, entryPath.slice(0, -1));
    const index = Number(entryPath[entryPath.length - 1]);
    if (!Array.isArray(list)) {
      throw new CommandError(l10n.t('The target was not found: {0}', path.join('.')));
    }
    const [entry] = (list as unknown[]).splice(index, 1);
    return { widget, item: isObject(entry) ? entry : undefined };
  }
  if (key === 'content') {
    delete objectAt(doc, parentPath)['content'];
    return { widget, item: undefined };
  }
  if (typeof key === 'number') {
    // 自由な配置の子・ステータスバーの部品
    const list = valueAt(doc, parentPath);
    if (Array.isArray(list)) {
      (list as unknown[]).splice(key, 1);
      return { widget, item: undefined };
    }
  }
  throw new CommandError(l10n.t('This cannot be moved or removed: {0}', path.join('.')));
}

/** 入れる先(配列と位置、または中身の持ち主)。外す前に見つけておく。 */
interface Destination {
  readonly slot: Slot;
  readonly list?: unknown[];
  readonly owner?: Json;
  readonly layoutClass?: string;
}

function destination(doc: Json, slot: Slot): Destination {
  switch (slot.kind) {
    case 'item': {
      const layout = objectAt(doc, slot.layout);
      return { slot, list: arrayOf(layout, 'items'), layoutClass: String(layout['class']) };
    }
    case 'child':
      return { slot, list: arrayOf(objectAt(doc, slot.parent), 'children') };
    case 'tab':
      return { slot, list: arrayOf(objectAt(doc, slot.parent), 'tabs') };
    case 'pane':
      return { slot, list: arrayOf(objectAt(doc, slot.parent), 'panes') };
    case 'content':
      return { slot, owner: objectAt(doc, slot.parent) };
    case 'tool_bar':
      return { slot, list: arrayOf(objectAt(doc, ['window', 'tool_bar']), 'items') };
    case 'status_bar':
      return { slot, list: arrayOf(objectAt(doc, ['window', 'status_bar']), 'widgets') };
  }
}

/** 引き継ぐレイアウトの項目の設定(同じ種類のレイアウトへ移すときだけ)。 */
const ITEM_SETTINGS: Readonly<Record<string, readonly string[]>> = {
  BoxLayout: ['stretch', 'alignment'],
  GridLayout: ['row_span', 'column_span', 'horizontal', 'vertical'],
  PlaceLayout: ['placement'],
};

function place(
  dest: Destination,
  widget: Json,
  from: { item: Json | undefined; layoutClass?: string },
): void {
  const { slot } = dest;
  const index = (i: number): number => Math.max(0, Math.min(i, dest.list?.length ?? 0));
  switch (slot.kind) {
    case 'item': {
      const item: Json = {};
      const cls = dest.layoutClass ?? '';
      if (from.item && from.layoutClass === cls) {
        for (const k of ITEM_SETTINGS[cls] ?? []) {
          if (from.item[k] !== undefined) {
            item[k] = from.item[k];
          }
        }
      }
      if (cls === 'GridLayout') {
        item['row'] = slot.row ?? 0;
        item['column'] = slot.column ?? 0;
      }
      if (cls === 'PlaceLayout' && (slot.x !== undefined || slot.y !== undefined)) {
        const old = isObject(item['placement']) ? item['placement'] : {};
        item['placement'] = { ...old, x: Math.round(slot.x ?? 0), y: Math.round(slot.y ?? 0) };
      }
      item['widget'] = widget;
      dest.list?.splice(index(slot.index), 0, item);
      return;
    }
    case 'child': {
      const props = isObject(widget['properties']) ? widget['properties'] : {};
      widget['properties'] = { ...props, x: Math.round(slot.x), y: Math.round(slot.y) };
      dest.list?.push(widget);
      return;
    }
    case 'tab': {
      dest.list?.splice(index(slot.index), 0, {
        properties: { title: typeof widget['name'] === 'string' ? widget['name'] : '' },
        page: widget,
      });
      return;
    }
    case 'pane':
      dest.list?.splice(index(slot.index), 0, { widget });
      return;
    case 'content':
      if (dest.owner?.['content'] !== undefined) {
        throw new CommandError(l10n.t('The scroll area already has content'));
      }
      if (dest.owner) {
        dest.owner['content'] = widget;
      }
      return;
    case 'tool_bar':
      dest.list?.splice(index(slot.index), 0, { widget });
      return;
    case 'status_bar':
      dest.list?.splice(index(slot.index), 0, widget);
      return;
  }
}

/** 自由な配置でなくなったら、位置と大きさを消す(レイアウトが決めるので書けない)。 */
function dropGeometry(widget: Json, slot: Slot): void {
  if (slot.kind === 'child' || !isObject(widget['properties'])) {
    return;
  }
  const props = Object.fromEntries(
    Object.entries(widget['properties']).filter(
      ([k]) => !['x', 'y', 'width', 'height'].includes(k),
    ),
  );
  if (Object.keys(props).length > 0) {
    widget['properties'] = props;
  } else {
    delete widget['properties'];
  }
}

/** 入れる先の部品(スロットの持ち主)のパス。自分の中へ移さないように調べる。 */
function slotOwner(slot: Slot): JsonPath {
  switch (slot.kind) {
    case 'item':
      return slot.layout;
    case 'tool_bar':
      return ['window', 'tool_bar'];
    case 'status_bar':
      return ['window', 'status_bar'];
    default:
      return slot.parent;
  }
}

function isInside(path: JsonPath, ancestor: JsonPath): boolean {
  return path.length >= ancestor.length && ancestor.every((k, i) => String(path[i]) === String(k));
}

export function addWidget(doc: TtuiDocument, slot: Slot, widget: WidgetNode): void {
  const root = doc as unknown as Json;
  if (namedNodes(doc).some((n) => n.name === widget.name)) {
    throw new CommandError(l10n.t('The name "{0}" is already used', widget.name));
  }
  const copy = JSON.parse(JSON.stringify(widget)) as Json;
  dropGeometry(copy, slot);
  place(destination(root, slot), copy, { item: undefined });
}

export function moveWidget(doc: TtuiDocument, from: JsonPath, slot: Slot): void {
  const root = doc as unknown as Json;
  if (isInside(slotOwner(slot), from)) {
    throw new CommandError(l10n.t('A widget cannot be moved into itself'));
  }
  const dest = destination(root, slot);
  // 元の項目のレイアウトのクラス(設定を引き継ぐか決める)
  const key = from[from.length - 1];
  const fromLayout =
    key === 'widget' && from[from.length - 3] === 'items'
      ? valueAt(root, from.slice(0, -3))
      : undefined;
  const layoutClass =
    isObject(fromLayout) && typeof fromLayout['class'] === 'string'
      ? fromLayout['class']
      : undefined;
  // 同じ配列の中で後ろへ移すときは、外したぶん位置を 1 つ前にする
  const fromList =
    key === 'widget' || key === 'page'
      ? valueAt(root, from.slice(0, -2))
      : valueAt(root, from.slice(0, -1));
  const fromIndex = Number(key === 'widget' || key === 'page' ? from[from.length - 2] : key);
  const { widget, item } = detach(root, from);
  let target = dest;
  if (
    dest.list !== undefined &&
    dest.list === fromList &&
    'index' in slot &&
    fromIndex < slot.index
  ) {
    target = { ...dest, slot: { ...slot, index: slot.index - 1 } };
  }
  dropGeometry(widget, slot);
  place(target, widget, { item, ...(layoutClass === undefined ? {} : { layoutClass }) });
}

/** ノード(部品・空き・タブ・区画・Action・オブジェクト)を消す。後ろのものから消す(前のものの位置がずれないように)。 */
export function removeNodes(doc: TtuiDocument, paths: readonly JsonPath[]): void {
  const root = doc as unknown as Json;
  // サブメニューは、それを入れているメニューの項目ごと消す
  const normalized = paths.map((p) => (p[p.length - 1] === 'menu' ? p.slice(0, -1) : p));
  const sorted = [...normalized].sort((a, b) => compareDesc(a, b));
  for (const path of sorted) {
    const key = path[path.length - 1];
    const parentKey = path[path.length - 2];
    if (path.length === 1 && path[0] === 'window') {
      throw new CommandError(l10n.t('The window cannot be removed'));
    }
    if (
      typeof key === 'number' &&
      (parentKey === 'items' ||
        parentKey === 'tabs' ||
        parentKey === 'panes' ||
        parentKey === 'menus' ||
        parentKey === 'actions' ||
        parentKey === 'objects' ||
        parentKey === 'variables' ||
        parentKey === 'columns')
    ) {
      // 空き・タブ・区画・メニュー・ツールバーの項目・Action・オブジェクト・Variable・列
      const list = valueAt(root, path.slice(0, -1));
      if (Array.isArray(list)) {
        (list as unknown[]).splice(key, 1);
        continue;
      }
    }
    if (key === 'layout' || key === 'menu_bar' || key === 'tool_bar' || key === 'status_bar') {
      removeKey(objectAt(root, path.slice(0, -1)), key);
      continue;
    }
    detach(root, path);
  }
  for (const key of ['actions', 'objects', 'variables'] as const) {
    const list = root[key];
    if (Array.isArray(list) && list.length === 0) {
      removeKey(root, key);
    }
  }
}

/** 項目を外して空になった配列(items・tabs・panes・children・widgets)を消す(空の配列を書き出さない)。 */
export function tidy(node: unknown): void {
  if (Array.isArray(node)) {
    for (const v of node as unknown[]) {
      tidy(v);
    }
    return;
  }
  if (!isObject(node)) {
    return;
  }
  for (const [key, value] of Object.entries(node)) {
    if (
      Array.isArray(value) &&
      value.length === 0 &&
      ['items', 'tabs', 'panes', 'children', 'widgets', 'menus', 'columns'].includes(key)
    ) {
      removeKey(node, key);
    } else {
      tidy(value);
    }
  }
}

/** パスを後ろのものが先になるように比べる(同じ配列の中では位置の大きいほうが先)。 */
function compareDesc(a: JsonPath, b: JsonPath): number {
  for (let i = 0; i < Math.min(a.length, b.length); i++) {
    const x = a[i];
    const y = b[i];
    if (x === y) {
      continue;
    }
    if (typeof x === 'number' && typeof y === 'number') {
      return y - x;
    }
    return String(y).localeCompare(String(x));
  }
  return b.length - a.length;
}

/** 画面に出ないオブジェクト(Timer 等)を足す。 */
export function addObject(doc: TtuiDocument, cls: string): void {
  if (findClass(cls)?.kind !== 'object') {
    throw new CommandError(l10n.t('"{0}" is not an object', cls));
  }
  const root = doc as unknown as Json;
  arrayOf(root, 'objects').push({ name: newName(doc, snakeCase(cls)), class: cls });
}

/** 一覧の部品に列を足す(見出しの文字を title に書く)。 */
export function addColumn(
  doc: TtuiDocument,
  target: JsonPath,
  title: string,
  index?: number,
): void {
  const node = objectAt(doc, target);
  const cls = typeof node['class'] === 'string' ? node['class'] : '';
  if (!(findClass(cls)?.child_keys ?? []).includes('columns')) {
    throw new CommandError(l10n.t('{0} cannot have "{1}"', cls, 'columns'));
  }
  const list = arrayOf(node, 'columns');
  list.splice(Math.max(0, Math.min(index ?? list.length, list.length)), 0, {
    properties: { title },
  });
}

/** 空き(BoxLayout の伸びる空き・固定の空き)を足す。 */
export function addSpacer(
  doc: TtuiDocument,
  layout: JsonPath,
  index: number,
  spacer: { stretch: number } | { spacing: number },
): void {
  const node = objectAt(doc, layout);
  if (node['class'] !== 'BoxLayout') {
    throw new CommandError(l10n.t('A spacer can be added only to a BoxLayout'));
  }
  const list = arrayOf(node, 'items');
  list.splice(Math.max(0, Math.min(index, list.length)), 0, { ...spacer });
}

/** レイアウトの項目の設定を変える(null なら消す)。 */
export function setItem(
  doc: TtuiDocument,
  item: JsonPath,
  values: Readonly<Record<string, PropertyValue>>,
): void {
  const node = objectAt(doc, item);
  for (const [k, v] of Object.entries(values)) {
    if (k === 'widget') {
      continue;
    }
    if (v === null) {
      removeKey(node, k);
    } else {
      node[k] = v;
    }
  }
}

/** 自由な配置へ移すときの、子の位置と大きさ(キャンバスの描画の結果。親の左上から)。 */
export type Positions = Readonly<Record<string, { readonly x: number; readonly y: number }>>;

/**
 * 部品のレイアウトを変える(null なら外して自由な配置にする)。項目は今の並びのまま、新しいレイアウトの項目にする。
 * BoxLayout から GridLayout へは、並べる向きに 1 列(1 行)に並べる。GridLayout から BoxLayout へは行・列の順に並べる。
 */
export function setLayout(
  doc: TtuiDocument,
  owner: JsonPath,
  cls: string | null,
  positions: Positions = {},
): void {
  const node = objectAt(doc, owner);
  const old = isObject(node['layout']) ? node['layout'] : undefined;
  const oldItems = old && Array.isArray(old['items']) ? (old['items'] as Json[]) : [];
  const children = Array.isArray(node['children']) ? (node['children'] as Json[]) : [];
  // 今の部品の並び(レイアウトの項目の順。GridLayout は行・列の順)
  const ordered =
    old?.['class'] === 'GridLayout'
      ? [...oldItems].sort(
          (a, b) =>
            Number(a['row'] ?? 0) - Number(b['row'] ?? 0) ||
            Number(a['column'] ?? 0) - Number(b['column'] ?? 0),
        )
      : oldItems;
  const widgets = [
    ...ordered.flatMap((i) => (isObject(i['widget']) ? [i['widget']] : [])),
    ...children,
  ];
  if (old?.['class'] === cls) {
    return;
  }
  delete node['children'];
  if (cls === null) {
    delete node['layout'];
    if (widgets.length > 0) {
      node['children'] = widgets.map((w) => {
        const p = positions[String(w['name'])] ?? { x: 0, y: 0 };
        const props = isObject(w['properties']) ? w['properties'] : {};
        return { ...w, properties: { ...props, x: Math.round(p.x), y: Math.round(p.y) } };
      });
    }
    return;
  }
  if (!getCatalog().classes[cls] || getCatalog().classes[cls]?.kind !== 'layout') {
    throw new CommandError(l10n.t('Unknown layout "{0}"', cls));
  }
  const horizontal =
    isObject(old?.['properties']) && old['properties']['orientation'] === 'HORIZONTAL';
  const items: LayoutItem[] = widgets.map((w, i) => {
    dropGeometry(w, { kind: 'item', layout: [], index: 0 });
    const widget = w as unknown as WidgetNode;
    if (cls === 'GridLayout') {
      return { row: horizontal ? 0 : i, column: horizontal ? i : 0, widget };
    }
    if (cls === 'PlaceLayout') {
      const p = positions[widget.name] ?? { x: 0, y: 0 };
      return { placement: { x: Math.round(p.x), y: Math.round(p.y) }, widget };
    }
    return { widget };
  });
  // 余白・間隔は引き継げるものだけ引き継ぐ
  const props: Json = {};
  const known = getCatalog().classes[cls]?.properties ?? {};
  for (const [k, v] of Object.entries(isObject(old?.['properties']) ? old['properties'] : {})) {
    if (k in known) {
      props[k] = v;
    }
  }
  node['layout'] = {
    ...(old && typeof old['name'] === 'string' ? { name: old['name'] } : {}),
    class: cls,
    ...(Object.keys(props).length > 0 ? { properties: props } : {}),
    ...(items.length > 0 ? { items } : {}),
  };
}
