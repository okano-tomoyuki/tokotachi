// カタログ(docs/designer/catalog.md)を読む。catalog.json は tools/catalog/extract.mts が作る。
import raw from './catalog.json' with { type: 'json' };
import type { Catalog, ClassInfo, HandlerInfo, PropertyInfo } from './types.ts';

const CATALOG = raw as unknown as Catalog;

export function getCatalog(): Catalog {
  return CATALOG;
}

/** 自作部品(docs/adr/0044)。開いている定義ファイルが属するプロジェクトのもの。 */
let custom: Readonly<Record<string, ClassInfo>> = {};

/** 自作部品を設定する(Webview: 開いている定義ファイルのプロジェクトのもの)。 */
export function setCustomClasses(classes: Readonly<Record<string, ClassInfo>>): void {
  custom = classes;
}

export function getCustomClasses(): Readonly<Record<string, ClassInfo>> {
  return custom;
}

/**
 * 自作部品を設定して fn を呼ぶ(拡張: いくつもの定義ファイルを扱うので、検証・コード生成・描画の間だけ設定する)。
 * fn は同期的に終わること。
 */
export function withCustomClasses<T>(classes: Readonly<Record<string, ClassInfo>>, fn: () => T): T {
  const previous = custom;
  custom = classes;
  try {
    return fn();
  } finally {
    custom = previous;
  }
}

export function findClass(name: string): ClassInfo | undefined {
  if (Object.hasOwn(CATALOG.classes, name)) {
    return CATALOG.classes[name];
  }
  return Object.hasOwn(custom, name) ? custom[name] : undefined;
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
