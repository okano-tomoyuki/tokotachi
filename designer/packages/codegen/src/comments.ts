/**
 * 生成するコードのコメント。UI の言語ではなくコード生成の設定(commentLocale)に従う(生成する人によって結果が変わらないように)。
 */
import type { CommentLocale } from './names.ts';

export interface GeneratedComments {
  /** 生成するクラスの説明。 */
  readonly classDoc: (sourceName: string) => string;
  /** tokotachi_setup_ui の説明。 */
  readonly setupUi: string;
  /** 埋め込んだ画像の説明。 */
  readonly images: string;
}

const COMMENTS: Readonly<Record<CommentLocale, GeneratedComments>> = {
  en: {
    classDoc: (sourceName) =>
      `Window created with the Tokotachi designer (${sourceName}). Regions enclosed in markers are overwritten when regenerated.`,
    setupUi:
      'Creates the widgets, sets their properties, adds them to the layouts and connects the handlers (generated).',
    images: 'Contents of the image files referenced by the window definition (generated).',
  },
  // l10n-ignore(生成するコードのコメントの定型文)
  ja: {
    classDoc: (sourceName) =>
      // l10n-ignore
      `デザイナーで作ったウィンドウ(${sourceName})。マーカーで囲んだ区間は、生成し直すと上書きされる。`,
    // l10n-ignore
    setupUi: '部品を作り、プロパティを設定し、レイアウトに入れ、ハンドラをつなぐ(生成したコード)。',
    // l10n-ignore
    images: '定義ファイルが参照する画像のファイルの中身(生成したコード)。',
  },
};

export function generatedComments(locale: CommentLocale): GeneratedComments {
  return COMMENTS[locale];
}
