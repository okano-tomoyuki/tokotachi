/**
 * プロジェクトファイル(*.ttproj.json)のスキーマ(docs/designer/project-spec.md)。
 * 実装後はこのスキーマと validate.ts を正とし、project-spec.md と食い違う場合は project-spec.md を直す。
 */
import * as z from 'zod';

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

export const TtprojDocument = z.strictObject({
  $schema: z.string().optional(),
  format_version: z.literal(PROJECT_FORMAT_VERSION),
  codegen: ProjectCodegen.optional(),
  /** 起動したときに開くウィンドウ(windows のどれか。class が Window のもの)。 */
  main_window: z.string().optional(),
  /** プロジェクトに属するウィンドウの定義ファイル(プロジェクトファイルのフォルダからの相対パス)。 */
  windows: z.array(z.string()).optional(),
});
export type TtprojDocument = z.infer<typeof TtprojDocument>;
