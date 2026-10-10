// 拡張の開発用のホストで開くフォルダ(.cache/playground)に、見本の写しを置く(無いものだけ。試した変更は残す)。
import { copyFileSync, existsSync, mkdirSync, readdirSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const designer = join(dirname(fileURLToPath(import.meta.url)), '..', '..');
const playground = join(designer, '.cache', 'playground');
mkdirSync(playground, { recursive: true });
for (const f of readdirSync(join(designer, 'samples'))) {
  if (f.endsWith('.ttui.json') && !existsSync(join(playground, f))) {
    copyFileSync(join(designer, 'samples', f), join(playground, f));
  }
}
console.log(`playground: ${playground}`);
