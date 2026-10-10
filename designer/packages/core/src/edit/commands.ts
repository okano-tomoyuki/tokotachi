/**
 * 編集コマンド(docs/designer/editor-design.md §3)。画面の操作を、定義ファイルに対する意図として表す。
 * Webview は自分のドキュメントに適用してすぐに表示し、拡張は TextDocument の現在の内容に同じコマンドを適用して書き出す
 * (Bethany・tk-designer ADR 0006 と同じ)。
 *
 * ノードは JSON 上の位置(パス)で指す。レイアウト・タブ・区画のように名前の無いノードもあるため。
 * パスが指すノードの種類が合わなければ、コマンドは失敗する(テキストエディタでの変更とすれ違ったとき)。
 */
import { findClass, getCatalog } from '../catalog/catalog.ts';
import { memberNameProblem, isValidIdentifier, describeIdentifierProblem } from '../identifier.ts';
import { l10n } from '../l10n.ts';
import type { JsonPath } from '../dsl/diagnostics.ts';
import type { PropertyValue, TtuiDocument } from '../dsl/schema.ts';
import { namedNodes } from '../dsl/tree.ts';

export type EditCommand =
  /** プロパティを設定する。値が null なら書いたものを消す(既定値に戻す)。 */
  | {
      readonly kind: 'setProperties';
      readonly target: JsonPath;
      readonly properties: Readonly<Record<string, PropertyValue>>;
    }
  /** ハンドラのメンバ関数の名前を設定する。null なら消す。 */
  | {
      readonly kind: 'setEvent';
      readonly target: JsonPath;
      readonly event: string;
      readonly handler: string | null;
    }
  /** ノードの名前を変える(ウィンドウなら生成するクラスの名前)。参照(Action の名前等)も書き換える。 */
  | { readonly kind: 'rename'; readonly target: JsonPath; readonly name: string }
  /** 複数のコマンドを 1 回の変更として適用する(どれかが失敗したら何も変えない)。 */
  | { readonly kind: 'batch'; readonly commands: readonly EditCommand[] };

export type CommandResult =
  | { readonly ok: true; readonly document: TtuiDocument }
  | { readonly ok: false; readonly error: string };

type Json = Record<string, unknown>;

class CommandError extends Error {}

