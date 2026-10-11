// プロジェクトのコード生成(docs/designer/project-spec.md §5)を、生成したプロジェクトを実際にビルドして確かめる。
//   node tools/codegen/verify-project.mts
// 見本の 2 つのウィンドウを ui/ に置いたプロジェクト(ヘッダは include/、ソースは src/、名前空間は app)を作業フォルダに作り、
// ウィンドウ・起動部分・CMakeLists.txt を生成して、インストールした Tokotachi(find_package)でビルドする。
// Tokotachi はリポジトリのビルド(既定は ../build。環境変数 TOKOTACHI_BUILD_DIR で変えられる)から、作業フォルダにインストールする。
import { spawnSync } from 'node:child_process';
import { cpSync, mkdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { generateCpp, generateProject, windowCppOptions } from '@tokotachi-designer/codegen';
import { serializeProject, TtuiDocument, type TtprojDocument } from '@tokotachi-designer/core';

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
  theme: { color_scheme: 'LIGHT', override: { colors: { ACCENT: '#7C3AED' }, field_radius: 2 } },
  main_window: 'ui/main_window.ttui.json',
  windows: ['ui/main_window.ttui.json', 'ui/settings_dialog.ttui.json'],
};

rmSync(project, { recursive: true, force: true });
write(join(project, 'sample_app.ttproj.json'), serializeProject(doc));
const windows = (doc.windows ?? []).map((path) => {
  const text = readFileSync(join(designer, 'samples', path.replace('ui/', '')), 'utf8');
  write(join(project, path), text);
  return { path, doc: TtuiDocument.parse(JSON.parse(text)) };
});
// 見本が参照する画像も写す(生成したコードには埋め込むので、ビルドには要らない)
cpSync(join(designer, 'samples', 'icons'), join(project, 'ui', 'icons'), { recursive: true });
for (const w of windows) {
  const generated = generateCpp(
    w.doc,
    w.path,
    () => undefined,
    windowCppOptions(doc, w.path),
    (path) => readFileSync(join(designer, 'samples', path)),
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
  write(join(project, f.path), f.result.text);
}
console.log(`生成した: ${project}`);

const buildDir = process.env['TOKOTACHI_BUILD_DIR'] ?? join(repoRoot, 'build');
run('cmake', ['--install', buildDir, '--prefix', install]);
const appBuild = join(work, 'build');
run('cmake', ['-S', project, '-B', appBuild, '-G', 'Ninja', `-DCMAKE_PREFIX_PATH=${install}`]);
run('cmake', ['--build', appBuild]);
console.log('生成したプロジェクトをビルドできた');
