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

/** 書体(標準・太字・斜体・太字の斜体)ごとのフォントの面。合成するかも含めて、ネイティブの探し方の結果と同じにする。 */
export interface FontSource {
  /** 0 標準・1 太字・2 斜体・3 太字の斜体(font_cache.hpp の font_style)。 */
  readonly style: number;
  readonly file: string;
  /** .ttc の中の面の番号。 */
  readonly index: number;
  /** 当たった書体が太字・斜体でないので、太らせる・傾ける。 */
  readonly syntheticBold: boolean;
  readonly syntheticItalic: boolean;
}

/** fontconfig の太さ・傾き(FC_WEIGHT_DEMIBOLD・FC_SLANT_ROMAN)。 */
const FC_WEIGHT_REGULAR = 80;
const FC_WEIGHT_DEMIBOLD = 180;
const FC_SLANT_ROMAN = 0;

/**
 * fontconfig の値(整数。可変フォントの範囲 `[40 250]` は、ネイティブの FcPatternGetInteger が読めないので既定の値にする)。
 */
function fcNumber(text: string | undefined, fallback: number): number {
  const v = Number(text);
  return text === undefined || text === '' || !Number.isFinite(v) ? fallback : v;
}

/**
 * Linux: 書体ごとに fontconfig に問い合わせる(font_cache.cpp の find_system_fonts と同じく、太字・斜体は weight・slant を
 * 付けて問い合わせ、当たった書体が太字・斜体でなければ合成する。各書体で 16 個まで)。
 * キャンバス(WebAssembly)は fontconfig を使えないので、この結果をそのまま渡す(docs/adr/0045)。
 */
export function systemFontSources(): FontSource[] {
  if (process.platform === 'win32') {
    return [];
  }
  const lang = language();
  const base = lang === undefined ? 'sans-serif' : `sans-serif:lang=${lang}`;
  // 太さ・傾きは書体そのものの値(fc-list)で見る。fc-match が出す値は、問い合わせた太さ・傾きと混ぜた後のものなので使わない
  const faces = new Map<string, { weight: number; slant: number }>();
  try {
    const list = execFileSync(
      'fc-list',
      ['--format=%{file}\\t%{index}\\t%{weight}\\t%{slant}\\n'],
      {
        encoding: 'utf8',
        stdio: ['ignore', 'pipe', 'ignore'],
      },
    );
    for (const line of list.split('\n')) {
      const [file, index, weight, slant] = line.split('\t');
      if (file !== undefined && index !== undefined) {
        faces.set(`${file}\t${index}`, {
          weight: fcNumber(weight, FC_WEIGHT_REGULAR),
          slant: fcNumber(slant, FC_SLANT_ROMAN),
        });
      }
    }
  } catch {
    return [];
  }
  const out: FontSource[] = [];
  for (let style = 0; style < 4; style++) {
    const bold = (style & 1) !== 0;
    const italic = (style & 2) !== 0;
    const pattern = `${base}${bold ? ':weight=bold' : ''}${italic ? ':slant=italic' : ''}:scalable=true`;
    let text: string;
    try {
      text = execFileSync('fc-match', ['-s', pattern, '--format=%{file}\\t%{index}\\n'], {
        encoding: 'utf8',
        stdio: ['ignore', 'pipe', 'ignore'],
      });
    } catch {
      return [];
    }
    const seen = new Set<string>();
    for (const line of text.split('\n')) {
      const [file, index] = line.split('\t');
      if (file === undefined || file === '' || !/\.(ttf|otf|ttc|otc)$/i.test(file)) {
        continue;
      }
      const face = faces.get(`${file}\t${index ?? '0'}`) ?? {
        weight: FC_WEIGHT_REGULAR,
        slant: FC_SLANT_ROMAN,
      };
      const source: FontSource = {
        style,
        file,
        index: fcNumber(index, 0),
        syntheticBold: bold && face.weight < FC_WEIGHT_DEMIBOLD,
        syntheticItalic: italic && face.slant === FC_SLANT_ROMAN,
      };
      const key = JSON.stringify(source);
      if (!seen.has(key) && seen.size < 16) {
        seen.add(key);
        out.push(source);
      }
    }
  }
  return out;
}

/** OS のフォントのファイル(見つかったものだけ)。 */
export function systemFonts(): string[] {
  if (process.platform === 'win32') {
    const dir = join(process.env['WINDIR'] ?? 'C:\\Windows', 'Fonts');
    return WINDOWS_FONTS.map((f) => join(dir, f)).filter((f) => existsSync(f));
  }
  return fontconfigFonts();
}
