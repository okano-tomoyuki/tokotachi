// Lucide(https://lucide.dev/。ISC。一部は Feather の MIT)のアイコンを、ライブラリの線画のデータにする(docs/adr/0054)。
//
//   npm pack lucide-static@1.48.0      (空のフォルダで。展開: tar xzf lucide-static-1.48.0.tgz)
//   node tools/icons/generate.mjs <展開した package フォルダ>
//
// 書くもの(手で直さない):
//   include/tokotachi/graphics/icon.hpp  列挙型 Icon(名前は Lucide の名前を大文字にしたもの)
//   src/graphics/icon_data.cpp           24 × 24 の格子の線画(移動・直線・3 次ベジェ・閉じる)
//   examples/gallery/icon_names.hpp      ギャラリーのサンプルが出す名前
//   designer/packages/core/src/dsl/icons.json  デザイナーが使う一覧(定義ファイルの名前・列挙の名前・分類・SVG のパス)
//
// アイコンを足すときは、下の ICONS に Lucide の名前を足して実行し直す(列挙の値は並びの順なので、足すのは分類の終わり)。
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const VERSION = '1.48.0';

/** 分類ごとのアイコン(Lucide の正式な名前。別名は使わない)。 */
const ICONS = [
  ['ファイル', ['file', 'file-plus', 'file-text', 'folder', 'folder-open', 'save', 'printer', 'download', 'upload']],
  [
    '編集',
    ['undo-2', 'redo-2', 'scissors', 'copy', 'clipboard-paste', 'trash', 'pencil', 'plus', 'minus', 'x', 'check'],
  ],
  [
    '移動',
    [
      'arrow-left',
      'arrow-right',
      'arrow-up',
      'arrow-down',
      'chevron-left',
      'chevron-right',
      'chevron-up',
      'chevron-down',
      'house',
      'menu',
      'ellipsis',
      'refresh-cw',
      'external-link',
    ],
  ],
  [
    '表示',
    ['search', 'zoom-in', 'zoom-out', 'eye', 'eye-off', 'funnel', 'settings', 'sliders-horizontal', 'list', 'layout-grid'],
  ],
  ['状態', ['info', 'triangle-alert', 'circle-x', 'circle-check', 'circle-question-mark', 'bell']],
  ['再生', ['play', 'pause', 'square', 'skip-back', 'skip-forward', 'volume-2', 'volume-x']],
  [
    'その他',
    [
      'user',
      'users',
      'mail',
      'calendar',
      'clock',
      'lock',
      'lock-open',
      'star',
      'heart',
      'link',
      'image',
      'log-in',
      'log-out',
      'sun',
      'moon',
    ],
  ],
];

/** 分類の英語の名前(デザイナーの画面で訳して出す)。 */
const CATEGORY_EN = {
  ファイル: 'File',
  編集: 'Edit',
  移動: 'Navigation',
  表示: 'View',
  状態: 'Status',
  再生: 'Media',
  その他: 'Other',
};

const MOVE = 0;
const LINE = 1;
const CUBIC = 2;
const CLOSE = 3;

// ---- SVG の path の d を、移動・直線・3 次ベジェ・閉じるにする ----

function tokenize(d) {
  const tokens = [];
  const re = /([MmLlHhVvCcSsQqTtAaZz])|(-?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?)/g;
  let m;
  while ((m = re.exec(d)) !== null) {
    tokens.push(m[1] !== undefined ? m[1] : Number(m[2]));
  }
  return tokens;
}

