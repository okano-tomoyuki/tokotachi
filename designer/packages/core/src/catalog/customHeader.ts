/**
 * 自作部品のヘッダを読む(docs/adr/0044)。利用者が C++ で書いた部品のクラスから、基底クラス・プロパティ・ハンドラを取り出して
 * カタログのクラスの情報にする。C++ をすべて解析するのではなく、決まった書き方(`class X : public Base`・`Property<T> name;`)だけを読む。
 * core は Webview でも動くため、ファイルを読むのは呼び出し側(拡張・CLI)。
 */
import { l10n } from '../l10n.ts';
import { getCatalog } from './catalog.ts';
import type { ClassInfo, HandlerInfo, PrimitiveType, PropertyInfo, PropertyType } from './types.ts';

/** ヘッダから読んだプロパティ(型は C++ の書き方のまま)。 */
interface RawMember {
  readonly name: string;
  readonly type: string;
  readonly initializer?: string | undefined;
  readonly doc?: string | undefined;
  readonly line: number;
}

/** ヘッダから読んだクラス。 */
interface RawClass {
  readonly name: string;
  readonly qualified: string;
  /** 基底クラス(`tokotachi::` を除いた名前)。 */
  readonly base: string;
  readonly doc?: string | undefined;
  readonly members: readonly RawMember[];
  readonly line: number;
}

export interface CustomHeader {
  /** プロジェクトのフォルダからの相対パス。 */
  readonly path: string;
  readonly text: string;
}

export interface CustomHeaderProblem {
  /** ヘッダ(プロジェクトのフォルダからの相対パス)。 */
  readonly header: string;
  /** 1 から始まる行。 */
  readonly line: number;
  readonly message: string;
}

export interface CustomClasses {
  readonly classes: Record<string, ClassInfo>;
  readonly problems: readonly CustomHeaderProblem[];
}

type Context =
  | { readonly kind: 'namespace'; readonly name: string }
  | {
      readonly kind: 'class';
      readonly name: string;
      readonly base: string | undefined;
      readonly doc: string | undefined;
      readonly line: number;
      access: 'public' | 'private';
      readonly members: RawMember[];
    }
  | { readonly kind: 'other' };

/** 文字の並びを空白 1 つにまとめる。 */
function squash(text: string): string {
  return text.replace(/\s+/g, ' ').trim();
}

/**
 * コメントを空白にし(改行は残す)、`///` の説明を行ごとに取り出す。文字列の中のコメントの記号は残す。
 */
function stripComments(text: string): { code: string; docs: Map<number, string> } {
  const docs = new Map<number, string>();
  let code = '';
  let line = 1;
  for (let i = 0; i < text.length;) {
    const c = text.charAt(i);
    const next = text.charAt(i + 1);
    if (c === '"' || c === "'") {
      // 文字列・文字
      let j = i + 1;
      while (j < text.length && text[j] !== c && text[j] !== '\n') {
        j += text[j] === '\\' ? 2 : 1;
      }
      code += text.slice(i, j + 1);
      i = j + 1;
      continue;
    }
    if (c === '/' && next === '/') {
      const end = text.indexOf('\n', i);
      const comment = text.slice(i, end < 0 ? text.length : end);
      if (comment.startsWith('///') && !comment.startsWith('////')) {
        const body = comment.slice(3).trim();
        docs.set(line, docs.has(line) ? `${docs.get(line) ?? ''} ${body}` : body);
      }
      code += ' '.repeat(comment.length);
      i += comment.length;
      continue;
    }
    if (c === '/' && next === '*') {
      const end = text.indexOf('*/', i + 2);
      const comment = text.slice(i, end < 0 ? text.length : end + 2);
      code += comment.replace(/[^\n]/g, ' ');
      line += comment.split('\n').length - 1;
      i += comment.length;
      continue;
    }
    if (c === '\n') {
      line++;
    }
    code += c;
    i++;
  }
  return { code, docs };
}

/** line の直前に続く `///` の説明。 */
function docBefore(docs: ReadonlyMap<number, string>, line: number): string | undefined {
  const lines: string[] = [];
  for (let l = line - 1; docs.has(l); l--) {
    lines.unshift(docs.get(l) ?? '');
  }
  // 同じ行の後ろに書いた説明(`Property<int> level; ///< 段階`)
  const trailing = docs.get(line)?.replace(/^</, '').trim();
  const text = [...lines, ...(trailing !== undefined ? [trailing] : [])].join(' ').trim();
  return text === '' ? undefined : text;
}

const PROPERTY = /^(?:tokotachi\s*::\s*)?Property\s*<(.+)>\s*(\w+)\s*(?:=\s*(.+))?$/s;
const CLASS = /^(?:class|struct)\s+(\w+)\s*(?:final\s*)?:\s*(?:public\s+)?([\w:]+)/;
const NAMESPACE = /^namespace\s+([\w:]+)$/;

