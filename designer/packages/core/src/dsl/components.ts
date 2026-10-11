/**
 * 部品ファイル(docs/adr/0043)。根が Widget の定義ファイルを、ほかの定義ファイルから `component` で参照して置く。
 * core は Webview でも動くため、ファイルを読むのは呼び出し側(拡張・CLI)。
 */
import { WINDOW_EXTENSION } from '../project/paths.ts';
import type { JsonPath } from './diagnostics.ts';
import type { TtuiDocument, WidgetNode } from './schema.ts';
import { namedNodes, widgets } from './tree.ts';

/** 部品ファイルか(根が Widget)。 */
export function isComponentDocument(doc: TtuiDocument): boolean {
  return doc.window.class === 'Widget';
}

export interface ComponentReference {
  /** 定義ファイルに書いたパス(定義ファイルのフォルダからの相対パス)。 */
  readonly path: string;
  /** 置いた部品の名前。 */
  readonly name: string;
  /** `component` の位置。 */
  readonly at: JsonPath;
}

/** ドキュメントが置いている部品ファイル(部品の順)。 */
export function componentReferences(doc: TtuiDocument): ComponentReference[] {
  return widgets(doc).flatMap((e) =>
    e.node.component === undefined
      ? []
      : [{ path: e.node.component, name: e.node.name, at: [...e.path, 'component'] }],
  );
}

/** 部品ファイルの名前から、置いた部品の名前の元(`parts/search_bar.ttui.json` → `search_bar`)。 */
function componentPrefix(path: string): string {
  const file = path.split('/').pop() ?? path;
  const base = (file.endsWith(WINDOW_EXTENSION) ? file.slice(0, -WINDOW_EXTENSION.length) : file)
    .replace(/[^A-Za-z0-9_]/g, '_')
    .toLowerCase();
  return /^[a-z]/.test(base) ? base : `component_${base}`;
}

/** パレットから置く部品ファイルの部品(名前は部品ファイルの名前に番号を付けたもの: search_bar_1)。 */
export function newComponentWidget(doc: TtuiDocument, path: string): WidgetNode {
  const prefix = componentPrefix(path);
  const used = new Set(namedNodes(doc).map((n) => n.name));
  for (let i = 1; ; i++) {
    const name = `${prefix}_${String(i)}`;
    if (!used.has(name)) {
      return { name, class: 'Widget', component: path };
    }
  }
}
