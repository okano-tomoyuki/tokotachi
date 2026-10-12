// 拡張の版の決まりを確かめる(docs/adr/0064)。pnpm check から呼ぶ。
//   - 1.0 より前は "preview": true、1.0 からは preview を付けない。
//   - 拡張の大きい版は、カタログを取り出したライブラリの大きい版と同じ(デザイナー 1.x は Tokotachi 1.x 用)。
import { readFileSync } from 'node:fs';
import { join } from 'node:path';

const root = join(import.meta.dirname, '..', '..');
const pkg = JSON.parse(
  readFileSync(join(root, 'packages', 'extension', 'package.json'), 'utf8'),
) as {
  version: string;
  preview?: boolean;
};
const catalog = JSON.parse(
  readFileSync(join(root, 'packages', 'core', 'src', 'catalog', 'catalog.json'), 'utf8'),
) as { tokotachi_version: string };

const problems: string[] = [];
const major = Number(pkg.version.split('.')[0]);
const libraryMajor = Number(catalog.tokotachi_version.split('.')[0]);
if (major < 1 && pkg.preview !== true) {
  problems.push(`版 ${pkg.version} は 1.0 より前なので、package.json に "preview": true を付ける`);
}
if (major >= 1 && pkg.preview !== undefined) {
  problems.push(`版 ${pkg.version} は 1.0 以降なので、package.json の "preview" を外す`);
}
if (major !== libraryMajor) {
  problems.push(
    `拡張の大きい版(${pkg.version})と、カタログのライブラリの大きい版(${catalog.tokotachi_version})をそろえる`,
  );
}
if (problems.length > 0) {
  for (const p of problems) {
    console.error(`error: ${p}`);
  }
  process.exit(1);
}
console.log(`拡張 ${pkg.version}(Tokotachi ${catalog.tokotachi_version} 用)`);
