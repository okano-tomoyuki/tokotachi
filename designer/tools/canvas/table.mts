// カタログから、キャンバスの描画の部分の対応表(designer/canvas/src/generated/table.cpp)を生成する(docs/adr/0039)。
//   node tools/canvas/table.mts          生成して書く
//   node tools/canvas/table.mts --check  生成した結果がコミットしたものと食い違っていればエラー
// カタログを取り出し直したら(pnpm catalog:extract)、これも生成し直す。
import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { getCatalog, type PrimitiveType, type PropertyType } from '@tokotachi-designer/core';

const here = dirname(fileURLToPath(import.meta.url));
const output = resolve(here, '../../canvas/src/generated/table.cpp');
const enumsOutput = resolve(here, '../../canvas/src/generated/enums.hpp');

const PRIMITIVES: Record<PrimitiveType, string> = {
  bool: 'bool',
  int: 'int',
  float: 'float',
  double: 'double',
  string: 'std::string',
};

/** プロパティの型の C++ の型。参照は対応表では扱わない(undefined)。 */
function cppType(type: PropertyType): string | undefined {
  if (typeof type === 'string') {
    return PRIMITIVES[type];
  }
  if ('enum' in type) {
    return `tokotachi::${type.enum}`;
  }
  if ('value' in type) {
    return `tokotachi::${type.value}`;
  }
  if ('array' in type) {
    return `std::vector<${PRIMITIVES[type.array]}>`;
  }
  return undefined;
}

function quote(s: string): string {
  return JSON.stringify(s);
}

