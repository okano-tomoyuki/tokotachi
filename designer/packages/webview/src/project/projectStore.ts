/**
 * プロジェクトの設定画面の状態(Zustand)。プロジェクトファイルは拡張から届いたテキストを読んだもので、編集は書き換えた
 * プロジェクト全体を拡張に送り、TextDocument が変わったら届く新しいテキストで置き換わる(デザイナーと同じ)。
 */
import {
  parseProject,
  type CanvasFrame,
  type Diagnostic,
  type ExtensionToWebviewMessage,
  type TtprojDocument,
} from '@tokotachi-designer/core';
import { create } from 'zustand';
import { postMessage } from '../vscode.ts';

export interface ProjectState {
  readonly version: number;
  readonly fileName: string;
  /** 形の検証を通ったプロジェクト(通らなければ undefined)。 */
  readonly project: TtprojDocument | undefined;
  readonly diagnostics: readonly Diagnostic[];
  /** プロジェクトに加えられるウィンドウ(プロジェクトのフォルダからの相対パス)。 */
  readonly candidates: readonly string[];
  /** 登録したテーマを参照している部品の数。 */
  readonly presetUsage: Readonly<Record<string, number>>;
  /** 部品ファイル(メインのウィンドウにできない)。 */
  readonly components: readonly string[];
  /** 編集しているテーマ(登録したテーマの名前。null ならプロジェクトのテーマ)。 */
  readonly themeTarget: string | null;
  /** テーマの見本を描いた結果。 */
  readonly preview:
    | {
        readonly target: string | null;
        readonly light?: CanvasFrame | undefined;
        readonly dark?: CanvasFrame | undefined;
        readonly error?: string | undefined;
      }
    | undefined;
  /** 最後の編集の失敗(しばらく表示する)。 */
  readonly editError: string | undefined;
}

export const useProject = create<ProjectState>()(() => ({
  version: -1,
  fileName: '',
  project: undefined,
  diagnostics: [],
  candidates: [],
  presetUsage: {},
  components: [],
  themeTarget: null,
  preview: undefined,
  editError: undefined,
}));

let nextRequestId = 1;

/** プロジェクトを書き換える(change は今のプロジェクトから新しいものを作る)。 */
export function editProject(change: (project: TtprojDocument) => TtprojDocument): void {
  const project = useProject.getState().project;
  if (project) {
    postMessage({ type: 'editProject', requestId: nextRequestId++, project: change(project) });
  }
}

/** 登録したテーマを改名する(to が null なら消す)。ウィンドウの参照も拡張が書き換える。 */
export function renamePreset(from: string, to: string | null): void {
  postMessage({ type: 'renamePreset', requestId: nextRequestId++, from, to });
  if (useProject.getState().themeTarget === from) {
    selectTheme(to);
  }
}

/** 編集するテーマを選び、その見本を描いてもらう。 */
export function selectTheme(target: string | null): void {
  useProject.setState({ themeTarget: target });
  postMessage({ type: 'previewTheme', target });
}

let errorTimer: ReturnType<typeof setTimeout> | undefined;

export function handleProjectMessage(message: ExtensionToWebviewMessage): void {
  switch (message.type) {
    case 'document': {
      const { project, diagnostics } = parseProject(message.text);
      useProject.setState((s) => {
        // 編集していたテーマが無くなった(テキストで消した・改名した)ら、プロジェクトのテーマに戻す
        const target =
          s.themeTarget !== null && !Object.hasOwn(project?.theme?.presets ?? {}, s.themeTarget)
            ? null
            : s.themeTarget;
        if (target !== s.themeTarget) {
          postMessage({ type: 'previewTheme', target });
        }
        return {
          version: message.version,
          fileName: message.fileName,
          project,
          diagnostics,
          themeTarget: target,
        };
      });
      break;
    }
    case 'projectInfo':
      useProject.setState({
        candidates: message.candidates,
        presetUsage: message.presetUsage,
        components: message.components,
      });
      break;
    case 'themePreview':
      useProject.setState({
        preview: {
          target: message.target,
          light: message.light,
          dark: message.dark,
          error: message.error,
        },
      });
      break;
    case 'editResult':
      if (!message.ok) {
        useProject.setState({ editError: message.error });
        clearTimeout(errorTimer);
        errorTimer = setTimeout(() => {
          useProject.setState({ editError: undefined });
        }, 6000);
      }
      break;
    default:
      break;
  }
}
