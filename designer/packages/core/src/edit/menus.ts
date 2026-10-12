/**
 * メニュー・ツールバー・Action の編集(docs/designer/editor-design.md §3.3)。
 * メニュー・ツールバーの項目は Action を名前で参照する(ADR 0027)。項目を足すときは、ふつう新しい Action を作って参照させる。
 */
import { l10n } from '../l10n.ts';
import type { JsonPath } from '../dsl/diagnostics.ts';
import type { TtuiDocument } from '../dsl/schema.ts';
import { namedNodes } from '../dsl/tree.ts';
import { CommandError, valueAt } from './structure.ts';

type Json = Record<string, unknown>;

function isObject(value: unknown): value is Json {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

function objectAt(root: unknown, path: JsonPath): Json {
  const node = valueAt(root, path);
  if (!isObject(node)) {
    throw new CommandError(l10n.t('The target was not found: {0}', path.join('.')));
  }
  return node;
}

function listOf(owner: Json, key: string): unknown[] {
  const value = owner[key];
  if (Array.isArray(value)) {
    return value as unknown[];
  }
  const created: unknown[] = [];
  owner[key] = created;
  return created;
}

function insertAt(list: unknown[], index: number, value: unknown): void {
  list.splice(Math.max(0, Math.min(index, list.length)), 0, value);
}

export type Bar = 'menu_bar' | 'tool_bar' | 'status_bar';

/** ウィンドウにメニューバー・ツールバー・ステータスバーを付ける。 */
export function addBar(doc: TtuiDocument, bar: Bar): void {
  const window = doc.window as unknown as Json;
  if (window[bar] !== undefined) {
    throw new CommandError(l10n.t('The window already has a {0}', bar));
  }
  window[bar] = {};
}

/** Action を足す(名前は呼び出し側が決める。同じ batch の中で項目から参照するため)。文字は名前にする。 */
export function addAction(doc: TtuiDocument, name: string, index?: number): void {
  if (namedNodes(doc).some((n) => n.name === name)) {
    throw new CommandError(l10n.t('The name "{0}" is already used', name));
  }
  const root = doc as unknown as Json;
  const list = listOf(root, 'actions');
  insertAt(list, index ?? list.length, { name, properties: { text: name } });
}

/**
 * メニューを足す。parent がメニューバーならメニューバーのメニュー、メニューならサブメニュー(その items の位置)。
 */
export function addMenu(doc: TtuiDocument, parent: JsonPath, index: number, title: string): void {
  const node = objectAt(doc, parent);
  if (parent[parent.length - 1] === 'menu_bar') {
    insertAt(listOf(node, 'menus'), index, { title });
  } else {
    insertAt(listOf(node, 'items'), index, { menu: { title } });
  }
}

/** メニュー・ツールバーの項目(Action の参照か区切り)を足す。container はメニューかツールバー。 */
export function addEntry(
  doc: TtuiDocument,
  container: JsonPath,
  index: number,
  entry: { readonly action: string } | { readonly separator: true },
): void {
  insertAt(listOf(objectAt(doc, container), 'items'), index, { ...entry });
}

/** 項目が参照する Action を変える。 */
export function setEntryAction(doc: TtuiDocument, entry: JsonPath, action: string): void {
  const node = objectAt(doc, entry);
  if (node['action'] === undefined) {
    throw new CommandError(l10n.t('The item does not refer to an action'));
  }
  node['action'] = action;
}

/** メニューの見出しを変える。 */
export function setMenuTitle(doc: TtuiDocument, menu: JsonPath, title: string): void {
  objectAt(doc, menu)['title'] = title;
}

/** 同じ並びの中で前後に動かす(delta は -1 か 1)。メニュー・項目・Action・レイアウトの項目・タブ・区画など。 */
export function moveSibling(doc: TtuiDocument, target: JsonPath, delta: number): void {
  // 部品の中身(レイアウトの項目の widget・タブの page 等)を指していれば、それを入れている項目を動かす
  let path = target;
  const last = path[path.length - 1];
  if (last === 'widget' || last === 'page' || last === 'menu') {
    path = path.slice(0, -1);
  }
  const index = path[path.length - 1];
  const list = valueAt(doc, path.slice(0, -1));
  if (typeof index !== 'number' || !Array.isArray(list)) {
    throw new CommandError(l10n.t('This cannot be moved'));
  }
  const to = index + delta;
  if (to < 0 || to >= list.length) {
    return;
  }
  const items = list as unknown[];
  const [moved] = items.splice(index, 1);
  items.splice(to, 0, moved);
}

/**
 * 複数を、それぞれ同じ並びの中でまとめて前後に動かす(delta は -1 か 1)。端に着いたものは動かさず、その次のものも追い越さない。
 * 動かした後の位置(targets と同じ順。部品の中身を指していれば、その中身の位置)を返す。
 */
export function moveSiblings(
  doc: TtuiDocument,
  targets: readonly JsonPath[],
  delta: number,
): JsonPath[] {
  // 部品の中身(widget・page・menu)を指していれば、それを入れている項目を動かす
  const items = targets.map((t) => {
    const last = t[t.length - 1];
    return last === 'widget' || last === 'page' || last === 'menu' ? t.slice(0, -1) : t;
  });
  const results = targets.map((t) => [...t]);
  // 並びごとにまとめ、深いものから動かす(浅いものを動かしても、深いものの位置の書き換えで追える)
  const lists = new Map<string, { list: JsonPath; indices: number[] }>();
  for (const item of items) {
    const index = item[item.length - 1];
    const list = item.slice(0, -1);
    if (typeof index !== 'number' || !Array.isArray(valueAt(doc, list))) {
      throw new CommandError(l10n.t('This cannot be moved'));
    }
    const key = JSON.stringify(list);
    const entry = lists.get(key) ?? { list, indices: [] };
    if (!entry.indices.includes(index)) {
      entry.indices.push(index);
    }
    lists.set(key, entry);
  }
  const ordered = [...lists.values()].sort((a, b) => b.list.length - a.list.length);
  for (const { list, indices } of ordered) {
    const array = valueAt(doc, list) as unknown[];
    // 進む向きの先頭から動かす。端に着いたもの(と、それに続くもの)は動かさない
    const sorted = [...indices].sort((a, b) => (delta < 0 ? a - b : b - a));
    let edge = delta < 0 ? 0 : array.length - 1;
    for (const from of sorted) {
      if (from === edge) {
        edge -= delta;
        continue;
      }
      const to = from + delta;
      const [moved] = array.splice(from, 1);
      array.splice(to, 0, moved);
      // 入れ替えた 2 つの位置を、その中を指す結果の位置で書き換える
      for (const r of results) {
        if (r.length > list.length && list.every((key, i) => r[i] === key)) {
          const at = r[list.length];
          r[list.length] = at === from ? to : at === to ? from : (at as number);
        }
      }
    }
  }
  return results;
}

/** Action を消すときに、その Action を参照する項目・部品の actions も消す(参照が切れないように)。 */
export function removeActionReferences(doc: TtuiDocument, name: string): void {
  const visit = (node: unknown, path: JsonPath): void => {
    if (Array.isArray(node)) {
      const list = node as unknown[];
      for (let i = list.length - 1; i >= 0; i--) {
        const item = list[i];
        if (isObject(item) && item['action'] === name) {
          list.splice(i, 1);
        } else {
          visit(item, [...path, i]);
        }
      }
      return;
    }
    if (!isObject(node)) {
      return;
    }
    const actions = node['actions'];
    if (path.length > 0 && Array.isArray(actions)) {
      node['actions'] = (actions as unknown[]).filter((a) => a !== name);
      if ((node['actions'] as unknown[]).length === 0) {
        Reflect.deleteProperty(node, 'actions');
      }
    }
    for (const [key, value] of Object.entries(node)) {
      if (key !== 'properties' && key !== 'events') {
        visit(value, [...path, key]);
      }
    }
  };
  visit(doc, []);
}
