/**
 * 拡張と Webview の間で交わすメッセージの型(docs/designer/editor-design.md §2。Bethany と同じ考え方)。
 * 両者はこの型を介してだけ通信し、互いのコードを直接 import しない。
 */
import type { EditCommand } from './edit/commands.ts';
import type { TtprojDocument } from './project/schema.ts';

/** キャンバスの部品の位置と大きさ(ウィンドウの座標。dp)。 */
export interface CanvasRect {
  readonly x: number;
  readonly y: number;
  readonly width: number;
  readonly height: number;
  readonly showing: boolean;
}

/** キャンバスの描画の結果(拡張の中の WebAssembly の Tokotachi が描く。docs/adr/0039・0040)。 */
export interface CanvasFrame {
  /** 描いたドキュメントの版(TextDocument の version)。 */
  readonly version: number;
  /** 画像の大きさ(ピクセル)と、dp からピクセルへの倍率。 */
  readonly width: number;
  readonly height: number;
  readonly scale: number;
  /** 画素(RGBA の並び)。 */
  readonly pixels: Uint8Array;
  /** 変数の名前 → 部品の位置と大きさ(構造の木の variable と結び付ける)。 */
  readonly rects: Readonly<Record<string, CanvasRect>>;
}

export type ExtensionToWebviewMessage =
  /** TextDocument の現在の内容。変更のたびに送る(テキストエディタでの編集・Undo・Redo を含む)。 */
  | {
      readonly type: 'document';
      readonly version: number;
      readonly text: string;
      readonly fileName: string;
    }
  /** キャンバスの描画の結果。document の後に届く(検証のエラーがあれば描かない)。 */
  | { readonly type: 'canvas'; readonly frame: CanvasFrame }
  /** ウィンドウが属するプロジェクトに登録したテーマの名前(属さなければ空)。描くたびに送る。 */
  | { readonly type: 'themePresets'; readonly names: readonly string[] }
  /** キャンバスを描けなかった理由(WebAssembly のモジュールが無い等)。 */
  | { readonly type: 'canvasError'; readonly message: string }
  /**
   * プロジェクトの設定画面(*.ttproj.json)へ: プロジェクトファイルの周りの情報。document の後に送る。
   * candidates はプロジェクトに加えられるウィンドウ(ワークスペースの定義ファイルのうち、まだ属さないもの。
   * プロジェクトのフォルダからの相対パス)、presetUsage は登録したテーマを参照している部品の数。
   */
  | {
      readonly type: 'projectInfo';
      readonly candidates: readonly string[];
      readonly presetUsage: Readonly<Record<string, number>>;
    }
  /** プロジェクトの設定画面へ: テーマの見本のウィンドウを描いた結果(ライトとダーク)。target は previewTheme のもの。 */
  | {
      readonly type: 'themePreview';
      readonly target: string | null;
      readonly light?: CanvasFrame;
      readonly dark?: CanvasFrame;
      readonly error?: string;
    }
  /** edit・editProject などの処理の結果。document の送信より後に届く。 */
  | {
      readonly type: 'editResult';
      readonly requestId: number;
      readonly ok: boolean;
      readonly error?: string;
    };

export type WebviewToExtensionMessage =
  | { readonly type: 'ready' }
  | { readonly type: 'edit'; readonly requestId: number; readonly command: EditCommand }
  /** コードを生成する。生成・書き込み・結果の通知は拡張が行う。 */
  | { readonly type: 'generateCode' }
  /** ハンドラの定義へ移動する(コードを生成してから開く)。直前に送った edit の適用の後に処理する。 */
  | { readonly type: 'goToHandler'; readonly handler: string }
  /** テキストエディタで開く。 */
  | { readonly type: 'openAsText' }
  /**
   * 画像のファイルを選んでもらい、ノードのプロパティに定義ファイルからの相対パスを設定する
   * (選ばなければ何もしない。設定は edit と同じく 1 回の変更)。
   */
  | {
      readonly type: 'pickImage';
      readonly target: readonly (string | number)[];
      readonly property: string;
    }
  /**
   * キャンバスの表示だけの設定(定義ファイルは変えない)。選んだ部品が入っているタブを表示する
   * (TabWidget の名前 → タブの位置)。
   */
  | { readonly type: 'view'; readonly tabs: Readonly<Record<string, number>> }
  // ---- プロジェクトの設定画面(*.ttproj.json)から ----
  /** プロジェクトファイルを、この内容に書き換える(決まった形で書き出し、変わった範囲だけを置き換える)。 */
  | { readonly type: 'editProject'; readonly requestId: number; readonly project: TtprojDocument }
  /**
   * 登録したテーマを改名する(to が文字列)か消す(to が null)。プロジェクトのウィンドウの参照(preset)も一緒に書き換える
   * (消すときは参照を外す。参照があれば確かめてから)。
   */
  | {
      readonly type: 'renamePreset';
      readonly requestId: number;
      readonly from: string;
      readonly to: string | null;
    }
  /** テーマの見本を描く(target は登録したテーマの名前。null ならプロジェクトのテーマそのもの)。 */
  | { readonly type: 'previewTheme'; readonly target: string | null }
  /** プロジェクトのコード(すべてのウィンドウと起動部分)を生成する。 */
  | { readonly type: 'generateProject' }
  /** プロジェクトのウィンドウをデザイナーで開く(プロジェクトのフォルダからの相対パス)。 */
  | { readonly type: 'openWindow'; readonly path: string };
