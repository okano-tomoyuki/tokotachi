// カタログの型(docs/designer/catalog.md §4)。catalog.json は tools/catalog/extract.mts が作る。
// キーは定義ファイルと同じく snake_case(Tokotachi の API に合わせる)。

/** 値そのものを書く型。 */
export type PrimitiveType = 'bool' | 'int' | 'float' | 'double' | 'string';

/** プロパティの型。 */
export type PropertyType =
  | PrimitiveType
  /** 列挙型(値の名前で書く)。 */
  | { enum: string }
  /** 値の型(Margins・Size・Font 等。`values` にある)。 */
  | { value: string }
  /** 配列(std::vector)。 */
  | { array: PrimitiveType }
  /** ほかのオブジェクトへのポインタ(同じファイルのノードの名前で書く)。 */
  | { ref: string };

/** クラスの種類。定義ファイルのどこに書けるかが決まる(docs/designer/dsl-spec.md §4)。 */
export type ClassKind =
  /** ルートのウィンドウ(Window・Dialog)。 */
  | 'window'
  /** 部品(レイアウトの項目・子に置ける)。 */
  | 'widget'
  /** レイアウト(BoxLayout 等)。 */
  | 'layout'
  /** 専用のキーで書くもの(Action・MenuBar・Tab・SplitterPane 等)。 */
  | 'part'
  /** 画面に出ないオブジェクト(Timer 等。`objects` に書く)。 */
  | 'object';

/** パレットの分類。 */
export type Category = 'input' | 'display' | 'view' | 'container' | 'drawing' | 'non_visual';

/** インスペクタの編集の種類(型から決まらないもの)。 */
export type EditorKind = 'multiline' | 'mnemonic';

export interface PropertyInfo {
  type: PropertyType;
  /** 宣言したクラス。 */
  declared_in: string;
  /** ヘッダの `///` の説明。 */
  doc?: string;
  /** 位置と大きさ。ウィンドウと自由な配置の子だけに書ける。 */
  geometry?: true;
  /** 子(タブ・区画)を入れた後に設定する。 */
  after_children?: true;
  editor?: EditorKind;
  /** 既定値(実物で測った値。まだ測っていなければ無い)。 */
  default?: unknown;
}

export interface HandlerInfo {
  /** ハンドラの型(`handler_types` にある)。 */
  type: string;
  declared_in: string;
  doc?: string;
}

export interface ClassInfo {
  /** 基底クラス(近い順)。 */
  bases: string[];
  kind: ClassKind;
  /** パレットの分類(パレットに出すものだけ)。 */
  category?: Category;
  /** `layout`・`children` を書けるか。 */
  accepts_children?: true;
  /** 専用の子のキー(`tabs`・`panes`・`content`)。 */
  child_keys?: string[];
  doc?: string;
  /** デザイン時に設定できるプロパティ(継承したものを含む。宣言の順)。 */
  properties: Record<string, PropertyInfo>;
  /** ハンドラ(継承したものを含む)。 */
  handlers: Record<string, HandlerInfo>;
}

export interface ValueTypeInfo {
  /** メンバとその型(専用の書き方の型には無い)。 */
  members?: Record<string, PrimitiveType | { enum: string }>;
  /** メンバではなく専用の書き方で表す(ThemeColor・Shortcut)。 */
  special?: true;
  doc?: string;
}

export interface EnumInfo {
  values: string[];
  /** 値ごとの説明。 */
  docs?: Record<string, string>;
  doc?: string;
}

export interface HandlerParam {
  name: string;
  /** C++ の型(`Object*`・`bool&` 等。名前空間は付けない)。 */
  type: string;
}

export interface Catalog {
  format_version: 1;
  /** カタログを取り出した Tokotachi の版。 */
  tokotachi_version: string;
  /** パレットの順。 */
  classes: Record<string, ClassInfo>;
  values: Record<string, ValueTypeInfo>;
  enums: Record<string, EnumInfo>;
  handler_types: Record<string, HandlerParam[]>;
}
