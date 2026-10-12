import { getCatalog, type TtprojDocument } from '@tokotachi-designer/core';
import { describe, expect, it } from 'vitest';
import { cppOutputPaths, generateCpp } from './index.ts';
import {
  generateProject,
  projectOutputPaths,
  requiredLibraryVersion,
  windowCppOptions,
} from './project.ts';
import { MAIN_WINDOW, readSampleComponent, SETTINGS_DIALOG } from './testing.ts';

const PROJECT: TtprojDocument = {
  format_version: 1,
  codegen: { comment_locale: 'ja', namespace: 'app', header_dir: 'include', source_dir: 'src' },
  main_window: 'ui/main_window.ttui.json',
  windows: ['ui/main_window.ttui.json', 'ui/dialogs/settings_dialog.ttui.json'],
};
const WINDOWS = [
  { path: 'ui/main_window.ttui.json', doc: MAIN_WINDOW },
  { path: 'ui/dialogs/settings_dialog.ttui.json', doc: SETTINGS_DIALOG },
];

function text(result: ReturnType<typeof generateProject>, path: string): string {
  if ('error' in result) {
    throw new Error(result.error);
  }
  const f = result.files.find((x) => x.path === path);
  if (!f?.result.ok) {
    throw new Error(path);
  }
  return f.result.text;
}

