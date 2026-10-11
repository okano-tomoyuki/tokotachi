import { TtuiDocument } from '@tokotachi-designer/core';
import { describe, expect, it } from 'vitest';
import goldenDialogCpp from './__golden__/settings_dialog.cpp?raw';
import goldenDialogHpp from './__golden__/settings_dialog.hpp?raw';
import goldenCpp from './__golden__/main_window.cpp?raw';
import goldenHpp from './__golden__/main_window.hpp?raw';
import goldenComponentCpp from './__golden__/labeled_edit.cpp?raw';
import goldenComponentHpp from './__golden__/labeled_edit.hpp?raw';
import { cppString } from './cpp/values.ts';
import { DEFAULT_CPP_OPTIONS, generateCpp, type CppOptions, type GenerateResult } from './index.ts';
import {
  LABELED_EDIT,
  MAIN_WINDOW,
  mainWindowCopy,
  readSampleComponent,
  readSampleImage,
  SETTINGS_DIALOG,
} from './testing.ts';

const lf = (text: string): string => text.replace(/\r\n/g, '\n');

/** 見本の画像を読んで生成する。 */
function gen(
  doc: TtuiDocument,
  dslFileName: string,
  readExisting: (path: string) => string | undefined,
  options: CppOptions = DEFAULT_CPP_OPTIONS,
): GenerateResult {
  return generateCpp(doc, dslFileName, readExisting, options, readSampleImage, (path) => {
    const document = readSampleComponent(path);
    return document ? { document, options } : undefined;
  });
}

function text(result: GenerateResult, path: string): string {
  const f = result.files.find((x) => x.path === path);
  if (!f?.result.ok) {
    throw new Error(`${path}: ${f?.result.ok === false ? f.result.error : 'not generated'}`);
  }
  return f.result.text;
}

/** 見本を生成したファイル(無ければ golden)を既存の内容として渡す。 */
function existing(files: Record<string, string>): (path: string) => string | undefined {
  return (path) => files[path];
}

