/**
 * マーカー区間の内容のハッシュ(docs/designer/codegen-design.md M2 / M3)。
 * 空白(改行・インデントを含む)を取り除いてから計算するため、フォーマッタによる改行や字下げの違いでは変わらない。
 * 改ざん検知ではなく「手で編集されたか」の検出が目的なので、軽量な FNV-1a(32bit)を使う。
 */

/** 正規表現の `\s` と同じ空白か(区間は大きいことがあるので、空白を除いた写しを作らずに飛ばす)。 */
function isSpace(c: number): boolean {
  return (
    (c >= 0x09 && c <= 0x0d) ||
    c === 0x20 ||
    c === 0xa0 ||
    c === 0x1680 ||
    (c >= 0x2000 && c <= 0x200a) ||
    c === 0x2028 ||
    c === 0x2029 ||
    c === 0x202f ||
    c === 0x205f ||
    c === 0x3000 ||
    c === 0xfeff
  );
}

export function regionHash(content: string): string {
  let hash = 0x811c9dc5;
  for (let i = 0; i < content.length; i++) {
    const c = content.charCodeAt(i);
    if (isSpace(c)) {
      continue;
    }
    hash ^= c;
    hash = Math.imul(hash, 0x01000193) >>> 0;
  }
  return hash.toString(16).padStart(8, '0');
}
