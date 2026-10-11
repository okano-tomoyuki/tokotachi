// コード生成のテストの期待する出力(packages/codegen/src/__golden__/)を、見本から作り直す。
//   node tools/codegen/golden.mts
// 生成するコードを変えたら実行し、差分を確かめてからコミットする。
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { TtuiDocument } from '@tokotachi-designer/core';
import { DEFAULT_CPP_OPTIONS, generateCpp } from '@tokotachi-designer/codegen';

const designer = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const golden = join(designer, 'packages', 'codegen', 'src', '__golden__');

const read = (file: string): TtuiDocument =>
  TtuiDocument.parse(JSON.parse(readFileSync(join(designer, 'samples', file), 'utf8')));

for (const file of [
  'main_window.ttui.json',
  'settings_dialog.ttui.json',
  'parts/labeled_edit.ttui.json',
]) {
  const doc = read(file);
  const readImage = (path: string): Uint8Array => readFileSync(join(designer, 'samples', path));
  const readComponent = (path: string) => ({
    document: read(path),
    options: DEFAULT_CPP_OPTIONS,
  });
  const name = file.split('/').pop() ?? file;
  for (const f of generateCpp(doc, name, () => undefined, undefined, readImage, readComponent)
    .files) {
    if (!f.result.ok) {
      throw new Error(f.result.error);
    }
    writeFileSync(join(golden, f.path), f.result.text);
    console.log(`${f.path} を書いた`);
  }
}
