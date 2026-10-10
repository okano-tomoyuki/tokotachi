import { RESERVED_PREFIX } from '@tokotachi-designer/core';
import type { LanguageSyntax } from '../region.ts';
import type { NameRules } from '../stale.ts';

/** C++ のソースの規則。ハンドラが定義済みかは `クラス名::ハンドラ名(` の有無で判定する。 */
export function cppSyntax(className: string): LanguageSyntax {
  return {
    comment: '//',
    indentUnit: '    ',
    blankLinesBeforeAppended: 1,
    // className とハンドラの名前は識別子の検証を通っているので、正規表現の特殊文字を含まない
    hasHandler: (text, name) => new RegExp(`\\b${className}::${name}\\s*\\(`).test(text),
    // stubs のマーカーが無ければ、名前空間の終わりの区間の前(無ければ末尾)に足す
    fallbackStubLine: (lines) => {
      const end = lines.findIndex((l) =>
        /<tokotachi-designer:begin id="tokotachi_namespace_end">/.test(l),
      );
      if (end >= 0) {
        return end;
      }
      return lines[lines.length - 1] === '' ? lines.length - 1 : lines.length;
    },
  };
}

/**
 * C++ の名前の規則(docs/designer/codegen-design.md M1 / M7)。
 * ヘッダの宣言の区間にあるメンバ変数(`tokotachi::Button* ok_button;`)とメンバ関数(ハンドラ)を、生成したメンバとする。
 */
export const CPP_NAMES: NameRules = {
  memberNames: (text) =>
    [...text.matchAll(/^\s*[\w:]+\*\s*(\w+)\s*;/gm), ...text.matchAll(/^\s*void\s+(\w+)\s*\(/gm)]
      .map((m) => m[1] ?? '')
      .filter((name) => !name.startsWith(RESERVED_PREFIX)),
  // ほかのオブジェクトのメンバ(`other.name`・`other->name`)は除く。名前は識別子の検証を通っている
  uses: (line, name) => new RegExp(`(?<![\\w.>])${name}\\b`).test(line),
};
