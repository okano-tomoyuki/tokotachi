/**
 * よく使うアイコン(Lucide の線画。docs/adr/0054)。画像のプロパティに `"icon:<名前>"` と書くと、画像のファイルの代わりに使える。
 * 一覧(icons.json)は tools/icons/generate.mjs がライブラリのアイコンと一緒に書く(並びは列挙型 tokotachi::Icon の順)。
 */
import raw from './icons.json' with { type: 'json' };

export interface IconInfo {
  /** 定義ファイルに書く名前(`folder_open`。列挙の名前を小文字にしたもの)。 */
  readonly name: string;
  /** Lucide の名前(`folder-open`)。 */
  readonly lucide: string;
  /** 分類(ファイル・編集 等)。 */
  readonly category: string;
  /** 24 × 24 の格子の線画(SVG の path の d。線の太さ 2、端と角は丸い)。 */
  readonly path: string;
  /** 列挙型 tokotachi::Icon の値(並びの位置)。 */
  readonly index: number;
}

/** 画像のプロパティの値で、アイコンを表す接頭辞。 */
export const ICON_PREFIX = 'icon:';

/** 取り込んだ Lucide の版。 */
export const LUCIDE_VERSION: string = raw.lucide;

export const ICONS: readonly IconInfo[] = raw.icons.map((icon, index) => ({ ...icon, index }));

const byName = new Map(ICONS.map((icon) => [icon.name, icon]));

/** 画像のプロパティの値がアイコン(`"icon:save"`)なら、そのアイコン(知らない名前なら undefined)。 */
export function iconOf(value: unknown): IconInfo | undefined {
  return typeof value === 'string' && value.startsWith(ICON_PREFIX)
    ? byName.get(value.slice(ICON_PREFIX.length))
    : undefined;
}

/** 画像のプロパティの値が、アイコンを書いたつもりのもの(`icon:` で始まる)か。 */
export function isIconValue(value: unknown): boolean {
  return typeof value === 'string' && value.startsWith(ICON_PREFIX);
}

/** アイコンの C++ の列挙の名前(`FOLDER_OPEN`)。 */
export function iconEnumName(icon: IconInfo): string {
  return icon.name.toUpperCase();
}