/** 弧(SVG の A)を 3 次ベジェにする(SVG 1.1 の付録 F.6 の中心の求め方)。 */
function arcToCubics(x1, y1, rx, ry, phiDeg, largeArc, sweep, x2, y2) {
  if (rx === 0 || ry === 0) {
    return [[x1, y1, x2, y2, x2, y2]];
  }
  const phi = (phiDeg * Math.PI) / 180;
  const cos = Math.cos(phi);
  const sin = Math.sin(phi);
  const dx = (x1 - x2) / 2;
  const dy = (y1 - y2) / 2;
  const x1p = cos * dx + sin * dy;
  const y1p = -sin * dx + cos * dy;
  rx = Math.abs(rx);
  ry = Math.abs(ry);
  const lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
  if (lambda > 1) {
    rx *= Math.sqrt(lambda);
    ry *= Math.sqrt(lambda);
  }
  const num = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p;
  const den = rx * rx * y1p * y1p + ry * ry * x1p * x1p;
  let coef = Math.sqrt(Math.max(0, num / den));
  if (largeArc === sweep) {
    coef = -coef;
  }
  const cxp = (coef * rx * y1p) / ry;
  const cyp = (-coef * ry * x1p) / rx;
  const cx = cos * cxp - sin * cyp + (x1 + x2) / 2;
  const cy = sin * cxp + cos * cyp + (y1 + y2) / 2;
  const angle = (ux, uy, vx, vy) => {
    const a = Math.atan2(ux * vy - uy * vx, ux * vx + uy * vy);
    return a;
  };
  const theta1 = angle(1, 0, (x1p - cxp) / rx, (y1p - cyp) / ry);
  let delta = angle((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx, (-y1p - cyp) / ry);
  if (!sweep && delta > 0) {
    delta -= 2 * Math.PI;
  } else if (sweep && delta < 0) {
    delta += 2 * Math.PI;
  }
  const segments = Math.ceil(Math.abs(delta) / (Math.PI / 2) - 1e-9);
  const step = delta / segments;
  const k = (4 / 3) * Math.tan(step / 4);
  const point = (t) => [
    cx + rx * Math.cos(t) * cos - ry * Math.sin(t) * sin,
    cy + rx * Math.cos(t) * sin + ry * Math.sin(t) * cos,
  ];
  const derivative = (t) => [
    -rx * Math.sin(t) * cos - ry * Math.cos(t) * sin,
    -rx * Math.sin(t) * sin + ry * Math.cos(t) * cos,
  ];
  const out = [];
  for (let i = 0; i < segments; i++) {
    const t0 = theta1 + i * step;
    const t1 = t0 + step;
    const [p0x, p0y] = point(t0);
    const [p1x, p1y] = point(t1);
    const [d0x, d0y] = derivative(t0);
    const [d1x, d1y] = derivative(t1);
    out.push([p0x + k * d0x, p0y + k * d0y, p1x - k * d1x, p1y - k * d1y, p1x, p1y]);
  }
  return out;
}

function pathOps(d) {
  const tokens = tokenize(d);
  const ops = [];
  let i = 0;
  let command = '';
  let x = 0;
  let y = 0;
  let startX = 0;
  let startY = 0;
  // 直前の 3 次・2 次ベジェの制御点(S・T の反射に使う)
  let lastCubic = null;
  let lastQuad = null;
  const num = () => {
    const v = tokens[i++];
    if (typeof v !== 'number') {
      throw new Error(`数が要る: ${d}`);
    }
    return v;
  };
  while (i < tokens.length) {
    if (typeof tokens[i] === 'string') {
      command = tokens[i++];
    } else if (command === 'M') {
      command = 'L';
    } else if (command === 'm') {
      command = 'l';
    }
    const rel = command === command.toLowerCase();
    const ox = rel ? x : 0;
    const oy = rel ? y : 0;
    let cubic = null;
    let quad = null;
    switch (command.toUpperCase()) {
      case 'M':
        x = ox + num();
        y = oy + num();
        startX = x;
        startY = y;
        ops.push([MOVE, x, y]);
        break;
      case 'L':
        x = ox + num();
        y = oy + num();
        ops.push([LINE, x, y]);
        break;
      case 'H':
        x = ox + num();
        ops.push([LINE, x, y]);
        break;
      case 'V':
        y = oy + num();
        ops.push([LINE, x, y]);
        break;
      case 'C': {
        const c = [ox + num(), oy + num(), ox + num(), oy + num(), ox + num(), oy + num()];
        ops.push([CUBIC, ...c]);
        cubic = [c[2], c[3]];
        x = c[4];
        y = c[5];
        break;
      }
      case 'S': {
        const c1 = lastCubic ? [2 * x - lastCubic[0], 2 * y - lastCubic[1]] : [x, y];
        const c = [ox + num(), oy + num(), ox + num(), oy + num()];
        ops.push([CUBIC, c1[0], c1[1], ...c]);
        cubic = [c[0], c[1]];
        x = c[2];
        y = c[3];
        break;
      }
      case 'Q':
      case 'T': {
        const q =
          command.toUpperCase() === 'Q'
            ? [ox + num(), oy + num()]
            : lastQuad
              ? [2 * x - lastQuad[0], 2 * y - lastQuad[1]]
              : [x, y];
        const ex = ox + num();
        const ey = oy + num();
        ops.push([CUBIC, x + (2 / 3) * (q[0] - x), y + (2 / 3) * (q[1] - y), ex + (2 / 3) * (q[0] - ex), ey + (2 / 3) * (q[1] - ey), ex, ey]);
        quad = q;
        x = ex;
        y = ey;
        break;
      }
      case 'A': {
        const rx = num();
        const ry = num();
        const phi = num();
        const large = num() !== 0;
        const sweep = num() !== 0;
        const ex = ox + num();
        const ey = oy + num();
        for (const c of arcToCubics(x, y, rx, ry, phi, large, sweep, ex, ey)) {
          ops.push([CUBIC, ...c]);
        }
        x = ex;
        y = ey;
        break;
      }
      case 'Z':
        ops.push([CLOSE]);
        x = startX;
        y = startY;
        break;
      default:
        throw new Error(`読めない命令 ${command}: ${d}`);
    }
    lastCubic = cubic;
    lastQuad = quad;
  }
  return ops;
}

// ---- 図形(rect・circle・line)を path にする ----

function rectPath({ x = 0, y = 0, width, height, rx, ry }) {
  [x, y, width, height] = [x, y, width, height].map(Number);
  let rX = rx !== undefined ? Number(rx) : ry !== undefined ? Number(ry) : 0;
  let rY = ry !== undefined ? Number(ry) : rX;
  rX = Math.min(rX, width / 2);
  rY = Math.min(rY, height / 2);
  if (rX === 0 || rY === 0) {
    return `M${x} ${y}H${x + width}V${y + height}H${x}Z`;
  }
  return (
    `M${x + rX} ${y}H${x + width - rX}A${rX} ${rY} 0 0 1 ${x + width} ${y + rY}` +
    `V${y + height - rY}A${rX} ${rY} 0 0 1 ${x + width - rX} ${y + height}` +
    `H${x + rX}A${rX} ${rY} 0 0 1 ${x} ${y + height - rY}` +
    `V${y + rY}A${rX} ${rY} 0 0 1 ${x + rX} ${y}Z`
  );
}

function circlePath({ cx, cy, r }) {
  [cx, cy, r] = [cx, cy, r].map(Number);
  return `M${cx + r} ${cy}A${r} ${r} 0 0 1 ${cx - r} ${cy}A${r} ${r} 0 0 1 ${cx + r} ${cy}Z`;
}

function shapeOps([tag, attrs]) {
  switch (tag) {
    case 'path':
      return pathOps(attrs.d);
    case 'rect':
      return pathOps(rectPath(attrs));
    case 'circle':
      return pathOps(circlePath(attrs));
    case 'line':
      return pathOps(`M${attrs.x1} ${attrs.y1}L${attrs.x2} ${attrs.y2}`);
    default:
      throw new Error(`扱えない図形 ${tag}`);
  }
}

// ---- 書く ----

const enumName = (name) => name.toUpperCase().replace(/-/g, '_');
const number = (v) => {
  const r = Math.round(v * 1000) / 1000;
  return Object.is(r, -0) ? '0' : String(r);
};

function main() {
  const dir = process.argv[2];
  if (!dir) {
    throw new Error('使い方: node tools/icons/generate.mjs <lucide-static の package フォルダ>');
  }
  const pkg = JSON.parse(readFileSync(join(dir, 'package.json'), 'utf8'));
  if (pkg.name !== 'lucide-static' || pkg.version !== VERSION) {
    throw new Error(`lucide-static ${VERSION} を渡す(渡したもの: ${pkg.name} ${pkg.version})`);
  }
  const nodes = JSON.parse(readFileSync(join(dir, 'icon-nodes.json'), 'utf8'));
  const root = resolve(dirname(fileURLToPath(import.meta.url)), '..', '..');
  const note = `// tools/icons/generate.mjs が lucide-static ${VERSION}(ISC。一部は Feather の MIT。THIRD_PARTY_LICENSES.txt)から生成した。手で直さない。`;

  const enumLines = [];
  const designerIcons = [];
  const dataLines = [];
  const tableLines = [];
  for (const [category, names] of ICONS) {
    enumLines.push(`    // ${category}`);
    for (const name of names) {
      const shapes = nodes[name];
      if (!shapes) {
        throw new Error(`${name} は lucide-static ${VERSION} の正式な名前ではない`);
      }
      const ops = shapes.flatMap(shapeOps);
      const flat = ops.flatMap(([op, ...pts]) => [String(op), ...pts.map(number)]);
      const id = `ICON_${enumName(name)}`;
      enumLines.push(`    ${enumName(name)},  ///< ${name}`);
      dataLines.push(`const float ${id}[] = {${flat.join(', ')}};`);
      tableLines.push(`    {${id}, sizeof(${id}) / sizeof(${id}[0])},`);
      // デザイナーの見本に出す SVG のパス(同じ線画)
      const d = ops
        .map(([op, ...pts]) => `${['M', 'L', 'C', 'Z'][op]}${pts.map(number).join(' ')}`)
        .join('');
      designerIcons.push({
        name: enumName(name).toLowerCase(),
        lucide: name,
        category: CATEGORY_EN[category],
        path: d,
      });
    }
  }

  const header = `/// @file
/// よく使うアイコン(docs/adr/0054)。Lucide(https://lucide.dev/)の線画を、24 × 24 の格子のパスとして持つ。
///
/// @code
/// action_save->icon = tokotachi::Image::icon(tokotachi::Icon::SAVE);
/// @endcode
///
/// Image::icon で画像にする。描くときは、部品の文字の色で塗り、描く大きさに合わせて線を引く(ぼやけない)。
/// 名前は Lucide の名前を大文字にし、- を _ にしたもの(folder-open → FOLDER_OPEN)。
${note}
#ifndef TOKOTACHI_GRAPHICS_ICON_HPP
#define TOKOTACHI_GRAPHICS_ICON_HPP

namespace tokotachi
{

enum class Icon
{
${enumLines.join('\n')}
};

}  // namespace tokotachi

#endif
`;
  const source = `${note}
#include "icon_data.hpp"

namespace tokotachi
{
namespace detail
{

namespace
{

// 命令(0 移動・1 直線・2 3 次ベジェ・3 閉じる)と、その点(24 × 24 の格子)
${dataLines.join('\n')}

}  // namespace

const IconShape ICON_SHAPES[] = {
${tableLines.join('\n')}
};

const int ICON_COUNT = static_cast<int>(sizeof(ICON_SHAPES) / sizeof(ICON_SHAPES[0]));

}  // namespace detail
}  // namespace tokotachi
`;
  // ギャラリーのサンプル(examples/gallery/icons.cpp)が出すアイコンの名前
  const names = ICONS.flatMap(([, list]) => list);
  const galleryNames = `${note.replace('// ', '// ギャラリーのサンプルが出すアイコンの名前。')}
#ifndef TOKOTACHI_EXAMPLES_ICON_NAMES_HPP
#define TOKOTACHI_EXAMPLES_ICON_NAMES_HPP

const char* const ICON_NAMES[] = {
${names.map((n) => `    "${n}",`).join('\n')}
};

#endif
`;
  writeFileSync(join(root, 'examples', 'gallery', 'icon_names.hpp'), galleryNames);
  // デザイナー(定義ファイルの "icon:<name>"。並びは列挙の順)
  const json = { lucide: VERSION, icons: designerIcons };
  writeFileSync(
    join(root, 'designer', 'packages', 'core', 'src', 'dsl', 'icons.json'),
    `${JSON.stringify(json, null, 2)}\n`,
  );
  writeFileSync(join(root, 'include', 'tokotachi', 'graphics', 'icon.hpp'), header);
  writeFileSync(join(root, 'src', 'graphics', 'icon_data.cpp'), source);
  console.log(`アイコン ${String(tableLines.length)} 個を書いた`);
}

main();
