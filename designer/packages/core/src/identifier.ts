/**
 * 定義ファイルの name・ハンドラの名前は生成するクラスのメンバ(C++)の名前になるので、C++ の識別子として使えなければならない
 * (docs/designer/dsl-spec.md §4)。
 */
import { getCatalog } from './catalog/catalog.ts';
import { l10n } from './l10n.ts';

const CPP_KEYWORDS: ReadonlySet<string> = new Set(
  (
    'alignas alignof and and_eq asm auto bitand bitor bool break case catch char char8_t char16_t char32_t class ' +
    'compl concept const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype ' +
    'default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline ' +
    'int long mutable namespace new noexcept not not_eq nullptr operator or or_eq private protected public register ' +
    'reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch template ' +
    'this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t ' +
    'while xor xor_eq'
  ).split(' '),
);

/** 生成したコードが使う名前の接頭辞(docs/designer/codegen-design.md N7)。利用者の名前には使わせない。 */
export const RESERVED_PREFIX = 'tokotachi_';

/** 生成したコードがメンバ関数の中で使う名前(メンバの名前にすると隠れる)。 */
const GENERATED_NAMES: ReadonlySet<string> = new Set(['tokotachi', 'std', 'sender']);

const IDENTIFIER_PATTERN = /^[A-Za-z_][A-Za-z0-9_]*$/;

export type IdentifierProblem =
  'empty' | 'invalid-characters' | 'cpp-keyword' | 'reserved-prefix' | 'cpp-reserved';

/**
 * name が C++ の識別子として使えるかを判定する。
 * @returns 問題がなければ null、あれば最初に見つかった問題
 */
export function isValidIdentifier(name: string): IdentifierProblem | null {
  if (name.length === 0) {
    return 'empty';
  }
  // ASCII に限る(処理系・エディタによって扱いが違うため)
  if (!IDENTIFIER_PATTERN.test(name)) {
    return 'invalid-characters';
  }
  if (CPP_KEYWORDS.has(name)) {
    return 'cpp-keyword';
  }
  if (name.startsWith(RESERVED_PREFIX)) {
    return 'reserved-prefix';
  }
  // C++ では "__" を含む名前と "_大文字" で始まる名前は処理系の予約
  if (name.includes('__') || /^_[A-Z]/.test(name)) {
    return 'cpp-reserved';
  }
  return null;
}

export function describeIdentifierProblem(problem: IdentifierProblem): string {
  switch (problem) {
    case 'empty':
      return l10n.t('Cannot be empty');
    case 'invalid-characters':
      return l10n.t('Only letters, digits and _ can be used, and it cannot start with a digit');
    case 'cpp-keyword':
      return l10n.t('C++ keywords cannot be used');
    case 'reserved-prefix':
      return l10n.t('Names starting with "{0}" are reserved', RESERVED_PREFIX);
    case 'cpp-reserved':
      return l10n.t(
        'Names containing "__" and names starting with "_" and an uppercase letter are reserved in C++',
      );
  }
}

/**
 * 生成するクラスのメンバ(部品・ハンドラ等)の名前として使えるかを判定する。
 * Window・Dialog のメンバ(title・close 等)は、生成したコードが使うか、利用者が使うので、隠してはならない。
 * @returns 問題がなければ null、あれば利用者向けの説明
 */
export function memberNameProblem(name: string): string | null {
  const problem = isValidIdentifier(name);
  if (problem) {
    return describeIdentifierProblem(problem);
  }
  if (getCatalog().window_members.includes(name) || GENERATED_NAMES.has(name)) {
    return l10n.t('"{0}" is a member of Window or a name used by the generated code', name);
  }
  return null;
}
