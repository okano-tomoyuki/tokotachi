import { describe, expect, it } from 'vitest';
import { isImagePath } from '../dsl/images.ts';
import { relativePath } from './paths.ts';

describe('relativePath', () => {
  it('フォルダから下・上のファイルへの相対パス', () => {
    expect(relativePath('/a/b', '/a/b/icons/open.png')).toBe('icons/open.png');
    expect(relativePath('/a/b', '/a/c/open.png')).toBe('../c/open.png');
  });

  it('Windows のドライブの大文字・小文字は区別しない(定義ファイルとファイルを選ぶダイアログで違うことがある)', () => {
    expect(relativePath('/c:/work/app', '/C:/work/app/icons/open.png')).toBe('icons/open.png');
    expect(relativePath('/C:/work/app', '/c:/work/shared/open.png')).toBe('../shared/open.png');
    // ドライブが違えば共通の部分は無い
    expect(relativePath('/c:/work', '/d:/work/open.png')).toBe('../../d:/work/open.png');
  });
});

describe('isImagePath', () => {
  it('定義ファイルからの相対パスだけを画像のパスにする(ほかのドライブを通るパスは不可)', () => {
    expect(isImagePath('icons/open.png')).toBe(true);
    expect(isImagePath('../shared/open.png')).toBe(true);
    expect(isImagePath('C:/icons/open.png')).toBe(false);
    expect(isImagePath('../../d:/icons/open.png')).toBe(false);
  });
});