function isObject(value: unknown): value is Json {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

/** path が指すオブジェクト(無ければ例外)。 */
function nodeAt(root: Json, path: JsonPath): Json {
  let node: unknown = root;
  for (const key of path) {
    node = Array.isArray(node)
      ? (node as unknown[])[Number(key)]
      : isObject(node)
        ? node[String(key)]
        : undefined;
  }
  if (!isObject(node)) {
    throw new CommandError(l10n.t('The target was not found: {0}', path.join('.')));
  }
  return node;
}

/** ノードのクラス(Action・メニューバー等、class を書かないノードは位置から決める)。 */
function classOf(path: JsonPath, node: Json): string {
  if (typeof node['class'] === 'string') {
    return node['class'];
  }
  const last = path[path.length - 1];
  const parentKey = path[path.length - 2];
  if (path[0] === 'actions') {
    return 'Action';
  }
  if (last === 'menu_bar') {
    return 'MenuBar';
  }
  if (last === 'tool_bar') {
    return 'ToolBar';
  }
  if (last === 'status_bar') {
    return 'StatusBar';
  }
  if (parentKey === 'tabs') {
    return 'Tab';
  }
  if (parentKey === 'panes') {
    return 'SplitterPane';
  }
  throw new CommandError(l10n.t('The target has no properties: {0}', path.join('.')));
}

function setProperties(
  doc: Json,
  path: JsonPath,
  values: Readonly<Record<string, PropertyValue>>,
): void {
  const node = nodeAt(doc, path);
  // null は消す
  const merged: Json = { ...(isObject(node['properties']) ? node['properties'] : {}), ...values };
  const props = Object.fromEntries(Object.entries(merged).filter(([, v]) => v !== null));
  if (Object.keys(props).length > 0) {
    node['properties'] = props;
  } else {
    delete node['properties'];
  }
}

/** ドキュメントの中の、同じ名前のハンドラの型(無ければ undefined)。 */
function handlerType(
  doc: TtuiDocument,
  handler: string,
  except: { path: JsonPath; event: string },
): string | undefined {
  const found = new Set<string>();
  const visit = (node: unknown, path: JsonPath): void => {
    if (Array.isArray(node)) {
      node.forEach((v, i) => {
        visit(v, [...path, i]);
      });
      return;
    }
    if (!isObject(node)) {
      return;
    }
    const events = node['events'];
    if (isObject(events)) {
      let cls: string | undefined;
      try {
        cls = classOf(path, node);
      } catch {
        cls = undefined;
      }
      const info = cls === undefined ? undefined : findClass(cls);
      for (const [event, name] of Object.entries(events)) {
        const same = JSON.stringify(path) === JSON.stringify(except.path) && event === except.event;
        const type = info?.handlers[event]?.type;
        if (name === handler && !same && type !== undefined) {
          found.add(type);
        }
      }
    }
    for (const [key, value] of Object.entries(node)) {
      if (key !== 'events' && key !== 'properties') {
        visit(value, [...path, key]);
      }
    }
  };
  visit(doc, []);
  return [...found][0];
}

function setEvent(doc: TtuiDocument, path: JsonPath, event: string, handler: string | null): void {
  const node = nodeAt(doc, path);
  const info = findClass(classOf(path, node));
  const h = info?.handlers[event];
  if (!h) {
    throw new CommandError(l10n.t('Unknown handler "{0}"', event));
  }
  const current: Json = isObject(node['events']) ? node['events'] : {};
  const events = Object.fromEntries(Object.entries(current).filter(([e]) => e !== event));
  if (handler !== null) {
    const problem = memberNameProblem(handler);
    if (problem) {
      throw new CommandError(problem);
    }
    if (namedNodes(doc).some((n) => n.name === handler) || handler === doc.window.name) {
      throw new CommandError(
        l10n.t('The handler name "{0}" is also the name of a member', handler),
      );
    }
    const other = handlerType(doc, handler, { path, event });
    if (other !== undefined && other !== h.type) {
      throw new CommandError(
        l10n.t(
          '"{0}" is already used for a handler of type {1}, but this handler is of type {2}',
          handler,
          other,
          h.type,
        ),
      );
    }
    events[event] = handler;
  }
  if (Object.keys(events).length > 0) {
    node['events'] = events;
  } else {
    delete node['events'];
  }
}

/** 名前の参照(メニュー・ツールバーの action、部品の actions、参照のプロパティ)を書き換える。 */
function renameReferences(doc: Json, from: string, to: string): void {
  const catalog = getCatalog();
  const visit = (node: unknown, path: JsonPath): void => {
    if (Array.isArray(node)) {
      node.forEach((v, i) => {
        visit(v, [...path, i]);
      });
      return;
    }
    if (!isObject(node)) {
      return;
    }
    if (node['action'] === from) {
      node['action'] = to;
    }
    // 部品の actions(名前の並び。ドキュメントの actions はノードの並びなので除く)
    const actions = node['actions'];
    if (path.length > 0 && Array.isArray(actions)) {
      node['actions'] = (actions as unknown[]).map((a) => (a === from ? to : a));
    }
    const props = node['properties'];
    if (isObject(props)) {
      let cls: string | undefined;
      try {
        cls = classOf(path, node);
      } catch {
        cls = undefined;
      }
      const info = cls === undefined ? undefined : catalog.classes[cls];
      for (const [name, value] of Object.entries(props)) {
        const type = info?.properties[name]?.type;
        if (value === from && typeof type === 'object' && 'ref' in type) {
          props[name] = to;
        }
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

function rename(doc: TtuiDocument, path: JsonPath, name: string): void {
  const root = doc as unknown as Json;
  const node = nodeAt(root, path);
  const old = node['name'];
  if (old === name) {
    return;
  }
  if (path.length === 1 && path[0] === 'window') {
    const problem = isValidIdentifier(name);
    if (problem) {
      throw new CommandError(describeIdentifierProblem(problem));
    }
    node['name'] = name;
    return;
  }
  const problem = memberNameProblem(name);
  if (problem) {
    throw new CommandError(problem);
  }
  if (namedNodes(doc).some((n) => n.name === name) || name === doc.window.name) {
    throw new CommandError(l10n.t('The name "{0}" is already used', name));
  }
  node['name'] = name;
  if (typeof old === 'string') {
    renameReferences(root, old, name);
  }
}

function apply(doc: TtuiDocument, command: EditCommand): void {
  switch (command.kind) {
    case 'setProperties':
      setProperties(doc, command.target, command.properties);
      return;
    case 'setEvent':
      setEvent(doc, command.target, command.event, command.handler);
      return;
    case 'rename':
      rename(doc, command.target, command.name);
      return;
    case 'batch':
      for (const c of command.commands) {
        apply(doc, c);
      }
      return;
  }
}

/** コマンドを適用した新しいドキュメント(元のドキュメントは変えない)。 */
export function applyCommand(doc: TtuiDocument, command: EditCommand): CommandResult {
  const copy = JSON.parse(JSON.stringify(doc)) as TtuiDocument;
  try {
    apply(copy, command);
  } catch (e) {
    if (e instanceof CommandError) {
      return { ok: false, error: e.message };
    }
    throw e;
  }
  return { ok: true, document: copy };
}

/** ハンドラの既定の名前(`search_button` の `on_click` → `search_button_click`。ウィンドウは `window_close`)。 */
export function defaultHandlerName(nodeName: string | undefined, event: string): string {
  return `${nodeName ?? 'window'}_${event.replace(/^on_/, '')}`;
}

/** ダブルクリックで開くハンドラ(C++Builder と同じく、部品のいちばん主なもの)。無ければ undefined。 */
export function defaultEventOf(cls: string): string | undefined {
  const handlers = findClass(cls)?.handlers ?? {};
  return [
    'on_click',
    'on_trigger',
    'on_submit',
    'on_change',
    'on_activate',
    'on_timer',
    'on_finish',
    'on_close',
    'on_paint',
  ].find((e) => e in handlers);
}