describe('project', () => {
  it('プロジェクトの設定から、ウィンドウの生成の設定を作る(フォルダはウィンドウから見たパス)', () => {
    const options = windowCppOptions(PROJECT, 'ui/dialogs/settings_dialog.ttui.json');
    expect(options).toMatchObject({
      namespace: ['app'],
      headerDir: '../../include',
      sourceDir: '../../src',
      commentLocale: 'ja',
    });
    const r = generateCpp(SETTINGS_DIALOG, 'settings_dialog.ttui.json', () => undefined, options);
    expect(r.files.map((f) => f.path)).toEqual([
      '../../include/settings_dialog.hpp',
      '../../src/settings_dialog.cpp',
    ]);
    expect(windowCppOptions({ format_version: 1 }, 'main_window.ttui.json')).toMatchObject({
      headerDir: undefined,
      namespace: [],
      commentLocale: 'en',
    });
  });

  it('生成先のパスを、定義ファイルの中身を読まずに求める(プロジェクトのビューの「未生成」の表示に使う)', () => {
    const options = windowCppOptions(PROJECT, 'ui/dialogs/settings_dialog.ttui.json');
    expect(cppOutputPaths('settings_dialog.ttui.json', options)).toEqual({
      header: '../../include/settings_dialog.hpp',
      source: '../../src/settings_dialog.cpp',
      images: '../../src/settings_dialog_images.cpp',
    });
    expect(projectOutputPaths(PROJECT)).toEqual(['main.cpp', 'CMakeLists.txt']);
    const themed = { ...PROJECT, theme: { presets: { danger: { radius: 0 } } } };
    expect(projectOutputPaths(themed)).toEqual([
      'main.cpp',
      'CMakeLists.txt',
      'include/tokotachi_themes.hpp',
    ]);
    // 生成したときに出力するファイルと一致する
    const generated = generateProject(themed, 'app.ttproj.json', WINDOWS, () => undefined);
    expect('error' in generated ? [] : generated.files.map((f) => f.path)).toEqual(
      projectOutputPaths(themed),
    );
  });

  it('起動部分と CMakeLists.txt を生成する', () => {
    const r = generateProject(PROJECT, 'my_app.ttproj.json', WINDOWS, () => undefined);
    const main = text(r, 'main.cpp');
    expect(main).toContain('#include "include/main_window.hpp"');
    expect(main).toContain('    app::MainWindow* window_main = new app::MainWindow();');
    expect(main).toContain('    app.main_window = window_main;');
    const cmake = text(r, 'CMakeLists.txt');
    expect(cmake).toContain('project(my_app LANGUAGES CXX)');
    expect(cmake).toContain(
      '    main.cpp\n    src/main_window.cpp\n    src/main_window_images.cpp\n    src/settings_dialog.cpp\n)',
    );
    expect(cmake).toContain('set(TOKOTACHI_DESIGNER_INCLUDE_DIRS\n    include\n)');
    expect(cmake).toContain('add_executable(my_app ${TOKOTACHI_DESIGNER_SOURCES})');
  });

  it('find_package で、カタログを取り出したライブラリの版(major.minor)を求める', () => {
    const version = getCatalog().tokotachi_version;
    expect(version).toMatch(/^\d+\.\d+\.\d+$/);
    expect(version.startsWith(`${requiredLibraryVersion()}.`)).toBe(true);
    const cmake = text(
      generateProject(PROJECT, 'my_app.ttproj.json', WINDOWS, () => undefined),
      'CMakeLists.txt',
    );
    expect(cmake).toContain(`find_package(tokotachi ${requiredLibraryVersion()} REQUIRED)`);
  });

  it('CMakeLists.txt の区間の外の、利用者が書いたものは残す', () => {
    const first = text(
      generateProject(PROJECT, 'my_app.ttproj.json', WINDOWS, () => undefined),
      'CMakeLists.txt',
    );
    const edited = `${first}target_compile_definitions(my_app PRIVATE APP_DEBUG)\n`;
    const only = { ...PROJECT, windows: ['ui/main_window.ttui.json'] };
    const again = text(
      generateProject(only, 'my_app.ttproj.json', WINDOWS.slice(0, 1), (p) =>
        p === 'CMakeLists.txt' ? edited : undefined,
      ),
      'CMakeLists.txt',
    );
    expect(again).toContain('target_compile_definitions(my_app PRIVATE APP_DEBUG)');
    expect(again).not.toContain('settings_dialog.cpp');
  });

  it('メインのウィンドウが無い・Dialog なら生成しない', () => {
    expect(
      generateProject(
        { ...PROJECT, main_window: undefined },
        'a.ttproj.json',
        WINDOWS,
        () => undefined,
      ),
    ).toHaveProperty('error');
    expect(
      generateProject(
        { ...PROJECT, main_window: 'ui/dialogs/settings_dialog.ttui.json' },
        'a.ttproj.json',
        WINDOWS,
        () => undefined,
      ),
    ).toHaveProperty('error');
  });

  it('テーマを Application に設定する(配色と、標準のライト・ダークに重ねる上書き)', () => {
    const main = text(
      generateProject(
        {
          ...PROJECT,
          theme: { color_scheme: 'DARK', override: { colors: { ACCENT: '#DC2626' }, radius: 0 } },
        },
        'my_app.ttproj.json',
        WINDOWS,
        () => undefined,
      ),
      'main.cpp',
    );
    expect(main).toContain('    app.color_scheme = tokotachi::ColorScheme::DARK;');
    expect(main).toContain(
      '    const tokotachi::ThemeOverride theme_override = tokotachi::ThemeOverride().set(tokotachi::ColorRole::ACCENT, tokotachi::Color(0xDC, 0x26, 0x26)).set_radius(0);',
    );
    expect(main).toContain(
      '    theme_override.apply_to(dark_theme);\n    app.dark_theme = dark_theme;',
    );
    // テーマが無ければ何も書かない
    const plain = text(
      generateProject(PROJECT, 'my_app.ttproj.json', WINDOWS, () => undefined),
      'main.cpp',
    );
    expect(plain).not.toContain('color_scheme');
    expect(plain).not.toContain('ThemeOverride');
  });

  it('登録したテーマを共通のヘッダの関数にし、参照するウィンドウはそれを include して呼ぶ', () => {
    const project: TtprojDocument = {
      ...PROJECT,
      theme: { presets: { danger: { colors: { ACCENT: '#DC2626' } } } },
    };
    const header = text(
      generateProject(project, 'my_app.ttproj.json', WINDOWS, () => undefined),
      'include/tokotachi_themes.hpp',
    );
    expect(header).toContain('inline tokotachi::ThemeOverride danger_theme()');
    expect(header).toContain('namespace app');
    // 参照するウィンドウ
    const doc = JSON.parse(JSON.stringify(SETTINGS_DIALOG)) as typeof SETTINGS_DIALOG;
    (doc.window as { properties?: Record<string, unknown> }).properties = {
      ...doc.window.properties,
      theme_override: { preset: 'danger', radius: 0 },
    };
    const path = 'ui/dialogs/settings_dialog.ttui.json';
    const r = generateCpp(
      doc,
      'settings_dialog.ttui.json',
      () => undefined,
      windowCppOptions(project, path),
      undefined,
      (p) => {
        const component = readSampleComponent(p);
        return component
          ? { document: component, options: windowCppOptions(project, `ui/dialogs/${p}`) }
          : undefined;
      },
    );
    const source = r.files.find((f) => f.path.endsWith('.cpp'));
    expect(source?.result.ok && source.result.text).toContain(
      '#include "../include/tokotachi_themes.hpp"',
    );
    expect(source?.result.ok && source.result.text).toContain(
      'theme_override = danger_theme().set_radius(0);',
    );
    // 登録していない名前・プロジェクトに属さないウィンドウはエラー
    const unknown = generateCpp(
      doc,
      'settings_dialog.ttui.json',
      () => undefined,
      windowCppOptions(PROJECT, path),
    );
    expect(unknown.files.every((f) => !f.result.ok)).toBe(true);
    const loose = generateCpp(doc, 'settings_dialog.ttui.json', () => undefined);
    expect(loose.files.every((f) => !f.result.ok)).toBe(true);
    // テーマを登録していなければ、ヘッダは作らない
    const files = generateProject(PROJECT, 'my_app.ttproj.json', WINDOWS, () => undefined);
    expect('error' in files ? [] : files.files.map((f) => f.path)).toEqual([
      'main.cpp',
      'CMakeLists.txt',
    ]);
  });
});
