/**
 * 画像の埋め込み(docs/designer/codegen-design.md N8)。定義ファイルに書いた画像のファイルの中身を、
 * 区間 tokotachi_setup_ui の中に unsigned char の配列として書き、`tokotachi::Image::load_from_data` で読む。
 */
import { normalizePath } from '@tokotachi-designer/core';

/** 1 行に書くバイトの数。 */
const BYTES_PER_LINE = 16;

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

/** 配列の定義(無名の名前空間の中。前に説明のコメント)。 */
export function imageDefinitions(images: readonly EmbeddedImage[], comment: string): string[] {
  if (images.length === 0) {
    return [];
  }
  const lines = [`// ${comment}`, 'namespace', '{', ''];
  for (const image of images) {
    lines.push(`// ${image.path}`, `const unsigned char ${image.variable}[] = {`);
    for (let i = 0; i < image.data.length; i += BYTES_PER_LINE) {
      const bytes = Array.from(
        image.data.subarray(i, i + BYTES_PER_LINE),
        (b) => `0x${b.toString(16).toUpperCase().padStart(2, '0')}`,
      );
      lines.push(`    ${bytes.join(', ')},`);
    }
    lines.push('};', '');
  }
  lines.push('}  // namespace', '');
  return lines;
}

/** 画像を読む式。 */
export function imageExpression(variable: string): string {
  return `tokotachi::Image::load_from_data(${variable}, sizeof(${variable}))`;
}
