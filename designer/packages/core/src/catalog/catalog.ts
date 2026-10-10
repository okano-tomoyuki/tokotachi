// カタログ(docs/designer/catalog.md)を読む。catalog.json は tools/catalog/extract.mts が作る。
import raw from './catalog.json' with { type: 'json' };
import type { Catalog, ClassInfo, HandlerInfo, PropertyInfo } from './types.ts';

const CATALOG = raw as unknown as Catalog;

export function getCatalog(): Catalog {
  return CATALOG;
}

export function findClass(name: string): ClassInfo | undefined {
  return Object.hasOwn(CATALOG.classes, name) ? CATALOG.classes[name] : undefined;
}

/** クラス name が base そのものか、その派生か(カタログに無いクラスは false)。 */
export function isSubclassOf(name: string, base: string): boolean {
  if (name === base) {
    return true;
  }
  return findClass(name)?.bases.includes(base) ?? false;
}

export function findProperty(info: ClassInfo, name: string): PropertyInfo | undefined {
  return Object.hasOwn(info.properties, name) ? info.properties[name] : undefined;
}

export function findHandler(info: ClassInfo, name: string): HandlerInfo | undefined {
  return Object.hasOwn(info.handlers, name) ? info.handlers[name] : undefined;
}
