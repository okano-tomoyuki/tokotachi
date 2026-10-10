import type {
  ExtensionToWebviewMessage,
  WebviewToExtensionMessage,
} from '@tokotachi-designer/core';

// acquireVsCodeApi は 1 つの Webview につき 1 回しか呼べないので、モジュールで 1 度だけ取る
const vscode = acquireVsCodeApi();

export function postMessage(message: WebviewToExtensionMessage): void {
  vscode.postMessage(message);
}

/** Webview を隠して戻したとき・開き直したときに戻す、画面の状態。 */
export interface PersistedState {
  readonly zoom?: number;
}

export function loadState(): PersistedState {
  return (vscode.getState() as PersistedState | undefined) ?? {};
}

export function saveState(state: PersistedState): void {
  vscode.setState(state);
}

export function onMessage(listener: (message: ExtensionToWebviewMessage) => void): () => void {
  const handler = (event: MessageEvent<ExtensionToWebviewMessage>): void => {
    listener(event.data);
  };
  window.addEventListener('message', handler);
  return () => {
    window.removeEventListener('message', handler);
  };
}
