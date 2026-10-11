/**
 * キャンバスの描画の部分(docs/adr/0039)。WebAssembly にした Tokotachi(designer/canvas)を Node.js から使う。
 *
 * 定義ファイルから「部品を組む操作の並び」(codegen の buildSetup)を作り、WebAssembly の Tokotachi で実行して描く。
 * 生成したコードも同じ並びから作るので、描いた画面は生成したコードが組む画面と同じになる。
 * WebAssembly のモジュールは `pnpm canvas:build` で wasm/ に作る(Emscripten が要る)。
 */
import { existsSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { type ComponentResolver, type Op } from '@tokotachi-designer/codegen';
import {
  isImageType,
  normalizePath,
  resolveThemePresets,
  type ThemePresets,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import { canvasSetup } from './components.ts';
import { systemFonts } from './fonts.ts';
import { withPlaceholders } from './placeholders.ts';

export { systemFonts } from './fonts.ts';
export { CUSTOM_PLACEHOLDER_SIZE, PLACEHOLDER_SIZE, withPlaceholders } from './placeholders.ts';
export { CUSTOM_PLACEHOLDER_BASES, substituteCustomClasses } from './custom.ts';

/** 部品の位置と大きさ(ウィンドウの座標。dp)。 */
export interface WidgetRect {
  readonly x: number;
  readonly y: number;
  readonly width: number;
  readonly height: number;
  /** 表示されているか(選んでいないタブの中身などは false)。 */
  readonly showing: boolean;
}

export interface RenderResult {
  /** 画像の大きさ(ピクセル)。 */
  readonly width: number;
  readonly height: number;
  /** dp からピクセルへの倍率。 */
  readonly scale: number;
  /** ウィンドウの大きさ(dp。Dialog は開いたときに中身に合わせた大きさ)。 */
  readonly windowWidth: number;
  readonly windowHeight: number;
  /** 画素(RGBA の並び。ImageData と同じ)。 */
  readonly pixels: Uint8Array;
  /** 変数(メンバ・一時的な変数)の名前 → 部品の位置と大きさ。 */
  readonly rects: Readonly<Record<string, WidgetRect>>;
  /** 実行できなかった操作(ふつうは無い)。 */
  readonly errors: readonly string[];
}

export interface ClassDefaults {
  readonly properties: Readonly<Record<string, unknown>>;
  /** 部品の望む大きさ(dp)。 */
  readonly preferred_size?: { readonly width: number; readonly height: number };
}

export type ColorScheme = 'LIGHT' | 'DARK';

interface Module {
  ccall(name: string, returnType: 'string' | null, argTypes: string[], args: unknown[]): unknown;
  HEAPU8: Uint8Array;
}

/** wasm/ の場所(既定はこのパッケージの wasm/)。拡張にまとめるときは、読み込むときに場所を渡す。 */
export function defaultWasmDir(): string {
  return join(dirname(fileURLToPath(import.meta.url)), '..', 'wasm');
}
const MODULE_FILE = 'tokotachi_canvas.mjs';

export function isBuilt(wasmDir: string = defaultWasmDir()): boolean {
  return existsSync(join(wasmDir, MODULE_FILE));
}

export class Canvas {
  private readonly module: Module;

  private constructor(module: Module) {
    this.module = module;
  }

  /**
   * モジュールを読み、フォントを設定する。
   * @param fonts 使うフォントのファイル(既定は OS のフォント)
   */
  static async load(
    options: { wasmDir?: string; fonts?: readonly string[] } = {},
  ): Promise<Canvas> {
    const dir = options.wasmDir ?? defaultWasmDir();
    const file = join(dir, MODULE_FILE);
    if (!existsSync(file)) {
      throw new Error(`${file} が無い。pnpm canvas:build で作る`);
    }
    const imported = (await import(pathToFileURL(file).href)) as {
      default: () => Promise<Module>;
    };
    const module = await imported.default();
    // ネイティブの Tokotachi が OS から得るもの(フォント・ダイアログのボタンの順)を渡す
    const config = {
      fonts: options.fonts ?? systemFonts(),
      os: process.platform === 'win32' ? 'windows' : process.platform,
      desktop: process.env['XDG_CURRENT_DESKTOP'] ?? '',
    };
    module.ccall('ttc_configure', null, ['string'], [JSON.stringify(config)]);
    return new Canvas(module);
  }

  /**
   * 検証を通った定義ファイルを描く。
   * @param options.placeholders 中身が空の入れ物に最小の大きさを与える(デザイナーで使う。placeholders.ts)
   * @param options.tabs 表示するタブ(TabWidget の名前 → 位置。デザイナーで、選んだ部品が入っているタブを出す)
   * @param options.images 定義ファイルが参照する画像の中身(正規化したパス → 中身)。無い画像は描かない
   * @param options.themeOverride アプリケーションのテーマに重ねる上書き(プロジェクトの theme.override。検証済みのもの)
   * @param options.themePresets プロジェクトに登録したテーマ(theme.presets)。部品の theme_override の preset を展開する
   * @param options.components 置いた部品ファイルを読む(docs/adr/0043)。読めない部品ファイルは空の Widget として描く
   */
  render(
    doc: TtuiDocument,
    options: {
      colorScheme?: ColorScheme;
      placeholders?: boolean;
      tabs?: Readonly<Record<string, number>>;
      images?: ReadonlyMap<string, Uint8Array>;
      themeOverride?: Readonly<Record<string, unknown>> | undefined;
      themePresets?: ThemePresets | undefined;
      components?: ComponentResolver | undefined;
    } = {},
  ): RenderResult {
    // プロジェクトに登録したテーマの参照は、その中身に展開して描く(生成するコードの <名前>_theme() と同じ)
    const resolved = resolveThemePresets(doc, options.themePresets);
    const target = options.placeholders === true ? withPlaceholders(resolved) : resolved;
    // タブは、すべての部品を組んだ後に選ぶ(タブが無いと効かない)
    const show = Object.entries(options.tabs ?? {}).map(([name, index]): Op => ({
      op: 'set',
      target: name,
      property: 'current_index',
      type: 'int',
      value: index,
    }));
    const setup = canvasSetup(target, options.components, options.themePresets);
    // 画像のプロパティの値は、正規化したパスにして、その中身と一緒に渡す
    const ops = setup.ops.map((op): Op =>
      op.op === 'set' && isImageType(op.type) && typeof op.value === 'string'
        ? { ...op, value: normalizePath(op.value) }
        : op,
    );
    const images: Record<string, string> = {};
    for (const [path, data] of options.images ?? []) {
      images[path] = Buffer.from(data).toString('base64');
    }
    const request = {
      window_class: setup.windowClass,
      color_scheme: options.colorScheme ?? 'LIGHT',
      ...(options.themeOverride !== undefined ? { theme_override: options.themeOverride } : {}),
      images,
      ops: [...ops, ...show],
    };
    const text = this.module.ccall('ttc_render', 'string', ['string'], [JSON.stringify(request)]);
    const r = JSON.parse(String(text)) as {
      error?: string;
      width: number;
      height: number;
      scale: number;
      window_width: number;
      window_height: number;
      pixels: number;
      rects: Record<string, WidgetRect>;
      errors: string[];
    };
    if (r.error !== undefined) {
      throw new Error(r.error);
    }
    // 次の描画で上書きされるので写す
    const pixels = this.module.HEAPU8.slice(r.pixels, r.pixels + r.width * r.height * 4);
    return {
      width: r.width,
      height: r.height,
      scale: r.scale,
      windowWidth: r.window_width,
      windowHeight: r.window_height,
      pixels,
      rects: r.rects,
      errors: r.errors,
    };
  }

  /** カタログの各クラスのプロパティの既定値と望む大きさ(カタログの既定値の測定)。 */
  defaults(): Record<string, ClassDefaults> {
    const text = this.module.ccall('ttc_defaults', 'string', [], []);
    return JSON.parse(String(text)) as Record<string, ClassDefaults>;
  }
}
