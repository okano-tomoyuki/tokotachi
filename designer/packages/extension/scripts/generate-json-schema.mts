// 定義ファイルの JSON Schema(schema/ttui.schema.json)を Zod のスキーマから作る(エディタの補完・検証用)。
//   node scripts/generate-json-schema.mts [--check]
import { existsSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { TtuiDocument } from '@tokotachi-designer/core';
import * as z from 'zod';

const out = join(dirname(fileURLToPath(import.meta.url)), '..', 'schema', 'ttui.schema.json');
const schema = z.toJSONSchema(TtuiDocument, { io: 'input', unrepresentable: 'any' });
const text = JSON.stringify(schema, null, 2) + '\n';
if (process.argv.includes('--check')) {
  if (!existsSync(out) || readFileSync(out, 'utf8') !== text) {
    console.error(
      'error: schema/ttui.schema.json が古い。pnpm --filter tokotachi-designer generate:schema で作り直す',
    );
    process.exitCode = 1;
  }
} else {
  writeFileSync(out, text);
  console.log(`${out} に書いた`);
}
