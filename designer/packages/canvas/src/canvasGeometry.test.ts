/**
 * 範囲で囲んで選ぶ部品と、入れ物の外にはみ出した部品(webview の canvasGeometry)を、実際のキャンバスの描画の結果で確かめる。
 * WebAssembly のモジュールが無ければ(pnpm canvas:build の前)飛ばす。
 */
import { outline, type TtuiDocument } from '@tokotachi-designer/core';
import { beforeAll, describe, expect, it } from 'vitest';
import { bandContainer, bandSelection, outsideParent } from '../../webview/src/canvasGeometry.ts';
import { Canvas, isBuilt, type RenderResult } from './index.ts';

/** 自由な配置のウィンドウ。枠の中に 2 つ、枠の外に 2 つ。button_out は枠から、button_far はウィンドウからはみ出している。 */
const DOC = {
  format_version: 1,
  window: {
    name: 'FreeWindow',
    class: 'Window',
    properties: { width: 400, height: 300 },
    children: [
      {
        name: 'group_box_main',
        class: 'GroupBox',
        properties: { x: 10, y: 10, width: 200, height: 150, text: 'Main' },
        children: [
          {
            name: 'button_a',
            class: 'Button',
            properties: { x: 10, y: 30, width: 80, height: 30 },
          },
          {
            name: 'button_out',
            class: 'Button',
            properties: { x: 150, y: 100, width: 100, height: 30 },
          },
        ],
      },
      {
        name: 'button_c',
        class: 'Button',
        properties: { x: 250, y: 20, width: 80, height: 30 },
      },
      {
        name: 'button_far',
        class: 'Button',
        properties: { x: 350, y: 250, width: 100, height: 30 },
      },
    ],
  },
} as unknown as TtuiDocument;

describe.skipIf(!isBuilt())('canvasGeometry', () => {
  let frame: RenderResult;
  beforeAll(async () => {
    frame = (await Canvas.load({ language: 'en' })).render(DOC, { placeholders: true });
  });
  const roots = () => outline(DOC);
  const names = (ids: readonly string[]) =>
    ids.map((id) => id.split('/').reduce<any>((n, k) => n?.[k], DOC)?.name);

  it('ウィンドウの空いた所から囲むと、ウィンドウの直接の部品のうち掛かったものを選ぶ', () => {
    const container = bandContainer(roots(), undefined);
    expect(container).toBe('window');
    const ids = bandSelection(roots(), frame.rects, container, {
      x: 100,
      y: 0,
      width: 200,
      height: 40,
    });
    // 枠(10〜210)と button_c(250〜330)に掛かる。枠の中の button_a は選ばない
    expect(names(ids)).toEqual(['group_box_main', 'button_c']);
  });

  it('部品を入れた部品の中から囲むと、その中の部品を選ぶ', () => {
    const group = roots()[0]?.children.find((n) => n.label === 'group_box_main');
    const container = bandContainer(roots(), group);
    expect(container).toBe(group?.id);
    const ids = bandSelection(roots(), frame.rects, container, {
      x: 0,
      y: 0,
      width: 400,
      height: 300,
    });
    expect(names(ids)).toEqual(['button_a', 'button_out']);
  });

  it('入れ物(枠・ウィンドウ)からはみ出した部品を見つける', () => {
    const window = {
      x: 0,
      y: 0,
      width: frame.width / frame.scale,
      height: frame.height / frame.scale,
    };
    const outside = outsideParent(roots(), frame.rects, window);
    expect(outside.map((o) => [names([o.id])[0], o.parent])).toEqual([
      ['button_out', 'group_box_main'],
      ['button_far', 'FreeWindow'],
    ]);
  });
});
