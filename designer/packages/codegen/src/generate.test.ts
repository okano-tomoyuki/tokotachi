import { readCustomClasses, TtuiDocument, withCustomClasses } from '@tokotachi-designer/core';
import { describe, expect, it } from 'vitest';
import goldenDialogCpp from './__golden__/settings_dialog.cpp?raw';
import goldenDialogHpp from './__golden__/settings_dialog.hpp?raw';
import goldenCpp from './__golden__/main_window.cpp?raw';
import goldenHpp from './__golden__/main_window.hpp?raw';
import goldenImagesCpp from './__golden__/main_window_images.cpp?raw';
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
  SAMPLE_OPTIONS,
  SETTINGS_DIALOG,
} from './testing.ts';

const lf = (text: string): string => text.replace(/\r\n/g, '\n');

/** 見本の画像を読んで生成する。 */
function gen(
  doc: TtuiDocument,
  dslFileName: string,
  readExisting: (path: string) => string | undefined,
  options: CppOptions = SAMPLE_OPTIONS,
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
    expect(text(r, 'main_window_images.cpp')).toBe(lf(goldenImagesCpp));
    const d = gen(SETTINGS_DIALOG, 'settings_dialog.ttui.json', () => undefined);
    expect(text(d, 'settings_dialog.hpp')).toBe(lf(goldenDialogHpp));
    expect(text(d, 'settings_dialog.cpp')).toBe(lf(goldenDialogCpp));
    const c = gen(LABELED_EDIT, 'labeled_edit.ttui.json', () => undefined);
    expect(text(c, 'labeled_edit.hpp')).toBe(lf(goldenComponentHpp));
    expect(text(c, 'labeled_edit.cpp')).toBe(lf(goldenComponentCpp));
  });

  it('部品ファイルのヘッダは、置いた定義ファイルのヘッダからのパスで include する', () => {
    const options = { ...SAMPLE_OPTIONS, headerDir: 'include', sourceDir: 'src' };
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

  it('自作部品は名前空間を付けたクラスで作り、プロジェクトのフォルダからたどってヘッダを include する', () => {
    const { classes } = readCustomClasses([
      {
        path: 'src/widgets/gauge.hpp',
        text: 'namespace app { class Gauge : public tokotachi::PaintBox { public: tokotachi::Property<double> level; }; }',
      },
    ]);
    const doc = TtuiDocument.parse({
      format_version: 1,
      window: {
        name: 'MainWindow',
        class: 'Window',
        layout: {
          class: 'BoxLayout',
          items: [{ widget: { name: 'gauge_1', class: 'Gauge', properties: { level: 40 } } }],
        },
      },
    });
    const options = { ...DEFAULT_CPP_OPTIONS, headerDir: 'include', projectDir: '..' };
    const r = withCustomClasses(classes, () =>
      generateCpp(doc, 'main_window.ttui.json', () => undefined, options),
    );
    const header = text(r, 'include/main_window.hpp');
    expect(header).toContain('#include "../../src/widgets/gauge.hpp"');
    expect(header).toContain('    app::Gauge* gauge_1;');
    expect(text(r, 'main_window.cpp')).toContain(
      '    gauge_1 = new app::Gauge(this);\n    gauge_1->level = 40;',
    );
    // プロジェクトに属さなければ使えない
    const loose = withCustomClasses(classes, () =>
      generateCpp(doc, 'main_window.ttui.json', () => undefined),
    );
    expect(loose.files[0]?.result).toMatchObject({ ok: false });
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
    expect(r.files.map((f) => f.path)).toEqual([
      'include/main_window.hpp',
      'src/main_window.cpp',
      'src/main_window_images.cpp',
    ]);
    const hpp = text(r, 'include/main_window.hpp');
    expect(hpp).toContain('#ifndef MYAPP_MAIN_WINDOW_HPP\n');
    expect(hpp).toContain('namespace app\n{\nnamespace ui\n{\n');
    expect(hpp).toContain('}  // namespace ui\n}  // namespace app\n');
    expect(hpp).toContain('/// デザイナーで作ったウィンドウ(main_window.ttui.json)。');
    expect(text(r, 'src/main_window.cpp')).toContain('#include "../include/main_window.hpp"\n');
    // 画像のソースも同じ名前空間に書く
    expect(text(r, 'src/main_window_images.cpp')).toContain(
      'namespace app\n{\nnamespace ui\n{\n\nnamespace tokotachi_images_MainWindow\n',
    );
  });

  it('画像の中身を別のソースに配列として埋め込み、ウィンドウのソースは宣言して load_from_data で読む', () => {
    const r = gen(MAIN_WINDOW, 'main_window.ttui.json', () => undefined);
    expect(r.files.map((f) => f.path)).toEqual([
      'main_window.hpp',
      'main_window.cpp',
      'main_window_images.cpp',
    ]);
    // プロジェクトなら CMakeLists.txt のソースの一覧に入る。プロジェクトでなければ、ビルドに加えるよう知らせる
    expect(r.warnings).toEqual([]);
    const standalone = gen(MAIN_WINDOW, 'main_window.ttui.json', () => undefined, {
      ...SAMPLE_OPTIONS,
      projectDir: undefined,
    });
    expect(standalone.warnings).toEqual([
      'main_window_images.cpp was created. Add it to the sources of your build.',
    ]);
    const images = text(r, 'main_window_images.cpp');
    expect(images).toContain(
      '// icons/open.png\nextern const unsigned char tokotachi_image_icons_open_png[];\n' +
        'extern const std::size_t tokotachi_image_icons_open_png_size;\n' +
        'const unsigned char tokotachi_image_icons_open_png[] = {\n    0x89, 0x50, 0x4E, 0x47,',
    );
    expect(images).toContain(
      'const std::size_t tokotachi_image_icons_open_png_size = sizeof(tokotachi_image_icons_open_png);',
    );
    const cpp = text(r, 'main_window.cpp');
    expect(cpp).not.toContain('0x89');
    expect(cpp).toContain(
      'namespace tokotachi_images_MainWindow\n{\nextern const unsigned char tokotachi_image_icons_open_png[];\n',
    );
    expect(cpp).toContain(
      '    action_open->icon = tokotachi::Image::load_from_data(tokotachi_images_MainWindow::tokotachi_image_icons_open_png, tokotachi_images_MainWindow::tokotachi_image_icons_open_png_size);',
    );
    // 同じ画像は 1 つだけ埋め込む
    const doc = mainWindowCopy();
    doc.actions[3].properties.icon = './icons/../icons/open.png';
    const twice = gen(TtuiDocument.parse(doc), 'main_window.ttui.json', () => undefined);
    expect(
      text(twice, 'main_window_images.cpp').match(/const unsigned char \w+\[\] = /g),
    ).toHaveLength(1);
    expect(text(twice, 'main_window.cpp')).toContain(
      '    action_quit->icon = tokotachi::Image::load_from_data(tokotachi_images_MainWindow::tokotachi_image_icons_open_png,',
    );
  });

  it('大きい画像(数十万行の区間)も、生成し直して既存のファイルの区間を置き換えられる', () => {
    const big = new Uint8Array(4 * 1024 * 1024).map((_, i) => i & 0xff);
    const generate = (existing: (path: string) => string | undefined) =>
      generateCpp(MAIN_WINDOW, 'main_window.ttui.json', existing, SAMPLE_OPTIONS, () => big);
    const first = generate(() => undefined);
    const images = text(first, 'main_window_images.cpp');
    expect(images.split('\n').length).toBeGreaterThan(250_000);
    // 2 回目は既存のファイルの区間を置き換える(引数に展開すると、スタックがあふれていた)
    const second = generate((path) => (path === 'main_window_images.cpp' ? images : undefined));
    expect(text(second, 'main_window_images.cpp')).toBe(images);
  }, 60_000);

  it('画像を参照しなくなったら、画像のソースは書かず、使われていないことを知らせる', () => {
    const doc = mainWindowCopy();
    delete doc.actions[0].properties.icon;
    const r = gen(TtuiDocument.parse(doc), 'main_window.ttui.json', (path) =>
      path === 'main_window_images.cpp' ? '// old\n' : undefined,
    );
    expect(r.files.map((f) => f.path)).toEqual(['main_window.hpp', 'main_window.cpp']);
    expect(text(r, 'main_window.cpp')).not.toContain('tokotachi_images_');
    expect(r.warnings.join('\n')).toContain('main_window_images.cpp is no longer used');
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