describe('generateCpp', () => {
  it('見本から期待するコードを生成する(tools/codegen/golden.mts で作り直す)', () => {
    const r = gen(MAIN_WINDOW, 'main_window.ttui.json', () => undefined);
    expect(text(r, 'main_window.hpp')).toBe(lf(goldenHpp));
    expect(text(r, 'main_window.cpp')).toBe(lf(goldenCpp));
    const d = gen(SETTINGS_DIALOG, 'settings_dialog.ttui.json', () => undefined);
    expect(text(d, 'settings_dialog.hpp')).toBe(lf(goldenDialogHpp));
    expect(text(d, 'settings_dialog.cpp')).toBe(lf(goldenDialogCpp));
    const c = gen(LABELED_EDIT, 'labeled_edit.ttui.json', () => undefined);
    expect(text(c, 'labeled_edit.hpp')).toBe(lf(goldenComponentHpp));
    expect(text(c, 'labeled_edit.cpp')).toBe(lf(goldenComponentCpp));
  });

  it('部品ファイルのヘッダは、置いた定義ファイルのヘッダからのパスで include する', () => {
    const options = { ...DEFAULT_CPP_OPTIONS, headerDir: 'include', sourceDir: 'src' };
    const d = gen(SETTINGS_DIALOG, 'settings_dialog.ttui.json', () => undefined, options);
    // 部品ファイルのヘッダは parts/include/labeled_edit.hpp
    expect(text(d, 'include/settings_dialog.hpp')).toContain(
      '#include "../parts/include/labeled_edit.hpp"',
    );
    expect(text(d, 'include/settings_dialog.hpp')).toContain('    LabeledEdit* panel_user;');
    expect(text(d, 'src/settings_dialog.cpp')).toContain(
      '    panel_user = new LabeledEdit(this);\n    panel_user->tooltip = ',
    );
    const missing = generateCpp(SETTINGS_DIALOG, 'settings_dialog.ttui.json', () => undefined);
    expect(missing.files[0]?.result).toMatchObject({ ok: false });
  });

  it('生成し直しても変わらない', () => {
    const files = { 'main_window.hpp': lf(goldenHpp), 'main_window.cpp': lf(goldenCpp) };
    const r = gen(MAIN_WINDOW, 'main_window.ttui.json', existing(files));
    for (const f of r.files) {
      expect(f.result).toMatchObject({ ok: true, modifiedRegions: [], addedStubs: [] });
    }
    expect(text(r, 'main_window.cpp')).toBe(files['main_window.cpp']);
    expect(r.warnings).toEqual([]);
  });

  it('区間の外の利用者のコードを残し、新しいハンドラの雛形を足す', () => {
    const cpp = lf(goldenCpp)
      .replace(
        '#include "main_window.hpp"\n// <tokotachi-designer:end id="tokotachi_source_begin" hash="27964efa">\n',
        '#include "main_window.hpp"\n// <tokotachi-designer:end id="tokotachi_source_begin" hash="27964efa">\n#include <cstdio>\n',
      )
      .replace(
        'void MainWindow::search(tokotachi::Object* sender)\n{\n}',
        'void MainWindow::search(tokotachi::Object* sender)\n{\n    std::puts("search");\n}',
      );
    const doc = mainWindowCopy();
    doc.window.layout.items[0].widget.layout.items[1].widget.properties.text = '探す';
    doc.actions[1].events = { on_trigger: 'clear_recent' };
    const r = gen(TtuiDocument.parse(doc), 'main_window.ttui.json', (path) =>
      path === 'main_window.cpp' ? cpp : lf(goldenHpp),
    );
    const out = text(r, 'main_window.cpp');
    expect(out).toContain('#include <cstdio>\n');
    expect(out).toContain('    std::puts("search");\n');
    expect(out).toContain('button_search->text = "探す";');
    expect(out).toContain(
      '// <tokotachi-designer:handler-stubs>\n\nvoid MainWindow::clear_recent(tokotachi::Object* sender)\n{\n}\n',
    );
    expect(r.files[1]?.result).toMatchObject({ addedStubs: ['clear_recent'] });
    expect(text(r, 'main_window.hpp')).toContain(
      '    void clear_recent(tokotachi::Object* sender);',
    );
  });

  it('区間の中を手で変えたことに気づく(整形の違いは除く)', () => {
    const reformatted = lf(goldenCpp).replace(
      '    action_open = new tokotachi::Action(this);',
      '    action_open =\n        new tokotachi::Action(this);',
    );
    const edited = lf(goldenCpp).replace(
      '    action_open->text',
      '    // 手で足した行\n    action_open->text',
    );
    const read = (cpp: string) => (path: string) =>
      path === 'main_window.cpp' ? cpp : lf(goldenHpp);
    const r1 = gen(MAIN_WINDOW, 'main_window.ttui.json', read(reformatted));
    expect(r1.files[1]?.result).toMatchObject({ ok: true, modifiedRegions: [] });
    const r2 = gen(MAIN_WINDOW, 'main_window.ttui.json', read(edited));
    expect(r2.files[1]?.result).toMatchObject({
      ok: true,
      modifiedRegions: ['tokotachi_setup_ui'],
    });
  });

  it('マーカーが壊れていれば何も書かない', () => {
    const broken = lf(goldenCpp).replace(
      /\/\/ <tokotachi-designer:end id="tokotachi_setup_ui" hash="[0-9a-f]+">\n/,
      '',
    );
    const r = gen(MAIN_WINDOW, 'main_window.ttui.json', (path) =>
      path === 'main_window.cpp' ? broken : undefined,
    );
    expect(r.files[1]?.result.ok).toBe(false);
  });

  it('定義ファイルから消えた名前が区間の外で使われていれば警告する', () => {
    const cpp = lf(goldenCpp).replace(
      'void MainWindow::search(tokotachi::Object* sender)\n{\n}',
      'void MainWindow::search(tokotachi::Object* sender)\n{\n    line_edit_search->text = "";\n}',
    );
    const doc = mainWindowCopy();
    doc.window.layout.items[0].widget.layout.items[0].widget.name = 'query_edit';
    const r = gen(TtuiDocument.parse(doc), 'main_window.ttui.json', (path) =>
      path === 'main_window.cpp' ? cpp : lf(goldenHpp),
    );
    expect(r.warnings).toHaveLength(1);
    expect(r.warnings[0]).toContain('line_edit_search');
  });

  it('名前空間・フォルダ・インクルードガードの接頭辞・コメントの言語', () => {
    const r = gen(MAIN_WINDOW, 'ui/main_window.ttui.json', () => undefined, {
      ...DEFAULT_CPP_OPTIONS,
      namespace: ['app', 'ui'],
      includeGuardPrefix: 'MYAPP_',
      headerDir: 'include',
      sourceDir: 'src',
      commentLocale: 'ja',
    });
    expect(r.files.map((f) => f.path)).toEqual(['include/main_window.hpp', 'src/main_window.cpp']);
    const hpp = text(r, 'include/main_window.hpp');
    expect(hpp).toContain('#ifndef MYAPP_MAIN_WINDOW_HPP\n');
    expect(hpp).toContain('namespace app\n{\nnamespace ui\n{\n');
    expect(hpp).toContain('}  // namespace ui\n}  // namespace app\n');
    expect(hpp).toContain('/// デザイナーで作ったウィンドウ(main_window.ttui.json)。');
    expect(text(r, 'src/main_window.cpp')).toContain('#include "../include/main_window.hpp"\n');
  });

  it('画像の中身を配列として埋め込み、load_from_data で読む', () => {
    const cpp = text(
      gen(MAIN_WINDOW, 'main_window.ttui.json', () => undefined),
      'main_window.cpp',
    );
    expect(cpp).toContain(
      '// icons/open.png\nconst unsigned char tokotachi_image_icons_open_png[] = {\n    0x89, 0x50, 0x4E, 0x47,',
    );
    expect(cpp).toContain(
      '    action_open->icon = tokotachi::Image::load_from_data(tokotachi_image_icons_open_png, sizeof(tokotachi_image_icons_open_png));',
    );
    // 同じ画像は 1 つだけ埋め込む
    const doc = mainWindowCopy();
    doc.actions[3].properties.icon = './icons/../icons/open.png';
    const twice = text(
      gen(TtuiDocument.parse(doc), 'main_window.ttui.json', () => undefined),
      'main_window.cpp',
    );
    expect(twice.match(/const unsigned char/g)).toHaveLength(1);
    expect(twice).toContain(
      '    action_quit->icon = tokotachi::Image::load_from_data(tokotachi_image_icons_open_png,',
    );
  });

  it('読めない画像があれば何も生成しない', () => {
    const doc = mainWindowCopy();
    doc.actions[0].properties.icon = 'icons/missing.png';
    const r = gen(TtuiDocument.parse(doc), 'main_window.ttui.json', () => undefined);
    expect(r.files.map((f) => f.result.ok)).toEqual([false, false]);
    expect(r.files[1]?.result).toMatchObject({
      error: expect.stringContaining('icons/missing.png'),
    });
  });

  it('Dialog はコンストラクタで親のウィンドウを受け取り、ボタンを加える', () => {
    const r = gen(SETTINGS_DIALOG, 'settings_dialog.ttui.json', () => undefined);
    expect(text(r, 'settings_dialog.hpp')).toContain(
      'class SettingsDialog : public tokotachi::Dialog\n{\npublic:\n    explicit SettingsDialog(tokotachi::Window* parent);',
    );
    const cpp = text(r, 'settings_dialog.cpp');
    expect(cpp).toContain('    : tokotachi::Dialog(parent, std::string())');
    expect(cpp).toContain('    add_button(tokotachi::DialogButton::CANCEL, "やめる");');
  });
});

describe('cppString', () => {
  it('UTF-8 のまま、引用符・逆斜線・制御文字をエスケープする', () => {
    expect(cppString('日本語')).toBe('"日本語"');
    expect(cppString('a"b\\c')).toBe('"a\\"b\\\\c"');
    expect(cppString('1\n2\t3\u00014')).toBe('"1\\n2\\t3\\0014"');
  });
});
