// カタログを公開のヘッダから取り出す(docs/designer/catalog.md §2)。
//   node tools/catalog/extract.mts          取り出して packages/core/src/catalog/catalog.json に書く
//   node tools/catalog/extract.mts --check  取り出した結果がコミットしたカタログと食い違っていればエラー
// Doxygen が要る(PATH にあるか、環境変数 DOXYGEN で指定する)。
import { spawnSync } from 'node:child_process';
import { mkdtempSync, readFileSync, readdirSync, rmSync, writeFileSync, existsSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { DOMParser, type Element } from '@xmldom/xmldom';
import type {
  Catalog,
  Category,
  ClassInfo,
  ClassKind,
  EditorKind,
  EnumInfo,
  HandlerInfo,
  HandlerParam,
  PrimitiveType,
  PropertyInfo,
  PropertyRange,
  PropertyType,
  ValueTypeInfo,
} from '../../packages/core/src/catalog/types.ts';

const here = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(here, '../../..');
const includeDir = join(repoRoot, 'include', 'tokotachi');
const catalogPath = resolve(here, '../../packages/core/src/catalog/catalog.json');
const overlayPath = join(here, 'overlay.json');

// ---- 補足 ----

interface OverlayClass {
  kind: ClassKind;
  category?: Category;
  accepts_children?: boolean;
  child_keys?: string[];
}

interface Overlay {
  classes: Record<string, OverlayClass>;
  excluded: Record<string, string>;
  not_designable: Record<string, string[]>;
  designable: Record<string, string[]>;
  geometry: Record<string, string[]>;
  after_children: Record<string, string[]>;
  editor: Record<string, EditorKind>;
  range: Record<string, PropertyRange>;
  extra_enums: string[];
  extra_values: string[];
  special_values: string[];
}

// ---- Doxygen の XML ----

function runDoxygen(outDir: string): string {
  const doxygen = process.env['DOXYGEN'] ?? 'doxygen';
  const config = [
    `INPUT = "${includeDir}"`,
    'RECURSIVE = YES',
    'FILE_PATTERNS = *.hpp',
    'EXTRACT_ALL = YES',
    'EXTRACT_PRIVATE = NO',
    'EXCLUDE_SYMBOLS = tokotachi::detail tokotachi::detail::*',
    // 公開の印(TOKOTACHI_API)と非推奨の印(TOKOTACHI_DEPRECATED。docs/adr/0057)は空にして読む
    'MACRO_EXPANSION = YES',
    'EXPAND_ONLY_PREDEF = YES',
    'PREDEFINED = TOKOTACHI_API= "TOKOTACHI_DEPRECATED(message)="',
    'GENERATE_HTML = NO',
    'GENERATE_LATEX = NO',
    'GENERATE_XML = YES',
    'XML_PROGRAMLISTING = NO',
    `OUTPUT_DIRECTORY = "${outDir}"`,
    'QUIET = YES',
    'WARNINGS = NO',
  ].join('\n');
  const r = spawnSync(doxygen, ['-'], { input: config, encoding: 'utf8' });
  if (r.error) {
    throw new Error(`Doxygen を起動できない(${doxygen}): ${r.error.message}`);
  }
  if (r.status !== 0) {
    throw new Error(`Doxygen が失敗した(${String(r.status)}): ${r.stderr}`);
  }
  return join(outDir, 'xml');
}

function elements(parent: Element, tag: string): Element[] {
  const out: Element[] = [];
  for (let n = parent.firstChild; n; n = n.nextSibling) {
    if (n.nodeType === n.ELEMENT_NODE && (n as Element).tagName === tag) {
      out.push(n as Element);
    }
  }
  return out;
}

function child(parent: Element, tag: string): Element | undefined {
  return elements(parent, tag)[0];
}

function descendants(parent: Element, tag: string): Element[] {
  return Array.from(parent.getElementsByTagName(tag));
}

/** 要素の文字列(コード例・見出しは除く)。空白は 1 つにまとめる。 */
function plainText(e: Element | undefined): string {
  if (!e) {
    return '';
  }
  let s = '';
  const walk = (n: Element): void => {
    for (let c = n.firstChild; c; c = c.nextSibling) {
      if (c.nodeType === c.TEXT_NODE) {
        s += c.nodeValue ?? '';
      } else if (c.nodeType === c.ELEMENT_NODE) {
        const tag = (c as Element).tagName;
        if (tag !== 'programlisting' && tag !== 'simplesect' && tag !== 'parameterlist') {
          walk(c as Element);
        }
      }
    }
  };
  walk(e);
  return s.replace(/\s+/g, ' ').trim();
}

/** 説明: brief、無ければ detailed の最初の段落。 */
function docOf(e: Element): string {
  const brief = plainText(child(e, 'briefdescription'));
  if (brief) {
    return brief;
  }
  const detailed = child(e, 'detaileddescription');
  const para = detailed ? child(detailed, 'para') : undefined;
  return plainText(para);
}

/** 型の文字列を正規化する(`Property< std::vector< std::string > >` → `Property<std::vector<std::string>>`)。 */
function normalizeType(t: string): string {
  return t
    .replace(/\btokotachi::/g, '')
    .replace(/\s*([<>*&,])\s*/g, '$1')
    .replace(/,/g, ', ')
    .trim();
}

function stripNamespace(name: string): string {
  return name.replace(/^tokotachi::/, '');
}

interface RawMember {
  name: string;
  type: string;
  doc: string;
}

interface RawClass {
  name: string;
  kind: string;
  bases: string[];
  doc: string;
  file: string;
  variables: RawMember[];
  /** public のメンバ関数の名前(コンストラクタ・演算子を除く)。 */
  functions: string[];
  constructors: string[];
  /** テンプレート(Variable<T> 等)か。 */
  template: boolean;
}

interface RawEnum {
  name: string;
  doc: string;
  values: { name: string; doc: string }[];
}

interface RawXml {
  classes: Map<string, RawClass>;
  enums: Map<string, RawEnum>;
  typedefs: Map<string, string>;
  fileDocs: Map<string, string>;
}

function parse(path: string): Element {
  const doc = new DOMParser().parseFromString(readFileSync(path, 'utf8'), 'text/xml');
  return doc.documentElement as unknown as Element;
}

function readMembers(def: Element, raw: RawXml): RawMember[] {
  const vars: RawMember[] = [];
  for (const m of descendants(def, 'memberdef')) {
    if (m.getAttribute('prot') !== 'public') {
      continue;
    }
    const kind = m.getAttribute('kind');
    const name = plainText(child(m, 'name'));
    if (kind === 'variable' && m.getAttribute('static') !== 'yes') {
      vars.push({ name, type: normalizeType(plainText(child(m, 'type'))), doc: docOf(m) });
    } else if (kind === 'enum') {
      raw.enums.set(name, {
        name,
        doc: docOf(m),
        values: elements(m, 'enumvalue').map((v) => ({
          name: plainText(child(v, 'name')),
          doc: docOf(v),
        })),
      });
    } else if (kind === 'typedef') {
      raw.typedefs.set(name, normalizeType(plainText(child(m, 'type'))));
    }
  }
  return vars;
}

function readXml(xmlDir: string): RawXml {
  const raw: RawXml = {
    classes: new Map(),
    enums: new Map(),
    typedefs: new Map(),
    fileDocs: new Map(),
  };
  for (const f of readdirSync(xmlDir).sort()) {
    if (!f.endsWith('.xml') || f === 'index.xml' || f === 'Doxyfile.xml') {
      continue;
    }
    const def = child(parse(join(xmlDir, f)), 'compounddef');
    if (!def) {
      continue;
    }
    const kind = def.getAttribute('kind') ?? '';
    const name = plainText(child(def, 'compoundname'));
    if (kind === 'file') {
      raw.fileDocs.set(name, docOf(def));
    } else if (kind === 'namespace' && name === 'tokotachi') {
      readMembers(def, raw);
    } else if ((kind === 'class' || kind === 'struct') && name.startsWith('tokotachi::')) {
      const short = stripNamespace(name);
      const location = child(def, 'location');
      const file = (location?.getAttribute('file') ?? '').replace(/^.*\//, '');
      const constructors = descendants(def, 'memberdef')
        .filter(
          (m) =>
            m.getAttribute('kind') === 'function' &&
            m.getAttribute('prot') === 'public' &&
            plainText(child(m, 'name')) === short.replace(/^.*::/, ''),
        )
        .map((m) => normalizeType(plainText(child(m, 'argsstring'))));
      const functions = descendants(def, 'memberdef')
        .filter((m) => m.getAttribute('kind') === 'function' && m.getAttribute('prot') === 'public')
        .map((m) => plainText(child(m, 'name')))
        .filter(
          (n) =>
            n !== short.replace(/^.*::/, '') && !n.startsWith('~') && !n.startsWith('operator'),
        );
      raw.classes.set(short, {
        name: short,
        kind,
        bases: elements(def, 'basecompoundref').map((b) => stripNamespace(plainText(b))),
        doc: docOf(def),
        file,
        variables: readMembers(def, raw),
        functions,
        constructors,
        template: child(def, 'templateparamlist') !== undefined,
      });
    }
  }
  return raw;
}

// ---- カタログを組み立てる ----

const PRIMITIVES: Record<string, PrimitiveType> = {
  bool: 'bool',
  int: 'int',
  float: 'float',
  double: 'double',
  'std::string': 'string',
};

class Builder {
  readonly raw: RawXml;
  readonly overlay: Overlay;
  readonly usedEnums = new Set<string>();
  readonly usedValues = new Set<string>();
  readonly usedHandlers = new Set<string>();
  readonly problems: string[] = [];

  constructor(raw: RawXml, overlay: Overlay) {
    this.raw = raw;
    this.overlay = overlay;
  }

  /** 値の型か(public のメンバを持つ struct か、専用の書き方の型)。 */
  isValueType(name: string): boolean {
    if (this.overlay.special_values.includes(name)) {
      return true;
    }
    const c = this.raw.classes.get(name);
    return c?.kind === 'struct' && c.variables.length > 0;
  }

  /** C++ の型からプロパティの型を決める。扱えない型は undefined。 */
  mapType(t: string): PropertyType | undefined {
    const p = PRIMITIVES[t];
    if (p) {
      return p;
    }
    const vec = /^std::vector<(.+)>$/.exec(t);
    if (vec?.[1] !== undefined) {
      const inner = PRIMITIVES[vec[1]];
      return inner ? { array: inner } : undefined;
    }
    const ptr = /^(\w+)\*$/.exec(t);
    if (ptr?.[1] !== undefined) {
      return this.raw.classes.has(ptr[1]) ? { ref: ptr[1] } : undefined;
    }
    if (this.raw.enums.has(t)) {
      return { enum: t };
    }
    if (this.isValueType(t)) {
      return { value: t };
    }
    return undefined;
  }

  /** クラスから基底へ、近い順に並べた継承の連なり(自分を含む)。 */
  chain(name: string): RawClass[] {
    const out: RawClass[] = [];
    for (let c = this.raw.classes.get(name); c;) {
      out.push(c);
      const base = c.bases.find((b) => this.raw.classes.has(b));
      c = base === undefined ? undefined : this.raw.classes.get(base);
    }
    return out;
  }

  /** chain の中で、そのクラスか基底に補足の一覧があるか(派生の designable で戻せる)。 */
  listed(chain: RawClass[], table: Record<string, string[]>, prop: string): boolean {
    return chain.some((c) => table[c.name]?.includes(prop) ?? false);
  }

  designable(chain: RawClass[], prop: string): boolean {
    // 派生に近いほうの指定を優先する
    for (const c of chain) {
      if (this.overlay.designable[c.name]?.includes(prop)) {
        return true;
      }
      if (this.overlay.not_designable[c.name]?.includes(prop)) {
        return false;
      }
    }
    return true;
  }

  buildClass(name: string, info: OverlayClass): ClassInfo {
    const chain = this.chain(name);
    const self = chain[0];
    if (!self) {
      throw new Error(`補足の classes の ${name} がヘッダに無い`);
    }
    const properties: Record<string, PropertyInfo> = {};
    const handlers: Record<string, HandlerInfo> = {};
    // 基底から順に(宣言の順に)並べる
    for (const c of [...chain].reverse()) {
      for (const v of c.variables) {
        const m = /^(ReadOnly)?Property<(.+)>$/.exec(v.type);
        if (!m?.[2]) {
          continue;
        }
        const inner = m[2];
        if (this.raw.typedefs.get(inner)?.startsWith('std::function<') === true) {
          this.usedHandlers.add(inner);
          handlers[v.name] = { type: inner, declared_in: c.name, ...(v.doc ? { doc: v.doc } : {}) };
          continue;
        }
        if (m[1] || !this.designable(chain, v.name)) {
          continue;
        }
        const type = this.mapType(inner);
        if (!type) {
          continue;
        }
        if (typeof type === 'object' && 'enum' in type) {
          this.usedEnums.add(type.enum);
        }
        if (typeof type === 'object' && 'value' in type) {
          this.usedValues.add(type.value);
        }
        const editor = chain
          .map((k) => this.overlay.editor[`${k.name}.${v.name}`])
          .find((e) => e !== undefined);
        const range = chain
          .map((k) => this.overlay.range[`${k.name}.${v.name}`])
          .find((r) => r !== undefined);
        properties[v.name] = {
          type,
          declared_in: c.name,
          ...(v.doc ? { doc: v.doc } : {}),
          ...(this.listed(chain, this.overlay.geometry, v.name) ? { geometry: true as const } : {}),
          ...(this.listed(chain, this.overlay.after_children, v.name)
            ? { after_children: true as const }
            : {}),
          ...(editor ? { editor } : {}),
          ...(range ? { range } : {}),
        };
      }
    }
    const doc = self.doc || (this.raw.fileDocs.get(self.file) ?? '');
    return {
      bases: chain.slice(1).map((c) => c.name),
      kind: info.kind,
      ...(info.category ? { category: info.category } : {}),
      ...(info.accepts_children ? { accepts_children: true as const } : {}),
      ...(info.child_keys ? { child_keys: info.child_keys } : {}),
      ...(self.template ? { template: true as const } : {}),
      ...(doc ? { doc } : {}),
      properties,
      handlers,
    };
  }

  /** Widget の派生で、親を渡して作れるもの(パレットに出せるもの)が、補足の classes か excluded にあるか。 */
  checkCoverage(): void {
    for (const c of this.raw.classes.values()) {
      const isWidget = this.chain(c.name).some((k) => k.name === 'Widget');
      const creatable = c.constructors.some((a) => a.startsWith('(Widget*parent=nullptr'));
      if (
        isWidget &&
        creatable &&
        !(c.name in this.overlay.classes) &&
        !(c.name in this.overlay.excluded)
      ) {
        this.problems.push(
          `${c.name} は親を渡して作れる部品だが、補足の classes にも excluded にも無い(overlay.json に書く)`,
        );
      }
    }
    for (const name of Object.keys(this.overlay.excluded)) {
      if (!this.raw.classes.has(name)) {
        this.problems.push(`補足の excluded の ${name} がヘッダに無い`);
      }
    }
  }

  buildEnum(name: string): EnumInfo {
    const e = this.raw.enums.get(name);
    if (!e) {
      throw new Error(`列挙型 ${name} がヘッダに無い`);
    }
    const docs: Record<string, string> = {};
    for (const v of e.values) {
      if (v.doc) {
        docs[v.name] = v.doc;
      }
    }
    return {
      values: e.values.map((v) => v.name),
      ...(Object.keys(docs).length > 0 ? { docs } : {}),
      ...(e.doc ? { doc: e.doc } : {}),
    };
  }

  buildValue(name: string): ValueTypeInfo {
    const c = this.raw.classes.get(name);
    if (!c) {
      throw new Error(`値の型 ${name} がヘッダに無い`);
    }
    const doc = c.doc ? { doc: c.doc } : {};
    if (this.overlay.special_values.includes(name)) {
      return { special: true, ...doc };
    }
    const members: Record<string, PrimitiveType | { enum: string }> = {};
    for (const v of c.variables) {
      const p = PRIMITIVES[v.type];
      if (p) {
        members[v.name] = p;
      } else if (this.raw.enums.has(v.type)) {
        this.usedEnums.add(v.type);
        members[v.name] = { enum: v.type };
      } else if (/^std::uint8_t$/.test(v.type)) {
        members[v.name] = 'int';
      } else {
        throw new Error(`値の型 ${name} のメンバ ${v.name} の型 ${v.type} を扱えない`);
      }
    }
    return { members, ...doc };
  }

  buildHandlerType(name: string): HandlerParam[] {
    const t = this.raw.typedefs.get(name) ?? '';
    const m = /^std::function<void\((.*)\)>$/.exec(t);
    if (!m) {
      throw new Error(`ハンドラの型 ${name} を読めない: ${t}`);
    }
    const args = m[1] ?? '';
    if (!args) {
      return [];
    }
    return args.split(', ').map((a) => {
      const p = /^(.*?[*&]?)(\w+)$/.exec(a.replace(/([*&])(\w)/, '$1 $2'));
      if (!p?.[1] || !p[2]) {
        throw new Error(`ハンドラの型 ${name} の引数を読めない: ${a}`);
      }
      return { name: p[2], type: p[1].trim() };
    });
  }

  build(): Catalog {
    this.checkCoverage();
    const classes: Record<string, ClassInfo> = {};
    for (const [name, info] of Object.entries(this.overlay.classes)) {
      classes[name] = this.buildClass(name, info);
    }
    for (const v of this.overlay.extra_values) {
      this.usedValues.add(v);
    }
    const values: Record<string, ValueTypeInfo> = {};
    for (const name of [...this.usedValues].sort()) {
      values[name] = this.buildValue(name);
    }
    for (const e of this.overlay.extra_enums) {
      this.usedEnums.add(e);
    }
    const enums: Record<string, EnumInfo> = {};
    for (const name of [...this.usedEnums].sort()) {
      enums[name] = this.buildEnum(name);
    }
    const handler_types: Record<string, HandlerParam[]> = {};
    for (const name of [...this.usedHandlers].sort()) {
      handler_types[name] = this.buildHandlerType(name);
    }
    // 生成するクラスのメンバの名前と衝突してはならない、ウィンドウの public のメンバ
    const windowMembers = new Set<string>();
    for (const [name, info] of Object.entries(this.overlay.classes)) {
      if (info.kind === 'window') {
        for (const c of this.chain(name)) {
          c.variables.forEach((v) => windowMembers.add(v.name));
          c.functions.forEach((f) => windowMembers.add(f));
        }
      }
    }
    return {
      format_version: 1,
      tokotachi_version: readVersion(),
      window_members: [...windowMembers].sort(),
      classes,
      values,
      enums,
      handler_types,
    };
  }
}

function readVersion(): string {
  const text = readFileSync(join(includeDir, 'core', 'version.hpp'), 'utf8');
  const part = (k: string): string => {
    const m = new RegExp(`#define TOKOTACHI_VERSION_${k} (\\d+)`).exec(text);
    if (!m?.[1]) {
      throw new Error(`version.hpp に TOKOTACHI_VERSION_${k} が無い`);
    }
    return m[1];
  };
  return `${part('MAJOR')}.${part('MINOR')}.${part('PATCH')}`;
}

/** 測った既定値・望む大きさは取り出し直しても消さない(測り直すまで前の値を使う。pnpm catalog:measure)。 */
function carryDefaults(next: Catalog, prev: Partial<Catalog> | undefined): void {
  if (!prev?.classes) {
    return;
  }
  for (const [cls, info] of Object.entries(next.classes)) {
    const size = prev.classes[cls]?.preferred_size;
    if (size) {
      info.preferred_size = size;
    }
    for (const [prop, p] of Object.entries(info.properties)) {
      const old = prev.classes[cls]?.properties[prop];
      if (old && 'default' in old && JSON.stringify(old.type) === JSON.stringify(p.type)) {
        p.default = old.default;
      }
    }
  }
}

function main(): void {
  const check = process.argv.includes('--check');
  const overlay = JSON.parse(readFileSync(overlayPath, 'utf8')) as Overlay;
  const work = mkdtempSync(join(tmpdir(), 'tokotachi-catalog-'));
  let catalog: Catalog;
  let problems: string[];
  try {
    const builder = new Builder(readXml(runDoxygen(work)), overlay);
    catalog = builder.build();
    problems = builder.problems;
  } finally {
    rmSync(work, { recursive: true, force: true });
  }
  const prevText = existsSync(catalogPath) ? readFileSync(catalogPath, 'utf8') : undefined;
  carryDefaults(catalog, prevText ? (JSON.parse(prevText) as Catalog) : undefined);
  const text = JSON.stringify(catalog, null, 2) + '\n';

  for (const p of problems) {
    console.error(`error: ${p}`);
  }
  if (check) {
    if (text !== prevText) {
      console.error(
        'error: カタログがヘッダ・補足と食い違っている。pnpm catalog:extract で取り出し直す',
      );
      process.exitCode = 1;
    }
  } else {
    writeFileSync(catalogPath, text);
    const count = Object.keys(catalog.classes).length;
    console.log(`${catalogPath} に書いた(クラス ${String(count)} 個)`);
  }
  if (problems.length > 0) {
    process.exitCode = 1;
  }
}

main();