function generate(): string {
  const catalog = getCatalog();
  const out: string[] = [];
  const line = (s = ''): void => {
    out.push(s);
  };
  line('// 生成したファイル(tools/canvas/table.mts がカタログから作る)。手で変えない。');
  line('#include "../table.hpp"');
  line('#include "../values.hpp"');
  line();
  line('namespace ttc');
  line('{');
  line();
  line('// ---- 列挙型 ----');
  for (const [name, info] of Object.entries(catalog.enums)) {
    line();
    line(`bool from_json(const Json& j, tokotachi::${name}& out)`);
    line('{');
    line('    if (!j.is_string())');
    line('    {');
    line('        return false;');
    line('    }');
    line('    const std::string& s = j.as_string();');
    for (const v of info.values) {
      line(`    if (s == ${quote(v)})`);
      line('    {');
      line(`        out = tokotachi::${name}::${v};`);
      line('        return true;');
      line('    }');
    }
    line('    return false;');
    line('}');
    line();
    line(`Json to_json(tokotachi::${name} v)`);
    line('{');
    line('    switch (v)');
    line('    {');
    for (const v of info.values) {
      line(`    case tokotachi::${name}::${v}:`);
      line(`        return Json(std::string(${quote(v)}));`);
    }
    line('    }');
    line('    return Json();');
    line('}');
  }

  line();
  line('namespace');
  line('{');
  for (const [cls, info] of Object.entries(catalog.classes)) {
    const props = Object.entries(info.properties).flatMap(([name, p]) => {
      const t = cppType(p.type);
      return t === undefined ? [] : [{ name, type: t }];
    });
    line();
    line(`bool set_${cls}(tokotachi::${cls}* o, const std::string& p, const Json& v)`);
    line('{');
    for (const p of props) {
      line(`    if (p == ${quote(p.name)})`);
      line('    {');
      line(`        ${p.type} t;`);
      line('        if (!from_json(v, t))');
      line('        {');
      line('            return false;');
      line('        }');
      line(`        o->${p.name} = t;`);
      line('        return true;');
      line('    }');
    }
    if (props.length === 0) {
      line('    (void)o;');
      line('    (void)p;');
      line('    (void)v;');
    }
    line('    return false;');
    line('}');
    line();
    line(`bool get_${cls}(tokotachi::${cls}* o, const std::string& p, Json& out)`);
    line('{');
    for (const p of props) {
      line(`    if (p == ${quote(p.name)})`);
      line('    {');
      line(`        out = to_json(o->${p.name}.get());`);
      line('        return true;');
      line('    }');
    }
    if (props.length === 0) {
      line('    (void)o;');
      line('    (void)p;');
      line('    (void)out;');
    }
    line('    return false;');
    line('}');
  }
  line();
  line('}  // namespace');

  // 作れるクラス: 部品・画面に出ないオブジェクト・レイアウトと、ウィンドウに付けるもの
  const creatable = Object.entries(catalog.classes).filter(
    ([cls, info]) =>
      info.kind === 'widget' ||
      info.kind === 'object' ||
      info.kind === 'layout' ||
      ['Action', 'MenuBar', 'ToolBar', 'StatusBar'].includes(cls),
  );
  line();
  line('tokotachi::Object* create(const std::string& cls, tokotachi::Object* parent)');
  line('{');
  line('    tokotachi::Widget* widget = dynamic_cast<tokotachi::Widget*>(parent);');
  for (const [cls, info] of creatable) {
    line(`    if (cls == ${quote(cls)})`);
    line('    {');
    if (info.kind === 'object' || cls === 'Action') {
      line(`        return new tokotachi::${cls}(parent);`);
    } else if (info.kind === 'layout') {
      line(`        return widget ? new tokotachi::${cls}(widget) : nullptr;`);
    } else {
      line(`        return new tokotachi::${cls}(widget);`);
    }
    line('    }');
  }
  line('    return nullptr;');
  line('}');
  line();
  line(
    'bool set_property(tokotachi::Object* object, const std::string& cls, const std::string& property, const Json& value)',
  );
  line('{');
  for (const cls of Object.keys(catalog.classes)) {
    line(`    if (cls == ${quote(cls)})`);
    line('    {');
    line(`        return set_${cls}(static_cast<tokotachi::${cls}*>(object), property, value);`);
    line('    }');
  }
  line('    return false;');
  line('}');
  line();
  line(
    'bool get_property(tokotachi::Object* object, const std::string& cls, const std::string& property, Json& out)',
  );
  line('{');
  for (const cls of Object.keys(catalog.classes)) {
    line(`    if (cls == ${quote(cls)})`);
    line('    {');
    line(`        return get_${cls}(static_cast<tokotachi::${cls}*>(object), property, out);`);
    line('    }');
  }
  line('    return false;');
  line('}');
  line();
  line('std::vector<std::string> property_names(const std::string& cls)');
  line('{');
  for (const [cls, info] of Object.entries(catalog.classes)) {
    const names = Object.entries(info.properties)
      .filter(([, p]) => cppType(p.type) !== undefined)
      .map(([n]) => quote(n));
    line(`    if (cls == ${quote(cls)})`);
    line('    {');
    line(`        return std::vector<std::string>{${names.join(', ')}};`);
    line('    }');
  }
  line('    return std::vector<std::string>();');
  line('}');
  line();
  line('std::vector<std::string> creatable_classes()');
  line('{');
  line(`    return std::vector<std::string>{${creatable.map(([c]) => quote(c)).join(', ')}};`);
  line('}');
  line();
  line('}  // namespace ttc');
  return `${out.join('\n')}\n`;
}

/** 列挙型の変換の宣言(values.hpp が読む)。 */
function generateEnums(): string {
  const out = [
    '// 生成したファイル(tools/canvas/table.mts がカタログから作る)。手で変えない。',
    '#ifndef TOKOTACHI_CANVAS_GENERATED_ENUMS_HPP',
    '#define TOKOTACHI_CANVAS_GENERATED_ENUMS_HPP',
    '',
    '#include <tokotachi/tokotachi.hpp>',
    '',
    '#include "../json.hpp"',
    '',
    'namespace ttc',
    '{',
    '',
  ];
  for (const name of Object.keys(getCatalog().enums)) {
    out.push(`bool from_json(const Json& j, tokotachi::${name}& out);`);
    out.push(`Json to_json(tokotachi::${name} v);`);
  }
  out.push('', '}  // namespace ttc', '', '#endif');
  return `${out.join('\n')}\n`;
}

let stale = false;
for (const [path, text] of [
  [output, generate()],
  [enumsOutput, generateEnums()],
] as const) {
  const previous = existsSync(path) ? readFileSync(path, 'utf8') : undefined;
  if (process.argv.includes('--check')) {
    stale ||= text !== previous;
  } else {
    mkdirSync(dirname(path), { recursive: true });
    writeFileSync(path, text);
    console.log(`${path} に書いた`);
  }
}
if (stale) {
  console.error(
    'error: キャンバスの対応表がカタログと食い違っている。pnpm canvas:table で生成し直す',
  );
  process.exitCode = 1;
}
