/**
 * 画像の埋め込み(docs/designer/codegen-design.md N8・docs/adr/0053)。定義ファイルに書いた画像のファイルの中身を、
 * 別のソース(*_images.cpp)に unsigned char の配列として書き、ウィンドウのソースは宣言だけを持って
 * `tokotachi::Image::load_from_data` で読む(画像が大きくても、ウィンドウのソースは読みやすく、ハンドラを直しても配列をコンパイルし直さない)。
 */
import { normalizePath } from '@tokotachi-designer/core';

/** 1 行に書くバイトの数。 */
const BYTES_PER_LINE = 16;

/** バイトの値 → `0xAB`。 */
const HEX: readonly string[] = Array.from(
  { length: 256 },
  (_, b) => `0x${b.toString(16).toUpperCase().padStart(2, '0')}`,
);

/** 埋め込む画像(同じファイルは 1 つ)。 */
export interface EmbeddedImage {
  /** 定義ファイルのフォルダからの相対パス(正規化したもの)。 */
  readonly path: string;
  /** 配列の名前(`tokotachi_image_icons_open_png`)。 */
  readonly variable: string;
  readonly data: Uint8Array;
}

/** 画像のパスの、配列の名前にする部分(英数字以外を `_` に)。 */
function identifierPart(path: string): string {
  return path
    .replace(/^(\.\.\/)+/, '')
    .replace(/[^A-Za-z0-9]+/g, '_')
    .replace(/^_+|_+$/g, '')
    .toLowerCase();
}

/**
 * 埋め込む画像を並べ、配列の名前を決める。
 * @param paths 定義ファイルに書いたパス(現れた順)
 * @param data 正規化したパス → 中身
 */
export function embedImages(
  paths: readonly string[],
  data: ReadonlyMap<string, Uint8Array>,
): EmbeddedImage[] {
  const out: EmbeddedImage[] = [];
  const used = new Set<string>();
  for (const raw of paths) {
    const path = normalizePath(raw);
    const bytes = data.get(path);
    if (bytes === undefined || out.some((i) => i.path === path)) {
      continue;
    }
    const base = `tokotachi_image_${identifierPart(path)}`;
    let variable = base;
    for (let n = 2; used.has(variable); n++) {
      variable = `${base}_${String(n)}`;
    }
    used.add(variable);
    out.push({ path, variable, data: bytes });
  }
  return out;
}

/** 画像の配列を入れる名前空間(ウィンドウのクラスごと。ウィンドウの名前空間の中)。 */
export function imagesNamespace(className: string): string {
  return `tokotachi_images_${className}`;
}

/** 配列の大きさの変数の名前。 */
function sizeVariable(variable: string): string {
  return `${variable}_size`;
}

/** ウィンドウのソースに書く、別のファイルに定義した配列の宣言(前に説明のコメント)。 */
export function imageDeclarations(
  images: readonly EmbeddedImage[],
  className: string,
  comment: string,
): string[] {
  if (images.length === 0) {
    return [];
  }
  return [
    `// ${comment}`,
    `namespace ${imagesNamespace(className)}`,
    '{',
    ...images.flatMap((image) => [
      `extern const unsigned char ${image.variable}[];`,
      `extern const std::size_t ${sizeVariable(image.variable)};`,
    ]),
    '}',
    '',
  ];
}

/**
 * 画像のファイル(*_images.cpp)の中身。ウィンドウと同じ名前空間の中の、ウィンドウのクラスごとの名前空間に配列と大きさを定義する
 * (別の翻訳単位なので、先に extern で宣言して外部結合にする)。
 */
export function imageDefinitions(
  images: readonly EmbeddedImage[],
  className: string,
  namespaceBegin: string,
  namespaceEnd: string,
): string[] {
  const lines = ['#include <cstddef>', ''];
  if (namespaceBegin !== '') {
    lines.push(namespaceBegin, '');
  }
  lines.push(`namespace ${imagesNamespace(className)}`, '{', '');
  for (const image of images) {
    const size = sizeVariable(image.variable);
    lines.push(
      `// ${image.path}`,
      `extern const unsigned char ${image.variable}[];`,
      `extern const std::size_t ${size};`,
      `const unsigned char ${image.variable}[] = {`,
    );
    // 画像は大きいことがある(1 MB で 6 万行)ので、バイトの文字は表から引き、行ごとに 1 つの文字列を作る
    const data = image.data;
    for (let i = 0; i < data.length; i += BYTES_PER_LINE) {
      const end = Math.min(i + BYTES_PER_LINE, data.length);
      let line = `    ${HEX[data[i] ?? 0] ?? ''}`;
      for (let j = i + 1; j < end; j++) {
        line += `, ${HEX[data[j] ?? 0] ?? ''}`;
      }
      lines.push(`${line},`);
    }
    lines.push('};', `const std::size_t ${size} = sizeof(${image.variable});`, '');
  }
  lines.push(`}  // namespace ${imagesNamespace(className)}`);
  if (namespaceEnd !== '') {
    lines.push('', namespaceEnd);
  }
  return lines;
}

/** 画像を読む式。 */
export function imageExpression(className: string, variable: string): string {
  const ns = imagesNamespace(className);
  return `tokotachi::Image::load_from_data(${ns}::${variable}, ${ns}::${sizeVariable(variable)})`;
}
