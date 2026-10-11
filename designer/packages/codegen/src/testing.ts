/** テスト用の見本(designer/samples。tools/codegen/verify-cpp.mts でビルド・実行して確かめているもの)。 */
import { TtuiDocument } from '@tokotachi-designer/core';
import mainWindow from '../../../samples/main_window.ttui.json' with { type: 'json' };
import settingsDialog from '../../../samples/settings_dialog.ttui.json' with { type: 'json' };
import labeledEdit from '../../../samples/parts/labeled_edit.ttui.json' with { type: 'json' };
import openPng from '../../../samples/icons/open.png?inline';

export const MAIN_WINDOW: TtuiDocument = TtuiDocument.parse(mainWindow);
export const SETTINGS_DIALOG: TtuiDocument = TtuiDocument.parse(settingsDialog);
/** 部品ファイルの見本(settings_dialog が置いている)。 */
export const LABELED_EDIT: TtuiDocument = TtuiDocument.parse(labeledEdit);

/** 見本が置いている部品ファイルを読む(パスは samples/ からの相対パス。無ければ undefined)。 */
export function readSampleComponent(path: string): TtuiDocument | undefined {
  return path === 'parts/labeled_edit.ttui.json' ? LABELED_EDIT : undefined;
}

/** 見本の写し(テストで書き換える)。 */
export function mainWindowCopy(): any {
  return JSON.parse(JSON.stringify(mainWindow));
}

/** data URL(base64)の中身。 */
function dataUrlBytes(url: string): Uint8Array {
  const digits = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
  const base64 = url.slice(url.indexOf(',') + 1).replace(/=+$/, '');
  const out: number[] = [];
  let bits = 0;
  let count = 0;
  for (const c of base64) {
    bits = (bits << 6) | digits.indexOf(c);
    count += 6;
    if (count >= 8) {
      count -= 8;
      out.push((bits >> count) & 0xff);
    }
  }
  return new Uint8Array(out);
}

/** 見本が参照する画像(samples/ からの相対パス)。 */
const SAMPLE_IMAGES: Readonly<Record<string, Uint8Array>> = {
  'icons/open.png': dataUrlBytes(openPng),
};

/** 見本が参照する画像を読む(無ければ undefined)。 */
export function readSampleImage(path: string): Uint8Array | undefined {
  return SAMPLE_IMAGES[path];
}
