/**
 * Variable の編集(docs/designer/editor-design.md §3.4)。Variable を足す・型と初期値を変える・結びつけを外す。
 * プロパティを結びつけるのは setProperties(値を `{ "bind": "名前" }` にする)。
 */
import { l10n } from '../l10n.ts';
import { bindingOf } from '../dsl/bindings.ts';
import type { JsonPath } from '../dsl/diagnostics.ts';
import type { PropertyValue, TtuiDocument, VariableType } from '../dsl/schema.ts';
import { VariableType as VariableTypeSchema } from '../dsl/schema.ts';
import { namedNodes } from '../dsl/tree.ts';
import { CommandError, valueAt } from './structure.ts';

type Json = Record<string, unknown>;

function isObject(value: unknown): value is Json {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

/** Variable を足す(名前は呼び出し側が決める。同じ batch の中で結びつけるため)。 */
export function addVariable(
  doc: TtuiDocument,
  name: string,
  type: VariableType,
  value?: PropertyValue,
): void {
  if (!VariableTypeSchema.options.includes(type)) {
    throw new CommandError(l10n.t('Unknown variable type "{0}"', type));
  }
  if (namedNodes(doc).some((n) => n.name === name) || name === doc.window.name) {
    throw new CommandError(l10n.t('The name "{0}" is already used', name));
  }
  const root = doc as unknown as Json;
  const list = Array.isArray(root['variables']) ? (root['variables'] as unknown[]) : [];
  root['variables'] = list;
  list.push(value === undefined ? { name, type } : { name, type, value });
}

/** Variable の型・初期値を変える(value が null なら初期値を消す)。 */
export function setVariable(
  doc: TtuiDocument,
  path: JsonPath,
  changes: { readonly type?: VariableType; readonly value?: PropertyValue },
): void {
  const node = valueAt(doc, path);
  if (path[0] !== 'variables' || !isObject(node)) {
    throw new CommandError(l10n.t('The target was not found: {0}', path.join('.')));
  }
  if (changes.type !== undefined) {
    if (!VariableTypeSchema.options.includes(changes.type)) {
      throw new CommandError(l10n.t('Unknown variable type "{0}"', changes.type));
    }
    if (node['type'] !== changes.type) {
      node['type'] = changes.type;
      // 型を変えたら、合わなくなる初期値は消す
      if (changes.value === undefined) {
        delete node['value'];
      }
    }
  }
  if (changes.value === null) {
    delete node['value'];
  } else if (changes.value !== undefined) {
    node['value'] = changes.value;
  }
}

/**
 * プロパティの結びつけをたどり、visit が返す値に置き換える。
 * visit は結びつけの Variable の名前を受け取る。
 */
function mapBindings(doc: TtuiDocument, visit: (name: string) => PropertyValue): void {
  const walk = (node: unknown): void => {
    if (Array.isArray(node)) {
      for (const v of node) {
        walk(v);
      }
      return;
    }
    if (!isObject(node)) {
      return;
    }
    const props = node['properties'];
    if (isObject(props)) {
      for (const [key, value] of Object.entries(props)) {
        const name = bindingOf(value as PropertyValue);
        if (name === undefined) {
          continue;
        }
        props[key] = visit(name);
      }
    }
    for (const [key, value] of Object.entries(node)) {
      if (key !== 'properties') {
        walk(value);
      }
    }
  };
  walk(doc.window);
  walk(doc.actions);
  walk(doc.objects);
}

/** Variable の名前を変えたら、結びつけも書き換える。 */
export function renameBindings(doc: TtuiDocument, from: string, to: string): void {
  mapBindings(doc, (name) => ({ bind: name === from ? to : name }));
}

/** Variable の初期値(省略したときは T() の値)。 */
export function variableValue(type: VariableType, value: PropertyValue | undefined): PropertyValue {
  if (value !== undefined) {
    return value;
  }
  return type === 'string' ? '' : type === 'bool' ? false : 0;
}

/**
 * Variable を消すときに、それに結びつけたプロパティを外し、Variable の初期値を書く
 * (結んだ時点でプロパティは Variable の値になるので、外しても見た目は変わらない)。
 */
export function removeBindings(doc: TtuiDocument, from: string, value: PropertyValue): void {
  mapBindings(doc, (name) => (name === from ? value : { bind: name }));
}
