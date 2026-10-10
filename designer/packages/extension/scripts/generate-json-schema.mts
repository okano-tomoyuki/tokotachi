// 定義ファイル・プロジェクトファイルの JSON Schema(schema/*.schema.json)を Zod のスキーマから作る(エディタの補完・検証用)。
//   node scripts/generate-json-schema.mts [--check]
import { existsSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { TtprojDocument, TtuiDocument } from '@tokotachi-designer/core';
import * as z from 'zod';

const dir = join(dirname(fileURLToPath(import.meta.url)), '..', 'schema');
for (const [file, type] of [
  ['ttui.schema.json', TtuiDocument],
  ['ttproj.schema.json', TtprojDocument],
] as const) {
  const out = join(dir, file);
  const text =
    JSON.stringify(z.toJSONSchema(type, { io: 'input', unrepresentable: 'any' }), null, 2) + '\n';
  if (process.argv.includes('--check')) {
    if (!existsSync(out) || readFileSync(out, 'utf8') !== text) {
      console.error(
        `error: schema/${file} が古い。pnpm --filter tokotachi-designer generate:schema で作り直す`,
      );
      process.exitCode = 1;
    }
  } else {
    writeFileSync(out, text);
    console.log(`${out} に書いた`);
  }
}
