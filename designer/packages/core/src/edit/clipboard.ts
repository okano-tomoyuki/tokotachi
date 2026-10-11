/**
 * コピー・貼り付け(docs/designer/editor-design.md §3.6)。選んだノードを JSON にしてクリップボードに置き、貼り付けるときは
 * 名前が重なるものを改名してから入れる。クリップボードはシステムのものなので、ほかの定義ファイルにも貼り付けられる。
 */
import { findClass } from '../catalog/catalog.ts';
import type { JsonPath } from '../dsl/diagnostics.ts';
import {
  ActionNode,
  ObjectNode,
  VariableNode,
  WidgetNode,
  type TtuiDocument,
} from '../dsl/schema.ts';
import { namedNodes } from '../dsl/tree.ts';
import { validateDocument } from '../dsl/validate.ts';
import { l10n } from '../l10n.ts';
import { addWidget, CommandError, valueAt, type Slot } from './structure.ts';

/** クリップボードの JSON の目印。 */
export const CLIPBOARD_FORMAT = 'tokotachi-designer/nodes';

export type ClipboardItem =
  | { readonly kind: 'widget'; readonly node: WidgetNode }
  | { readonly kind: 'action'; readonly node: ActionNode }
  | { readonly kind: 'object'; readonly node: ObjectNode }
  | { readonly kind: 'variable'; readonly node: VariableNode };

export interface ClipboardData {
  readonly format: typeof CLIPBOARD_FORMAT;
  readonly version: 1;
  readonly items: readonly ClipboardItem[];
}

type Json = Record<string, unknown>;

