/**
 * キャンバスの描画(docs/adr/0039・0040)。拡張の中で WebAssembly の Tokotachi を 1 つ読み込み、どのデザイナーからも使う。
 * 描画は同期的に行う(1 回 10 ms ほど)ので、編集が続いたときは最後の内容だけを描く。
 */
import { Canvas } from '@tokotachi-designer/canvas';
import type { CanvasFrame, TtuiDocument } from '@tokotachi-designer/core';
import * as vscode from 'vscode';

let loading: Promise<Canvas> | undefined;

function load(context: vscode.ExtensionContext): Promise<Canvas> {
  loading ??= Canvas.load({
    wasmDir: vscode.Uri.joinPath(context.extensionUri, 'dist', 'canvas').fsPath,
  });
  return loading;
}

export async function renderCanvas(
  context: vscode.ExtensionContext,
  doc: TtuiDocument,
  version: number,
  colorScheme: 'LIGHT' | 'DARK',
  tabs: Readonly<Record<string, number>>,
  images: ReadonlyMap<string, Uint8Array>,
  themeOverride?: Readonly<Record<string, unknown>>,
): Promise<CanvasFrame> {
  const canvas = await load(context);
  const r = canvas.render(doc, { colorScheme, placeholders: true, tabs, images, themeOverride });
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
