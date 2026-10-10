/** テスト用の見本(designer/samples。コード生成の確認でビルド・実行しているもの)。 */
import raw from '../../../samples/main_window.ttui.json' with { type: 'json' };
import { TtuiDocument } from './dsl/schema.ts';

export const SAMPLE: TtuiDocument = TtuiDocument.parse(raw);

/** 見本の写し(テストで自由に書き換えて、異常系を作る)。 */
export function sampleCopy(): any {
  return JSON.parse(JSON.stringify(raw));
}
