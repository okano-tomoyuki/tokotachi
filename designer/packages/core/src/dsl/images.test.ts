import { describe, expect, it } from 'vitest';
import { SAMPLE, sampleCopy } from '../testing.ts';
import { imageReferences } from './images.ts';
import { TtuiDocument } from './schema.ts';

describe('imageReferences', () => {
  it('画像のプロパティを、値とその位置で並べる', () => {
    expect(imageReferences(SAMPLE)).toEqual([
      { path: 'icons/open.png', at: ['actions', 0, 'properties', 'icon'] },
    ]);
  });

  it('文字列でない値(検証のエラー)は除く', () => {
    const doc = sampleCopy();
    doc.actions[0].properties.icon = 1;
    doc.actions[3].properties.icon = 'icons/quit.png';
    expect(imageReferences(TtuiDocument.parse(doc)).map((r) => r.path)).toEqual(['icons/quit.png']);
  });
});
