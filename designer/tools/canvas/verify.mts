// キャンバス(WebAssembly)で見本を描き、生成したコードをネイティブでビルドして描いた画像(pnpm codegen:verify-cpp)と比べる
// (docs/adr/0039)。同じ操作の並びを実行するので、画素まで同じになるはず。
//   node tools/canvas/verify.mts
// 先に pnpm canvas:build と pnpm codegen:verify-cpp を実行しておく。
import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { Canvas } from '@tokotachi-designer/canvas';
import {
  hasErrors,
  imageReferences,
  normalizePath,
  parseDocument,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import { useSampleCustomClasses } from '../samples.mts';
import { decodePng, encodePng } from './png.mts';

const designer = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const native = join(designer, '.cache', 'verify-cpp');
const work = join(designer, '.cache', 'canvas-verify');

const SAMPLES = ['main_window', 'settings_dialog'];

/** 見本が置いている部品ファイルを読む(パスは samples/ からの相対パス)。 */
function readComponent(path: string): { document: TtuiDocument } | undefined {
  const parsed = parseDocument(readFileSync(join(designer, 'samples', path), 'utf8'));
  return parsed.document ? { document: parsed.document } : undefined;
}

/** 見本が参照する画像を読む。 */
function sampleImages(doc: TtuiDocument): Map<string, Uint8Array> {
  return new Map(
    imageReferences(doc).map((r) => {
      const path = normalizePath(r.path);
      return [path, readFileSync(join(designer, 'samples', path))];
    }),
  );
}

async function main(): Promise<void> {
  mkdirSync(work, { recursive: true });
  // 自作部品(docs/adr/0044)は基底(ProgressBar)として描く。見本の自作部品は描画を変えないので、ネイティブと同じになる
  useSampleCustomClasses();
  let t = performance.now();
  const canvas = await Canvas.load();
  console.log(`読み込み: ${(performance.now() - t).toFixed(0)} ms`);
  let failed = false;
  for (const name of SAMPLES) {
    const parsed = parseDocument(
      readFileSync(join(designer, 'samples', `${name}.ttui.json`), 'utf8'),
    );
    if (!parsed.document || hasErrors(parsed.diagnostics)) {
      throw new Error(`${name}: 定義ファイルに誤りがある`);
    }
    const images = sampleImages(parsed.document);
    t = performance.now();
    const first = canvas.render(parsed.document, { images, components: readComponent });
    const firstMs = performance.now() - t;
    t = performance.now();
    const N = 20;
    for (let i = 0; i < N; i++) {
      canvas.render(parsed.document, { images, components: readComponent });
    }
    const eachMs = (performance.now() - t) / N;
    writeFileSync(join(work, `${name}.png`), encodePng(first));
    console.log(
      `${name}: ${String(first.width)}x${String(first.height)}、部品 ${String(Object.keys(first.rects).length)} 個、` +
        `最初 ${firstMs.toFixed(0)} ms、組み直して描く ${eachMs.toFixed(1)} ms/回`,
    );
    for (const e of first.errors) {
      console.log(`  FAIL 実行できない操作: ${e}`);
      failed = true;
    }
    const nativePng = join(native, `${name}.png`);
    if (!existsSync(nativePng)) {
      console.log(`  ネイティブの画像が無い(pnpm codegen:verify-cpp を先に実行する): ${nativePng}`);
      failed = true;
      continue;
    }
    const expected = decodePng(readFileSync(nativePng));
    if (expected.width !== first.width || expected.height !== first.height) {
      console.log(
        `  FAIL 大きさが違う: ネイティブ ${String(expected.width)}x${String(expected.height)}`,
      );
      failed = true;
      continue;
    }
    let diff = 0;
    for (let i = 0; i < first.pixels.length; i += 4) {
      if (
        first.pixels[i] !== expected.pixels[i] ||
        first.pixels[i + 1] !== expected.pixels[i + 1] ||
        first.pixels[i + 2] !== expected.pixels[i + 2]
      ) {
        diff++;
      }
    }
    console.log(
      `  ${diff === 0 ? 'ok  ' : 'FAIL'} ネイティブと違う画素: ${String(diff)} / ${String(first.width * first.height)}`,
    );
    failed ||= diff !== 0;
  }
  console.log(`画像: ${work}`);
  if (failed) {
    process.exitCode = 1;
  }
}

await main();
