/** テスト用の見本(designer/samples。tools/codegen/verify-cpp.mts でビルド・実行して確かめているもの)。 */
import { TtuiDocument } from '@tokotachi-designer/core';
import mainWindow from '../../../samples/main_window.ttui.json' with { type: 'json' };
import settingsDialog from '../../../samples/settings_dialog.ttui.json' with { type: 'json' };

export const MAIN_WINDOW: TtuiDocument = TtuiDocument.parse(mainWindow);
export const SETTINGS_DIALOG: TtuiDocument = TtuiDocument.parse(settingsDialog);

/** 見本の写し(テストで書き換える)。 */
export function mainWindowCopy(): any {
  return JSON.parse(JSON.stringify(mainWindow));
}