/** ヘッダの中のクラス(基底クラスを持つもの)と、その public の Property を読む。 */
function readHeader(text: string): RawClass[] {
  const stripped = stripComments(text);
  const { docs } = stripped;
  // プリプロセッサの行(#include・#pragma 等)は読まない
  const code = stripped.code.replace(/^[ \t]*#.*$/gm, (m) => ' '.repeat(m.length));
  const out: RawClass[] = [];
  const stack: Context[] = [];
  let statement = '';
  let statementLine = 1;
  let line = 1;
  const top = (): Context | undefined => stack[stack.length - 1];
  const namespaces = (): string[] =>
    stack.flatMap((c) => (c.kind === 'namespace' ? c.name.split('::') : []));
  /** 文の先頭のアクセス指定(`public:` 等)を読み、残りを返す。 */
  const takeAccess = (s: string): string => {
    let rest = s;
    for (;;) {
      const m = /^(public|protected|private)\s*:(?!:)\s*/.exec(rest);
      if (!m) {
        return rest;
      }
      const ctx = top();
      if (ctx?.kind === 'class') {
        ctx.access = m[1] === 'public' ? 'public' : 'private';
      }
      rest = rest.slice(m[0].length);
    }
  };
  const begin = (): void => {
    if (statement.trim() === '') {
      statementLine = line;
    }
  };
  for (let i = 0; i < code.length; i++) {
    const c = code[i] ?? '';
    if (c === '\n') {
      line++;
    }
    if (c === '"' || c === "'") {
      begin();
      let j = i + 1;
      while (j < code.length && code[j] !== c && code[j] !== '\n') {
        j += code[j] === '\\' ? 2 : 1;
      }
      statement += code.slice(i, j + 1);
      i = j;
      continue;
    }
    if (c === '{' || c === '}' || c === ';') {
      const s = squash(takeAccess(squash(statement)));
      const ctx = top();
      if (c === '{') {
        // `Property<int> level{3};` の初期化は、文の続きとして読む
        if (ctx?.kind === 'class' && PROPERTY.test(s) && !s.includes('=')) {
          const end = code.indexOf('}', i);
          statement += ` = ${code.slice(i + 1, end < 0 ? code.length : end)}`;
          line += (code.slice(i, end).match(/\n/g) ?? []).length;
          i = end < 0 ? code.length : end;
          continue;
        }
        const ns = NAMESPACE.exec(s);
        const cls = CLASS.exec(s);
        if (ns?.[1] !== undefined) {
          stack.push({ kind: 'namespace', name: ns[1] });
        } else if (cls?.[1] !== undefined && !/^template\b/.test(s)) {
          stack.push({
            kind: 'class',
            name: cls[1],
            base: cls[2]?.replace(/^(::)?tokotachi::/, ''),
            doc: docBefore(docs, statementLine),
            line: statementLine,
            access: s.startsWith('struct') ? 'public' : 'private',
            members: [],
          });
        } else {
          stack.push({ kind: 'other' });
        }
      } else if (c === '}') {
        const closed = stack.pop();
        if (closed?.kind === 'class' && closed.base !== undefined) {
          out.push({
            name: closed.name,
            qualified: [...namespaces(), closed.name].join('::'),
            base: closed.base,
            doc: closed.doc,
            members: closed.members,
            line: closed.line,
          });
        }
      } else if (ctx?.kind === 'class' && ctx.access === 'public') {
        const m = PROPERTY.exec(s);
        if (m?.[1] !== undefined && m[2] !== undefined) {
          ctx.members.push({
            name: m[2],
            type: m[1],
            initializer: m[3]?.trim(),
            doc: docBefore(docs, statementLine),
            line: statementLine,
          });
        }
      }
      statement = '';
      continue;
    }
    begin();
    statement += c;
  }
  return out;
}

const PRIMITIVES: Readonly<Record<string, PrimitiveType>> = {
  bool: 'bool',
  int: 'int',
  float: 'float',
  double: 'double',
  'std::string': 'string',
  string: 'string',
};

type MemberKind = { property: PropertyType } | { handler: string } | undefined;

/** C++ の型をカタログの型にする(読めなければ undefined)。 */
function memberKind(raw: string, known: (name: string) => boolean): MemberKind {
  const type = squash(raw)
    .replace(/\btokotachi\s*::\s*/g, '')
    .replace(/\s*([<>*:,])\s*/g, '$1');
  const catalog = getCatalog();
  const primitive = PRIMITIVES[type];
  if (primitive) {
    return { property: primitive };
  }
  const array = /^std::vector<(.+)>$/.exec(type);
  if (array?.[1] !== undefined) {
    const item = PRIMITIVES[array[1]];
    return item ? { property: { array: item } } : undefined;
  }
  const pointer = /^(\w+)\*$/.exec(type);
  if (pointer?.[1] !== undefined) {
    return known(pointer[1]) ? { property: { ref: pointer[1] } } : undefined;
  }
  if (Object.hasOwn(catalog.handler_types, type)) {
    return { handler: type };
  }
  if (Object.hasOwn(catalog.enums, type)) {
    return { property: { enum: type } };
  }
  if (Object.hasOwn(catalog.values, type)) {
    return { property: { value: type } };
  }
  return undefined;
}

/** 既定値の文字(`3`・`0.5f`・`true`・`"abc"`・`Align::CENTER`)を値にする。読めなければ undefined。 */
function literal(text: string | undefined, type: PropertyType): unknown {
  if (text === undefined) {
    return undefined;
  }
  const t = squash(text);
  if (type === 'bool') {
    return t === 'true' ? true : t === 'false' ? false : undefined;
  }
  if (type === 'int' || type === 'float' || type === 'double') {
    const m = /^([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)[fF]?$/.exec(t);
    const n = m?.[1] === undefined ? Number.NaN : Number(m[1]);
    return Number.isFinite(n) ? (type === 'int' ? Math.trunc(n) : n) : undefined;
  }
  if (type === 'string') {
    const m = /^(?:std::string\s*\(\s*)?"((?:[^"\\]|\\.)*)"\s*\)?$/.exec(t);
    return m?.[1]?.replace(/\\(.)/g, (_, ch: string) => (ch === 'n' ? '\n' : ch));
  }
  if (typeof type === 'object' && 'enum' in type) {
    const m = /(\w+)$/.exec(t);
    const value = m?.[1];
    return value !== undefined && getCatalog().enums[type.enum]?.values.includes(value)
      ? value
      : undefined;
  }
  return undefined;
}

