// カタログの既定値を、キャンバス(WebAssembly の Tokotachi)で各クラスを作って測る(docs/designer/catalog.md §2 の 2)。
//   node tools/catalog/measure.mts
// 先に pnpm canvas:build を実行しておく。測った値(プロパティの default と、部品の preferred_size)を catalog.json に書く。
// 取り出し直し(pnpm catalog:extract)は、測った値を引き継ぐ。
// 望む大きさは文字の大きさで変わるので、Windows で測った値を基準にする(コミットするのは Windows で測ったもの)。
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { Canvas } from '@tokotachi-designer/canvas';
import type { Catalog } from '@tokotachi-designer/core';

const catalogPath = resolve(
  dirname(fileURLToPath(import.meta.url)),
  '../../packages/core/src/catalog/catalog.json',
);

const catalog = JSON.parse(readFileSync(catalogPath, 'utf8')) as Catalog;
const canvas = await Canvas.load();
const measured = canvas.defaults();
let count = 0;
const missing: string[] = [];
for (const [cls, info] of Object.entries(catalog.classes)) {
  const m = measured[cls];
  if (!m) {
    missing.push(cls);
    continue;
  }
  for (const [name, p] of Object.entries(info.properties)) {
    if (typeof p.type === 'object' && 'ref' in p.type) {
      // 参照の既定値はいつも「無し」(書かない)
      continue;
    }
    if (m.properties[name] === null) {
      // 値で書けない既定値(画像の「無し」)は書かない
      delete p.default;
    } else if (name in m.properties) {
      p.default = m.properties[name];
      count++;
    } else {
      missing.push(`${cls}.${name}`);
    }
  }
  if (m.preferred_size && info.kind === 'widget') {
    info.preferred_size = m.preferred_size;
  }
}
writeFileSync(catalogPath, JSON.stringify(catalog, null, 2) + '\n');
console.log(`${catalogPath} に、既定値 ${String(count)} 個を書いた`);
if (missing.length > 0) {
  console.log(`測れなかったもの: ${missing.join(', ')}`);
}
