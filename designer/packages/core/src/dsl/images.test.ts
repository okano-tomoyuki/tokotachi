import { describe, expect, it } from 'vitest';
import { SAMPLE, sampleCopy } from '../testing.ts';
import { iconEnumName, iconOf, ICONS } from './icons.ts';
import { imageReferences } from './images.ts';
import { checkValue } from './values.ts';
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

describe('よく使うアイコン', () => {
  it('"icon:<名前>" は画像のファイルではないので、参照に入れない', () => {
    // 見本の action_quit は "icon:log_out"
    expect(imageReferences(SAMPLE).map((r) => r.path)).not.toContain('icon:log_out');
  });

  it('知っている名前なら値として正しく、知らない名前はエラーにする', () => {
    expect(checkValue({ value: 'Image' }, 'icon:folder_open')).toBeNull();
    expect(checkValue({ value: 'Image' }, 'icon:no_such_icon')).toContain('no_such_icon');
    expect(iconOf('icon:folder_open')).toMatchObject({ lucide: 'folder-open', category: 'File' });
    expect(iconEnumName(ICONS[0] ?? ({} as never))).toBe('FILE');
    // 並びはライブラリの列挙型 tokotachi::Icon の順(71 個。最後は MOON)
    expect(ICONS).toHaveLength(71);
    expect(ICONS[70]?.name).toBe('moon');
  });
});

describe('よく使うアイコン', () => {
  it('"icon:<名前>" は画像のファイルではないので、参照に入れない', () => {
    // 見本の action_quit は "icon:log_out"
    expect(imageReferences(SAMPLE).map((r) => r.path)).toEqual(['icons/open.png']);
  });

  it('知っている名前なら値として正しく、知らない名前はエラーにする', () => {
    expect(checkValue({ value: 'Image' }, 'icon:folder_open')).toBeNull();
    expect(checkValue({ value: 'Image' }, 'icon:no_such_icon')).toContain('no_such_icon');
    expect(iconOf('icon:folder_open')).toMatchObject({ lucide: 'folder-open', category: 'File' });
    expect(iconOf('icons/open.png')).toBeUndefined();
    const first = ICONS[0];
    expect(first && iconEnumName(first)).toBe('FILE');
    // 並びはライブラリの列挙型 tokotachi::Icon の順(71 個。最後は MOON)
    expect(ICONS).toHaveLength(71);
    expect(ICONS[70]?.name).toBe('moon');
  });
});
