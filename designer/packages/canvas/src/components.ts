/**
 * キャンバスの部品ファイル(docs/adr/0043)。部品ファイルのクラスは WebAssembly の Tokotachi に無いので、置いた部品は Widget として作り、
 * 部品ファイルの操作の並びを展開する。部品ファイルそのものを開いたときは、隠れたウィンドウいっぱいに根の Widget を置いて描く。
 */
import {
  buildSetup,
  COMPONENT_ROOT_SKIP,
  inlineOps,
  type ComponentResolver,
  type Op,
} from '@tokotachi-designer/codegen';
import { substituteCustomClasses } from './custom.ts';
import {
  isComponentDocument,
  resolveThemePresets,
  type ThemePresets,
  type TtuiDocument,
} from '@tokotachi-designer/core';

/** 部品ファイルを開いたときに、根の Widget を入れるレイアウトの変数。 */
const ROOT_LAYOUT = 'tokotachi_component_layout';

/** 読んだ部品ファイルにも、プロジェクトに登録したテーマを展開する(入れ子の部品ファイルも)。 */
function withPresets(
  resolve: ComponentResolver | undefined,
  presets: ThemePresets | undefined,
): ComponentResolver | undefined {
  if (!resolve) {
    return undefined;
  }
  return (path) => {
    const c = resolve(path);
    return c
      ? {
          document: resolveThemePresets(c.document, presets),
          resolve: withPresets(c.resolve, presets),
        }
      : undefined;
  };
}

export interface CanvasSetup {
  /** WebAssembly の Tokotachi が作るウィンドウのクラス(Window・Dialog)。 */
  readonly windowClass: string;
  readonly ops: readonly Op[];
}

/**
 * 描く操作の並び。置いた部品ファイルは展開する。
 * @param doc 登録したテーマを展開した定義ファイル
 */
export function canvasSetup(
  doc: TtuiDocument,
  resolve: ComponentResolver | undefined,
  presets: ThemePresets | undefined,
): CanvasSetup {
  const options = { resolve: withPresets(resolve, presets), inline: true };
  const ops = substituteCustomClasses(buildSetup(doc, options).ops);
  if (!isComponentDocument(doc)) {
    return { windowClass: doc.window.class, ops };
  }
  // 部品ファイル: ウィンドウを根の描く大きさにし、いっぱいに根の Widget を置く
  const root = doc.window.name;
  const size = COMPONENT_ROOT_SKIP.flatMap((property): Op[] => {
    const value = doc.window.properties?.[property];
    return typeof value === 'number' ? [{ op: 'set', property, type: 'float', value }] : [];
  });
  return {
    windowClass: 'Window',
    ops: [
      ...size,
      { op: 'new', variable: ROOT_LAYOUT, class: 'BoxLayout', declare: true, parent: 'this' },
      { op: 'new', variable: root, class: 'Widget', declare: false, parent: 'this' },
      {
        op: 'call',
        target: ROOT_LAYOUT,
        method: 'add',
        args: [
          { kind: 'ref', name: root },
          { kind: 'int', value: 1 },
        ],
      },
      ...inlineOps(ops, root, (n) => n),
    ],
  };
}
