/**
 * 画像のプロパティ(Action の icon 等。docs/designer/dsl-spec.md §5.1)。
 * 定義ファイルには、定義ファイルのあるフォルダからの相対パスを書く。生成するコードには画像の中身を埋め込み、
 * キャンバスには拡張が読んだ中身を渡す(実行時のパスに依らない)。
 * core は Webview でも動くため、ファイルを読むのは呼び出し側(拡張・CLI)。
 */
import { findClass } from '../catalog/catalog.ts';
import type { PropertyType } from '../catalog/types.ts';
import { normalizePath } from '../project/paths.ts';
import type { JsonPath } from './diagnostics.ts';
import type { Properties, TtuiDocument } from './schema.ts';
import { layouts, widgets } from './tree.ts';

/** 読める画像の拡張子(Tokotachi の Image::load が読めるもの)。 */
export const IMAGE_EXTENSIONS: readonly string[] = ['png', 'jpg', 'jpeg', 'bmp', 'gif'];

export function isImageType(type: PropertyType): boolean {
  return typeof type === 'object' && 'value' in type && type.value === 'Image';
}

/** 定義ファイルに書ける画像のパスか(相対パス・区切りは `/`・画像の拡張子で終わる)。 */
export function isImagePath(path: string): boolean {
  const extension = /\.([A-Za-z0-9]+)$/.exec(path)?.[1]?.toLowerCase();
  return (
    extension !== undefined &&
    IMAGE_EXTENSIONS.includes(extension) &&
    !path.startsWith('/') &&
    !path.includes('\\') &&
    !/^[A-Za-z]:/.test(path) &&
    // 途中にドライブ(`../../d:/x.png`。ほかのドライブのファイルは相対パスにできない)は不可
    !path.split('/').some((part) => /^[A-Za-z]:$/.test(part)) &&
    // ファイルの名前が拡張子だけ(`.png`)は不可
    !/^\.[A-Za-z0-9]+$/.test(normalizePath(path).split('/').pop() ?? '')
  );
}

export interface ImageReference {
  /** 定義ファイルに書いたパス。 */
  readonly path: string;
  /** プロパティの位置。 */
  readonly at: JsonPath;
}

/**
 * ドキュメントの画像のプロパティを、生成するコードの順(Action・オブジェクト・ウィンドウ・バー・部品)に並べる。
 * 値が文字列でないもの(検証のエラー)は除く。
 */
export function imageReferences(doc: TtuiDocument): ImageReference[] {
  const out: ImageReference[] = [];
  const visit = (cls: string, props: Properties | undefined, path: JsonPath): void => {
    const info = findClass(cls);
    if (!info || !props) {
      return;
    }
    for (const [name, prop] of Object.entries(info.properties)) {
      const value = props[name];
      if (isImageType(prop.type) && typeof value === 'string') {
        out.push({ path: value, at: [...path, 'properties', name] });
      }
    }
  };
  doc.actions?.forEach((a, i) => {
    visit('Action', a.properties, ['actions', i]);
  });
  doc.objects?.forEach((o, i) => {
    visit(o.class, o.properties, ['objects', i]);
  });
  const w = doc.window;
  visit(w.class, w.properties, ['window']);
  const bars = [
    ['menu_bar', 'MenuBar', w.menu_bar],
    ['tool_bar', 'ToolBar', w.tool_bar],
    ['status_bar', 'StatusBar', w.status_bar],
  ] as const;
  for (const [key, cls, bar] of bars) {
    visit(cls, bar?.properties, ['window', key]);
  }
  for (const e of widgets(doc)) {
    visit(e.node.class, e.node.properties, e.path);
  }
  for (const l of layouts(doc)) {
    visit(l.node.class, l.node.properties, l.path);
  }
  return out;
}
