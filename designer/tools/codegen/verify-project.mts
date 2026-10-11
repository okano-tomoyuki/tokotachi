// プロジェクトのコード生成(docs/designer/project-spec.md §5)を、生成したプロジェクトを実際にビルドして確かめる。
//   node tools/codegen/verify-project.mts
// 見本のウィンドウ・部品ファイルを ui/ に、自作部品を widgets/ に置いたプロジェクト(ヘッダは include/、ソースは src/、名前空間は app)を
// 作業フォルダに作り、
// ウィンドウ・起動部分・CMakeLists.txt を生成して、インストールした Tokotachi(find_package)でビルドする。
// Tokotachi はリポジトリのビルド(既定は ../build。環境変数 TOKOTACHI_BUILD_DIR で変えられる)から、作業フォルダにインストールする。
import { spawnSync } from 'node:child_process';
import { cpSync, mkdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { generateCpp, generateProject, windowCppOptions } from '@tokotachi-designer/codegen';
import {
  normalizePath,
  readCustomClasses,
  serializeProject,
  setCustomClasses,
  TtuiDocument,
  type TtprojDocument,
} from '@tokotachi-designer/core';

const designer = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const repoRoot = resolve(designer, '..');
const work = join(designer, '.cache', 'verify-project');
const install = join(designer, '.cache', 'tokotachi-install');
const project = join(work, 'project');

function run(command: string, args: readonly string[]): void {
  const r = spawnSync(command, args, { stdio: 'inherit' });
  if (r.status !== 0) {
    throw new Error(`${command} ${args.join(' ')} が失敗した(${String(r.status ?? r.error)})`);
  }
}

function write(path: string, text: string): void {
  mkdirSync(dirname(path), { recursive: true });
  writeFileSync(path, text);
}

const doc: TtprojDocument = {
  format_version: 1,
  codegen: { comment_locale: 'ja', namespace: 'app', header_dir: 'include', source_dir: 'src' },
  theme: {
    color_scheme: 'LIGHT',
    override: { colors: { ACCENT: '#7C3AED' }, field_radius: 2 },
    presets: { danger: { colors: { ACCENT: '#DC2626' }, radius: 0 } },
  },
  main_window: 'ui/main_window.ttui.json',
  windows: [
    'ui/main_window.ttui.json',
    'ui/settings_dialog.ttui.json',
    'ui/parts/labeled_edit.ttui.json',
  ],
  custom_widgets: ['widgets/level_bar.hpp'],
};

rmSync(project, { recursive: true, force: true });
write(join(project, 'sample_app.ttproj.json'), serializeProject(doc));
const windows = (doc.windows ?? []).map((path) => {
  let text = readFileSync(join(designer, 'samples', path.replace('ui/', '')), 'utf8');
  // 登録したテーマの参照を試す(見本はプロジェクトに属さないので、ここで足す)
  text = text.replace(
    '"style": "SECONDARY"',
    '"style": "SECONDARY", "theme_override": { "preset": "danger", "field_padding": 4 }',
  );
  write(join(project, path), text);
  return { path, doc: TtuiDocument.parse(JSON.parse(text)) };
});
// 見本が参照する画像も写す(生成したコードには埋め込むので、ビルドには要らない)
cpSync(join(designer, 'samples', 'icons'), join(project, 'ui', 'icons'), { recursive: true });
// 自作部品(docs/adr/0044)のヘッダ・ソースを写し、カタログに足す
cpSync(join(designer, 'samples', 'widgets'), join(project, 'widgets'), { recursive: true });
setCustomClasses(
  readCustomClasses(
    (doc.custom_widgets ?? []).map((path) => ({
      path,
      text: readFileSync(join(project, path), 'utf8'),
    })),
  ).classes,
);
const byPath = new Map(windows.map((w) => [w.path, w.doc]));
for (const w of windows) {
  const generated = generateCpp(
    w.doc,
    w.path,
    () => undefined,
    windowCppOptions(doc, w.path),
    (path) => readFileSync(join(designer, 'samples', path)),
    (path) => {
      const componentPath = normalizePath(`${dirname(w.path)}/${path}`);
      const component = byPath.get(componentPath);
      return component
        ? { document: component, options: windowCppOptions(doc, componentPath) }
        : undefined;
    },
  );
  for (const f of generated.files) {
    if (!f.result.ok) {
      throw new Error(f.result.error);
    }
    write(join(project, dirname(w.path), f.path), f.result.text);
  }
}
const startup = generateProject(doc, 'sample_app.ttproj.json', windows, () => undefined);
if ('error' in startup) {
  throw new Error(startup.error);
}
for (const f of startup.files) {
  if (!f.result.ok) {
    throw new Error(f.result.error);
  }
  // 自作部品のソースは利用者が CMakeLists.txt に足す(生成する区間の外)
  const text =
    f.path === 'CMakeLists.txt'
      ? `${f.result.text}\ntarget_sources(sample_app PRIVATE widgets/level_bar.cpp)\n`
      : f.result.text;
  write(join(project, f.path), text);
}
console.log(`生成した: ${project}`);

const buildDir = process.env['TOKOTACHI_BUILD_DIR'] ?? join(repoRoot, 'build');
run('cmake', ['--install', buildDir, '--prefix', install]);
const appBuild = join(work, 'build');
run('cmake', ['-S', project, '-B', appBuild, '-G', 'Ninja', `-DCMAKE_PREFIX_PATH=${install}`]);
run('cmake', ['--build', appBuild]);
console.log('生成したプロジェクトをビルドできた');