function isObject(value: unknown): value is Json {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

function clone<T>(value: T): T {
  return JSON.parse(JSON.stringify(value)) as T;
}

/** path のノードをコピーできる形にする(部品は子孫ごと)。コピーできないノードなら undefined。 */
function itemAt(doc: TtuiDocument, path: JsonPath): ClipboardItem | undefined {
  const node = valueAt(doc, path);
  if (!isObject(node)) {
    return undefined;
  }
  if (path.length === 2) {
    switch (path[0]) {
      case 'actions':
        return { kind: 'action', node: clone(node) as ActionNode };
      case 'objects':
        return { kind: 'object', node: clone(node) as ObjectNode };
      case 'variables':
        return { kind: 'variable', node: clone(node) as VariableNode };
      default:
        break;
    }
  }
  const cls = node['class'];
  if (path.length > 1 && typeof cls === 'string' && findClass(cls)?.kind === 'widget') {
    return { kind: 'widget', node: clone(node) as unknown as WidgetNode };
  }
  return undefined;
}

/** コピーできるノードか(部品・Action・オブジェクト・Variable)。 */
export function canCopy(doc: TtuiDocument, path: JsonPath): boolean {
  return itemAt(doc, path) !== undefined;
}

/** ノードをコピーする。コピーできるものが無ければ undefined。 */
export function copyNodes(
  doc: TtuiDocument,
  paths: readonly JsonPath[],
): ClipboardData | undefined {
  const items = paths.flatMap((p) => {
    const item = itemAt(doc, p);
    return item ? [item] : [];
  });
  return items.length === 0 ? undefined : { format: CLIPBOARD_FORMAT, version: 1, items };
}

export function serializeClipboard(data: ClipboardData): string {
  return JSON.stringify(data, null, 2);
}

/** クリップボードの文字を読む。デザイナーがコピーしたものでなければ undefined。 */
export function parseClipboard(text: string): ClipboardData | undefined {
  let json: unknown;
  try {
    json = JSON.parse(text);
  } catch {
    return undefined;
  }
  if (!isObject(json) || json['format'] !== CLIPBOARD_FORMAT || json['version'] !== 1) {
    return undefined;
  }
  const raw = Array.isArray(json['items']) ? (json['items'] as unknown[]) : [];
  const items: ClipboardItem[] = [];
  for (const r of raw) {
    if (!isObject(r)) {
      return undefined;
    }
    const schema =
      r['kind'] === 'widget'
        ? WidgetNode
        : r['kind'] === 'action'
          ? ActionNode
          : r['kind'] === 'object'
            ? ObjectNode
            : r['kind'] === 'variable'
              ? VariableNode
              : undefined;
    const node = schema?.safeParse(r['node']);
    if (!node?.success) {
      return undefined;
    }
    items.push({ kind: r['kind'], node: node.data } as ClipboardItem);
  }
  return items.length === 0 ? undefined : { format: CLIPBOARD_FORMAT, version: 1, items };
}

/** namedNodes の位置(名前の欄)から、ノードの位置にする。 */
function nodePath(namePath: JsonPath): JsonPath {
  return namePath[namePath.length - 1] === 'name' ? namePath.slice(0, -1) : namePath;
}

/** 名前を持つノードの、item の中での位置(部品は子孫まで。名前の無いノードは除く)。 */
function namedPaths(item: ClipboardItem): { name: string; path: JsonPath }[] {
  if (item.kind !== 'widget') {
    return [{ name: item.node.name, path: [] }];
  }
  // 部品だけのウィンドウに入れて、定義ファイルと同じ規則で名前を持つノードを探す
  const prefix = ['window', 'children', 0];
  const temp = {
    format_version: 1,
    window: { name: 'Clipboard', class: 'Window', children: [item.node] },
  } as unknown as TtuiDocument;
  return namedNodes(temp).flatMap((n) =>
    prefix.every((key, i) => n.path[i] === key)
      ? [{ name: n.name, path: nodePath(n.path).slice(prefix.length) }]
      : [],
  );
}

/** 重ならない名前(末尾の _数字 を付け替える。button_ok → button_ok_2、button_ok_2 → button_ok_3)。 */
function freeName(name: string, taken: ReadonlySet<string>): string {
  const match = /^(.*?)(?:_(\d+))?$/.exec(name);
  const base = match?.[1] !== undefined && match[1] !== '' ? match[1] : name;
  for (let i = match?.[2] !== undefined ? Number(match[2]) + 1 : 2; ; i++) {
    const candidate = `${base}_${String(i)}`;
    if (!taken.has(candidate)) {
      return candidate;
    }
  }
}

/**
 * 貼り付ける前に、ドキュメントの名前(とほかの項目の名前)と重なる名前を改名した写しを作る。
 * 文字(text)が元の名前と同じなら(パレットから足したまま)、文字も新しい名前にする。
 */
export function preparePaste(doc: TtuiDocument, items: readonly ClipboardItem[]): ClipboardItem[] {
  const taken = new Set([...namedNodes(doc).map((n) => n.name), doc.window.name]);
  return items.map((item) => {
    const copy = clone(item);
    for (const { name, path } of namedPaths(copy)) {
      if (!taken.has(name)) {
        taken.add(name);
        continue;
      }
      const next = freeName(name, taken);
      taken.add(next);
      const node = valueAt(copy.node, path) as Json;
      node['name'] = next;
      const properties = node['properties'];
      if (isObject(properties) && properties['text'] === name) {
        properties['text'] = next;
      }
    }
    return copy;
  });
}

/** path の値を消す(配列の要素なら詰める)。 */
function removeAt(doc: TtuiDocument, path: JsonPath): void {
  const parent = valueAt(doc, path.slice(0, -1));
  const key = path[path.length - 1];
  if (Array.isArray(parent) && typeof key === 'number') {
    parent.splice(key, 1);
  } else if (isObject(parent) && key !== undefined) {
    Reflect.deleteProperty(parent, String(key));
  }
}

/** 位置が base の中(base 自身を含む)か。 */
function isInside(path: JsonPath, base: JsonPath): boolean {
  return base.length <= path.length && base.every((key, i) => path[i] === key);
}

/** item を入れた場所(部品は slot から、ほかは一覧の終わり)。 */
function insertedPaths(doc: TtuiDocument, items: readonly ClipboardItem[]): JsonPath[] {
  const names = new Set(items.map((i) => i.node.name));
  return namedNodes(doc).flatMap((n) => (names.has(n.name) ? [nodePath(n.path)] : []));
}

/**
 * 貼り付ける(preparePaste で改名した後の items)。部品は slot に入れ、Action・オブジェクト・Variable は一覧の終わりに足す。
 * 貼り付けたものが、このドキュメントに無い Action・Variable・部品を参照していれば、その参照を外す(ほかのファイルからの貼り付け)。
 */
export function pasteNodes(
  doc: TtuiDocument,
  items: readonly ClipboardItem[],
  slot: Slot | undefined,
): void {
  const prepared = preparePaste(doc, items);
  const widgets = prepared.filter((i) => i.kind === 'widget');
  if (widgets.length > 0 && !slot) {
    throw new CommandError(l10n.t('Select where to paste the widgets'));
  }
  const before = new Set(
    validateDocument(doc)
      .filter((d) => d.severity === 'error')
      .map((d) => `${d.code}:${d.path.join('.')}`),
  );
  const root = doc as unknown as Json;
  const listOf = (key: string): unknown[] => {
    const list = Array.isArray(root[key]) ? (root[key] as unknown[]) : [];
    root[key] = list;
    return list;
  };
  for (const item of prepared) {
    switch (item.kind) {
      case 'action':
        listOf('actions').push(item.node);
        break;
      case 'object':
        listOf('objects').push(item.node);
        break;
      case 'variable':
        listOf('variables').push(item.node);
        break;
      case 'widget':
        break;
    }
  }
  // 同じ位置に後ろから入れて、コピーした順に並べる
  for (const item of [...widgets].reverse()) {
    if (slot) {
      addWidget(doc, slot, item.node);
    }
  }
  // 貼り付けたものの中の、見つからない参照を外す(外すと位置がずれるので、1 つずつ調べ直す)
  for (let i = 0; i < 100; i++) {
    const pasted = insertedPaths(doc, prepared);
    const broken = validateDocument(doc).find(
      (d) =>
        d.severity === 'error' &&
        d.code === 'unknown-reference' &&
        !before.has(`${d.code}:${d.path.join('.')}`) &&
        pasted.some((p) => isInside(d.path, p)),
    );
    if (!broken) {
      break;
    }
    removeAt(doc, broken.path);
  }
}
