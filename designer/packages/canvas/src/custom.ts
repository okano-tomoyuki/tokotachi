/**
 * キャンバスの自作部品(docs/adr/0044)。利用者のコードは WebAssembly の Tokotachi に無いので、基底のカタログの部品として作り、
 * 自作のプロパティは設定しない。
 */
import type { Op } from '@tokotachi-designer/codegen';
import { findClass } from '@tokotachi-designer/core';

/** 自作部品のうち、デザイナーが仮の絵(枠とクラスの名前)を重ねるものの基底。 */
export const CUSTOM_PLACEHOLDER_BASES: readonly string[] = ['Widget', 'PaintBox'];

/** 自作部品を基底のカタログの部品に置き換える。基底に無いプロパティの設定は除く。 */
export function substituteCustomClasses(ops: readonly Op[]): Op[] {
  const classes = new Map<string, string>();
  const has = (target: string | undefined, property: string): boolean => {
    const cls = target === undefined ? undefined : classes.get(target);
    return cls === undefined || Object.hasOwn(findClass(cls)?.properties ?? {}, property);
  };
  return ops.flatMap((op): Op[] => {
    switch (op.op) {
      case 'new': {
        const base = findClass(op.class)?.custom?.base;
        if (base === undefined) {
          return [op];
        }
        classes.set(op.variable, base);
        return [{ ...op, class: base }];
      }
      case 'set':
      case 'bind':
        return has(op.target, op.property) ? [op] : [];
      case 'connect':
        return [];
      default:
        return [op];
    }
  });
}
