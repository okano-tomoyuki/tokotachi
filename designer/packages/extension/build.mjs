// 拡張のビルド(1 つの CommonJS にまとめる)。キャンバスの WebAssembly のモジュール(packages/canvas/wasm)を dist/canvas に写す。
//   node build.mjs [--watch]
import { copyFileSync, existsSync, mkdirSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { context } from 'esbuild';

const here = dirname(fileURLToPath(import.meta.url));
const wasm = join(here, '..', 'canvas', 'wasm');
const out = join(here, 'dist', 'canvas');

function copyCanvas() {
  if (!existsSync(join(wasm, 'tokotachi_canvas.wasm'))) {
    console.warn(
      'warning: キャンバスのモジュールが無い(pnpm canvas:build で作る)。キャンバスは描けない',
    );
    return;
  }
  mkdirSync(out, { recursive: true });
  for (const f of ['tokotachi_canvas.mjs', 'tokotachi_canvas.wasm']) {
    copyFileSync(join(wasm, f), join(out, f));
  }
}

const ctx = await context({
  entryPoints: [join(here, 'src', 'extension.ts')],
  bundle: true,
  platform: 'node',
  format: 'cjs',
  target: 'node20',
  external: ['vscode'],
  // jsonc-parser の UMD 版(main)は require を実行時に解決するため、まとめられる ESM 版(module)を優先する
  mainFields: ['module', 'main'],
  sourcemap: true,
  outfile: join(here, 'dist', 'extension.cjs'),
  logLevel: 'info',
});
copyCanvas();
if (process.argv.includes('--watch')) {
  await ctx.watch();
} else {
  await ctx.rebuild();
  await ctx.dispose();
}
