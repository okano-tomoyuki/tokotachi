/**
 * キャンバスの描画(docs/adr/0039・0040)。拡張の中で WebAssembly の Tokotachi を 1 つ読み込み、どのデザイナーからも使う。
 * 描画は同期的に行う(1 回 10 ms ほど)ので、編集が続いたときは最後の内容だけを描く。
 */
import { Canvas } from '@tokotachi-designer/canvas';
import type { ComponentResolver } from '@tokotachi-designer/codegen';
import {
  withCustomClasses,
  type CanvasFrame,
  type ClassInfo,
  type ThemePresets,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';

let loading: Promise<Canvas> | undefined;

function load(context: vscode.ExtensionContext): Promise<Canvas> {
  loading ??= Canvas.load({
    wasmDir: vscode.Uri.joinPath(context.extensionUri, 'dist', 'canvas').fsPath,
  });
  return loading;
}

/**
 * @param classes 自作部品(docs/adr/0044)。描く間だけカタログに足す
 */
export async function renderCanvas(
  classes: Readonly<Record<string, ClassInfo>>,
  context: vscode.ExtensionContext,
  doc: TtuiDocument,
  version: number,
  colorScheme: 'LIGHT' | 'DARK',
  tabs: Readonly<Record<string, number>>,
  images: ReadonlyMap<string, Uint8Array>,
  themeOverride?: Readonly<Record<string, unknown>>,
  themePresets?: ThemePresets,
  components?: ComponentResolver,
): Promise<CanvasFrame> {
  const canvas = await load(context);
  let r;
  try {
    r = withCustomClasses(classes, () =>
      canvas.render(doc, {
        colorScheme,
        placeholders: true,
        tabs,
        images,
        themeOverride,
        themePresets,
        components,
      }),
    );
  } catch (e) {
    // WebAssembly の中で止まると、その状態は壊れていることがあるので捨て、次の描画で読み込み直す
    // (一度の失敗で、以後どのファイルも描けなくなるのを防ぐ)
    loading = undefined;
    throw e;
  }
  if (r.errors.length > 0) {
    console.warn(`Tokotachi Designer: ${r.errors.join(', ')}`);
  }
  return {
    version,
    width: r.width,
    height: r.height,
    scale: r.scale,
    pixels: r.pixels,
    rects: r.rects,
  };
}
