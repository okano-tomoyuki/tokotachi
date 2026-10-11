/**
 * プロジェクトファイル(*.ttproj.json)のスキーマ(docs/designer/project-spec.md)。
 * 実装後はこのスキーマと validate.ts を正とし、project-spec.md と食い違う場合は project-spec.md を直す。
 */
import * as z from 'zod';
import { getCatalog } from '../catalog/catalog.ts';
import { COLOR_PATTERN } from '../dsl/values.ts';

export const PROJECT_FORMAT_VERSION = 1;

/** コード生成の設定。パスはプロジェクトファイルのフォルダからの相対パス(区切りは /)。 */
export const ProjectCodegen = z.strictObject({
  /** 生成するコードのコメントの言語(既定は en)。 */
  comment_locale: z.enum(['en', 'ja']).optional(),
  /** ウィンドウを保存したときにコードを生成するか(既定は true)。 */
  generate_on_save: z.boolean().optional(),
  /** 生成するクラスの名前空間(`app`・`app::ui`)。 */
  namespace: z.string().optional(),
  /** インクルードガードのマクロの名前の先頭に付ける文字。 */
  include_guard_prefix: z.string().optional(),
  /** ウィンドウのヘッダ・ソースの出力先のフォルダ(既定はウィンドウの定義ファイルと同じフォルダ)。 */
  header_dir: z.string().optional(),
  source_dir: z.string().optional(),
});
export type ProjectCodegen = z.infer<typeof ProjectCodegen>;

const HexColor = z.string().regex(COLOR_PATTERN);
const ColorRoles = (getCatalog().enums['ColorRole']?.values ?? ['WINDOW']) as [string, ...string[]];

/** テーマの上書き(部品の theme_override と同じ書き方。dsl-spec.md §5)。 */
export const ThemeOverrideSchema = z.strictObject({
  /** 役割 → 色(ライト・ダークで同じ)か { light, dark }。 */
  colors: z
    .partialRecord(
      z.enum(ColorRoles),
      z.union([HexColor, z.strictObject({ light: HexColor, dark: HexColor })]),
    )
    .optional(),
  radius: z.number().min(0).optional(),
  field_radius: z.number().min(0).optional(),
  field_padding: z.number().min(0).optional(),
});

/** アプリケーションのテーマ(docs/designer/project-spec.md §4.1。docs/adr/0023・0042)。 */
export const ProjectTheme = z.strictObject({
  /** ライト・ダークの選び方(既定は SYSTEM: OS の設定に従う)。 */
  color_scheme: z.enum(['SYSTEM', 'LIGHT', 'DARK']).optional(),
  /** 標準のテーマ(ライト・ダーク)に重ねる上書き(部品の theme_override と同じ書き方。dsl-spec.md §5)。 */
  override: ThemeOverrideSchema.optional(),
  /** 名前を付けて登録したテーマ(部品の theme_override から { "preset": "名前" } で参照する。名前は識別子)。 */
  presets: z.record(z.string(), ThemeOverrideSchema).optional(),
});
export type ProjectTheme = z.infer<typeof ProjectTheme>;

export const TtprojDocument = z.strictObject({
  $schema: z.string().optional(),
  format_version: z.literal(PROJECT_FORMAT_VERSION),
  codegen: ProjectCodegen.optional(),
  theme: ProjectTheme.optional(),
  /** 起動したときに開くウィンドウ(windows のどれか。class が Window のもの)。 */
  main_window: z.string().optional(),
  /** プロジェクトに属するウィンドウの定義ファイル(プロジェクトファイルのフォルダからの相対パス)。 */
  windows: z.array(z.string()).optional(),
});
export type TtprojDocument = z.infer<typeof TtprojDocument>;
