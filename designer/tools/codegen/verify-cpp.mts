// 見本の定義ファイルから C++ を生成し、Tokotachi と一緒にビルドして動かして確かめる(docs/designer/codegen-design.md §7)。
//   node tools/codegen/verify-cpp.mts
// C++ のコンパイラ・CMake・Ninja と、ビルド済みの依存ライブラリ(deps/install/<プラットフォーム>)が要る。
// 作業フォルダは designer/.cache/verify-cpp(2 回目からは差分だけビルドする)。
import { spawnSync, type SpawnSyncOptions } from 'node:child_process';
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { hasErrors, parseDocument } from '@tokotachi-designer/core';
import { DEFAULT_CPP_OPTIONS, generateCpp } from '@tokotachi-designer/codegen';

const here = dirname(fileURLToPath(import.meta.url));
const designer = resolve(here, '../..');
const repoRoot = resolve(designer, '..');
const SAMPLES = ['main_window.ttui.json', 'settings_dialog.ttui.json'];
const work = join(designer, '.cache', 'verify-cpp');
const generatedDir = join(work, 'generated');
const buildDir = join(work, 'build');

/** 依存ライブラリのインストール先のプラットフォーム名(cmake/TokotachiPlatform.cmake と同じ)。 */
function platform(): string {
  const os = process.platform === 'win32' ? 'windows' : process.platform;
  const arch =
    process.arch === 'x64' ? 'x86_64' : process.arch === 'arm64' ? 'aarch64' : process.arch;
  return `${os}-${arch}`;
}

function run(command: string, args: readonly string[], options: SpawnSyncOptions = {}): void {
  const r = spawnSync(command, args, { stdio: 'inherit', ...options });
  if (r.status !== 0) {
    throw new Error(`${command} ${args.join(' ')} が失敗した(${String(r.status ?? r.error)})`);
  }
}

/** ハンドラの雛形の中身を、呼ばれたことを記録する処理にする。 */
function recordHandlers(source: string, className: string): string {
  return source.replace(
    new RegExp(`^(void ${className}::(\\w+)\\([^)]*\\)\\n)\\{\\n\\}`, 'gm'),
    (_, head: string, name: string) =>
      `${head}{\n    void verify_record(const char* name);\n    verify_record("${name}");\n}`,
  );
}

/** 見本から C++ を生成して作業フォルダに書く。 */
function generate(file: string): void {
  const parsed = parseDocument(readFileSync(join(designer, 'samples', file), 'utf8'));
  for (const d of parsed.diagnostics) {
    console.error(`${file}: ${d.severity}: ${d.path.join('.')}: ${d.message}`);
  }
  if (!parsed.document || hasErrors(parsed.diagnostics)) {
    throw new Error(`見本の定義ファイル ${file} に誤りがある`);
  }
  const generated = generateCpp(parsed.document, file, () => undefined, {
    ...DEFAULT_CPP_OPTIONS,
    commentLocale: 'ja',
  });
  for (const f of generated.files) {
    if (!f.result.ok) {
      throw new Error(`${f.path}: ${f.result.error}`);
    }
    const text = f.path.endsWith('.cpp')
      ? recordHandlers(f.result.text, parsed.document.window.name)
      : f.result.text;
    writeFileSync(join(generatedDir, f.path), text);
  }
}

function main(): void {
  mkdirSync(generatedDir, { recursive: true });
  for (const file of SAMPLES) {
    generate(file);
  }

  run('cmake', [
    '-S',
    join(here, 'verify'),
    '-B',
    buildDir,
    '-G',
    'Ninja',
    '-DCMAKE_BUILD_TYPE=Release',
    `-DTOKOTACHI_ROOT=${repoRoot}`,
    `-DGENERATED_DIR=${generatedDir}`,
    `-DTOKOTACHI_DEPS_PREFIX=${join(repoRoot, 'deps', 'install', platform())}`,
  ]);
  run('cmake', ['--build', buildDir, '--target', 'verify_cpp']);

  const exe = join(buildDir, process.platform === 'win32' ? 'verify_cpp.exe' : 'verify_cpp');
  const png = join(work, 'main_window.png');
  const r = spawnSync(exe, [png, join(work, 'settings_dialog.png')], {
    encoding: 'utf8',
    env: { ...process.env, SDL_VIDEO_DRIVER: 'offscreen' },
  });
  if (r.error) {
    throw r.error;
  }
  const report = JSON.parse(r.stdout) as {
    checks: { check: string; ok: boolean }[];
    rects: unknown[];
  };
  writeFileSync(join(work, 'report.json'), r.stdout);
  for (const c of report.checks) {
    console.log(`${c.ok ? 'ok  ' : 'FAIL'} ${c.check}`);
  }
  const failed = report.checks.filter((c) => !c.ok).length;
  console.log(
    `${String(report.checks.length - failed)}/${String(report.checks.length)} の項目が通った(メインのウィンドウの部品 ${String(report.rects.length)} 個。画像: ${work})`,
  );
  if (r.status !== 0 || failed > 0) {
    process.exitCode = 1;
  }
}

main();