/**
 * 登録したヘッダから自作部品のクラスの情報を作る。基底はカタログの部品か、同じく登録した自作部品。
 */
export function readCustomClasses(headers: readonly CustomHeader[]): CustomClasses {
  const problems: CustomHeaderProblem[] = [];
  const raw = new Map<string, { cls: RawClass; header: string }>();
  for (const h of headers) {
    for (const cls of readHeader(h.text)) {
      if (getCatalog().classes[cls.name] !== undefined) {
        problems.push({
          header: h.path,
          line: cls.line,
          message: l10n.t('"{0}" is the name of a class of the library', cls.name),
        });
      } else if (raw.has(cls.name)) {
        problems.push({
          header: h.path,
          line: cls.line,
          message: l10n.t('The class "{0}" is declared in more than one header', cls.name),
        });
      } else {
        raw.set(cls.name, { cls, header: h.path });
      }
    }
  }
  const classes: Record<string, ClassInfo> = {};
  const known = (name: string): boolean =>
    raw.has(name) || getCatalog().classes[name] !== undefined;
  const resolving = new Set<string>();
  /** 部品の派生でないクラス(ヘッダの中のほかのクラス)は undefined。 */
  const resolve = (name: string): ClassInfo | undefined => {
    const done = classes[name];
    if (done) {
      return done;
    }
    const entry = raw.get(name);
    if (!entry || resolving.has(name)) {
      return undefined;
    }
    resolving.add(name);
    const { cls, header } = entry;
    const libraryBase = getCatalog().classes[cls.base];
    const base = libraryBase ?? resolve(cls.base);
    resolving.delete(name);
    if (!base || base.kind !== 'widget') {
      return undefined;
    }
    const properties: Record<string, PropertyInfo> = { ...base.properties };
    const handlers: Record<string, HandlerInfo> = { ...base.handlers };
    for (const m of cls.members) {
      const kind = memberKind(m.type, known);
      if (!kind) {
        problems.push({
          header,
          line: m.line,
          message: l10n.t(
            'The property "{0}" of {1} is not shown in the designer (unsupported type: {2})',
            m.name,
            cls.name,
            squash(m.type),
          ),
        });
        continue;
      }
      if ('handler' in kind) {
        handlers[m.name] = {
          type: kind.handler,
          declared_in: cls.name,
          ...(m.doc !== undefined ? { doc: m.doc } : {}),
        };
        continue;
      }
      const value = literal(m.initializer, kind.property);
      properties[m.name] = {
        type: kind.property,
        declared_in: cls.name,
        ...(m.doc !== undefined ? { doc: m.doc } : {}),
        ...(value !== undefined ? { default: value } : {}),
      };
    }
    const info: ClassInfo = {
      bases: [cls.base, ...base.bases],
      kind: 'widget',
      category: 'custom',
      ...(cls.doc !== undefined ? { doc: cls.doc } : {}),
      properties,
      handlers,
      custom: {
        header,
        qualified: cls.qualified,
        base: libraryBase ? cls.base : (base.custom?.base ?? cls.base),
      },
    };
    classes[name] = info;
    return info;
  };
  for (const [name, { cls, header }] of raw) {
    if (!resolve(name) && (known(cls.base) || raw.has(cls.base))) {
      problems.push({
        header,
        line: cls.line,
        message: l10n.t('The base class "{0}" of {1} is not a widget', cls.base, name),
      });
    }
  }
  return { classes, problems };
}
