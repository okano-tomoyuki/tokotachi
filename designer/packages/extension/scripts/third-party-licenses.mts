// 拡張に入れる他者のソフトウェアのライセンス(THIRD_PARTY_LICENSES.txt)を作る(docs/designer/ci.md)。
//   node scripts/third-party-licenses.mts [--check]
// 中身: キャンバス(WebAssembly)に入る Tokotachi の依存(リポジトリの THIRD_PARTY_LICENSES.txt。Lucide を含む)、
// Emscripten の実行部分と musl(licenses/ に写したもの)、拡張・Webview にまとめる JavaScript のパッケージ(node_modules の LICENSE)。
// --check は、作り直したものと食い違えば失敗する(依存の版を変えたら作り直す)。
import { readdirSync, readFileSync, realpathSync, writeFileSync } from 'node:fs';
import { createRequire } from 'node:module';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const extension = join(here, '..');
const repo = join(extension, '..', '..', '..');
const output = join(extension, 'THIRD_PARTY_LICENSES.txt');

/**
 * まとめて入れる JavaScript のパッケージと、どこから解決するか(packages/ の下のパッケージの名前。
 * 依存の依存は [パッケージ, 親] で、親のパッケージの場所から解決する)。
 */
const PACKAGES: readonly (readonly [name: string, from: string, parent?: string])[] = [
  ['react', 'webview'],
  ['react-dom', 'webview'],
  ['scheduler', 'webview', 'react-dom'],
  ['zustand', 'webview'],
  ['zod', 'core'],
  ['jsonc-parser', 'core'],
  ['@vscode/l10n', 'core'],
];

const RULE = '='.repeat(79);

/** MIT の文(LICENSE のファイルを入れていないパッケージは、package.json の作者の著作権で補う)。 */
const MIT = `Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.`;

interface PackageJson {
  readonly version: string;
  readonly license?: string;
  readonly author?: string;
}

/** パッケージのフォルダ(package.json のある所。pnpm のリンクの先の実際の場所)。 */
function packageDir(name: string, from: string, parent?: string): string {
  const base = parent === undefined ? join(extension, '..', from) : packageDir(parent, from);
  const require = createRequire(join(realpathSync(base), 'package.json'));
  return realpathSync(dirname(require.resolve(`${name}/package.json`)));
}

const lf = (text: string): string => text.replace(/\r\n/g, '\n');

function licenseText(dir: string, pkg: PackageJson): string {
  const file = readdirSync(dir).find((f) => /^(license|licence)(\.(md|txt))?$/i.test(f));
  if (file) {
    return lf(readFileSync(join(dir, file), 'utf8')).trim();
  }
  if (pkg.license === 'MIT' && pkg.author !== undefined) {
    return `MIT License\n\nCopyright (c) ${pkg.author}\n\n${MIT}`;
  }
  throw new Error(`${dir} に LICENSE が無い`);
}

function section(title: string, body: string): string {
  return `${RULE}\n${title}\n${RULE}\n\n${body.trim()}\n`;
}

const parts: string[] = [
  'Tokotachi Designer includes the following third-party software.\n',
  section(
    'The Tokotachi library and its dependencies (compiled into the WebAssembly canvas)',
    lf(readFileSync(join(repo, 'THIRD_PARTY_LICENSES.txt'), 'utf8')),
  ),
  section('Emscripten', lf(readFileSync(join(extension, 'licenses', 'emscripten.txt'), 'utf8'))),
  section('musl', lf(readFileSync(join(extension, 'licenses', 'musl.txt'), 'utf8'))),
];
for (const [name, from, parent] of PACKAGES) {
  const dir = packageDir(name, from, parent);
  const pkg = JSON.parse(readFileSync(join(dir, 'package.json'), 'utf8')) as PackageJson;
  parts.push(
    section(
      `${name} ${pkg.version} (${pkg.license ?? 'see below'}; bundled JavaScript)`,
      licenseText(dir, pkg),
    ),
  );
}
const text = parts.join('\n');

if (process.argv.includes('--check')) {
  let current: string;
  try {
    current = lf(readFileSync(output, 'utf8'));
  } catch {
    current = '';
  }
  if (current !== text) {
    console.error(
      'THIRD_PARTY_LICENSES.txt が古い。node packages/extension/scripts/third-party-licenses.mts で作り直す',
    );
    process.exit(1);
  }
} else {
  writeFileSync(output, text);
  console.log(`${output} に書いた`);
}
