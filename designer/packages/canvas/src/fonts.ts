/**
 * キャンバスに渡す OS のフォントのファイル(docs/adr/0039)。
 * WebAssembly の Tokotachi はフォントを探せないので、ネイティブの Tokotachi(src/graphics/font_cache.cpp)と同じ順で探して渡す。
 * 同じフォントで描くので、キャンバスの文字の大きさ・配置はネイティブと同じになる。
 */
import { execFileSync } from 'node:child_process';
import { existsSync } from 'node:fs';
import { join } from 'node:path';

/** Windows: Fonts フォルダの既知のファイル(font_cache.cpp と同じ順)。 */
const WINDOWS_FONTS = [
  'segoeui.ttf',
  'YuGothM.ttc',
  'meiryo.ttc',
  'msgothic.ttc',
  'seguisym.ttf',
  'seguiemj.ttf',
  'malgun.ttf',
  'msyh.ttc',
];

/** ロケールの言語(fontconfig に渡す。font_cache.cpp と同じく LC_ALL・LC_CTYPE・LANG の順)。 */
function language(): string | undefined {
  const env = process.env;
  const lang = env['LC_ALL'] || env['LC_CTYPE'] || env['LANG'];
  if (!lang || lang.length < 2 || lang.startsWith('C') || lang.startsWith('POSIX')) {
    return undefined;
  }
  return lang.slice(0, 2);
}

/** Linux: fontconfig に sans-serif を問い合わせる(font_cache.cpp と同じく、文字の範囲が広い順に 16 個まで)。 */
function fontconfigFonts(): string[] {
  const lang = language();
  const pattern = lang === undefined ? 'sans-serif' : `sans-serif:lang=${lang}`;
  try {
    const out = execFileSync('fc-match', ['-s', pattern, '--format=%{file}\\n'], {
      encoding: 'utf8',
      stdio: ['ignore', 'pipe', 'ignore'],
    });
    const files = out.split('\n').filter((f) => f !== '' && /\.(ttf|otf|ttc|otc)$/i.test(f));
    return [...new Set(files)].slice(0, 16);
  } catch {
    return [];
  }
}

/** OS のフォントのファイル(見つかったものだけ)。 */
export function systemFonts(): string[] {
  if (process.platform === 'win32') {
    const dir = join(process.env['WINDIR'] ?? 'C:\\Windows', 'Fonts');
    return WINDOWS_FONTS.map((f) => join(dir, f)).filter((f) => existsSync(f));
  }
  return fontconfigFonts();
}
