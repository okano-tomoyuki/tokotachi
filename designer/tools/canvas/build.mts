// キャンバスの描画の部分(designer/canvas)を Emscripten で WebAssembly にし、packages/canvas/wasm/ に写す(docs/adr/0039)。
//   node tools/canvas/build.mts
// Emscripten(emsdk)が要る: 環境変数 EMSDK に emsdk のフォルダを設定する(例: C:\tool\emsdk)。
// 依存ライブラリの WebAssembly 版(deps/install/emscripten-wasm32)が無ければ、先に作る(数分かかる)。
import { spawnSync } from 'node:child_process';
import { copyFileSync, existsSync, mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const designer = resolve(here, '../..');
const repoRoot = resolve(designer, '..');
const work = join(designer, '.cache');
const out = join(designer, 'packages', 'canvas', 'wasm');

const emsdkEnv = process.env['EMSDK'];
const emsdk = emsdkEnv ? resolve(emsdkEnv) : undefined;
if (!emsdk || !existsSync(emsdk)) {
  console.error('error: 環境変数 EMSDK に emsdk のフォルダを設定する(例: C:\\tool\\emsdk)');
  process.exit(1);
}

/** emsdk の Emscripten の設定(emsdk activate が書く)を使って、コマンドを実行する。 */
function run(command: string, args: readonly string[]): void {
  const r = spawnSync(command, args, {
    stdio: 'inherit',
    env: { ...process.env, EM_CONFIG: join(emsdk ?? '', '.emscripten') },
  });
  if (r.status !== 0) {
    throw new Error(`失敗した(${String(r.status ?? r.error)}): ${command} ${args.join(' ')}`);
  }
}

const emcmake = join(
  emsdk,
  'upstream',
  'emscripten',
  process.platform === 'win32' ? 'emcmake.exe' : 'emcmake',
);

if (!existsSync(join(repoRoot, 'deps', 'install', 'emscripten-wasm32', 'lib', 'cmake'))) {
  const deps = join(work, 'deps-wasm');
  run(emcmake, ['cmake', '-S', join(repoRoot, 'deps'), '-B', deps, '-G', 'Ninja']);
  run('cmake', ['--build', deps]);
}
const build = join(work, 'canvas');
run(emcmake, [
  'cmake',
  '-S',
  join(designer, 'canvas'),
  '-B',
  build,
  '-G',
  'Ninja',
  '-DCMAKE_BUILD_TYPE=Release',
]);
run('cmake', ['--build', build]);
mkdirSync(out, { recursive: true });
for (const f of ['tokotachi_canvas.mjs', 'tokotachi_canvas.wasm']) {
  copyFileSync(join(build, f), join(out, f));
}
console.log(`${out} に写した`);
