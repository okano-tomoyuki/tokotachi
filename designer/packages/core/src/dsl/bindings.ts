/**
 * Variable との結びつけ(docs/designer/dsl-spec.md §5.2)。プロパティに `{ "bind": "Variable の名前" }` と書くと、
 * 生成するコードでは値を設定する代わりに `tokotachi::bind(部品->プロパティ, 変数->value)` で結びつける
 * (結んだ時点で、プロパティは Variable の値になる)。
 */
import type { PropertyInfo, PropertyType } from '../catalog/types.ts';
import type { PropertyValue, VariableType } from './schema.ts';

/** 値が結びつけなら、その Variable の名前(`bind` だけを持つオブジェクト)。 */
export function bindingOf(value: PropertyValue | undefined): string | undefined {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return undefined;
  }
  const keys = Object.keys(value);
  const name = value['bind'];
  return keys.length === 1 && typeof name === 'string' ? name : undefined;
}

/** プロパティの型に結びつけられる Variable の型(結びつけられなければ undefined)。 */
export function variableTypeOf(type: PropertyType): VariableType | undefined {
  return typeof type === 'string' ? type : undefined;
}

/**
 * Variable に結びつけられるプロパティか。値そのものの型(bool・int・float・double・string)だけで、
 * 位置と大きさ(レイアウトが決める。デザイナーでつまみで変える)は除く。
 */
export function isBindable(info: PropertyInfo): boolean {
  return variableTypeOf(info.type) !== undefined && info.geometry !== true;
}
