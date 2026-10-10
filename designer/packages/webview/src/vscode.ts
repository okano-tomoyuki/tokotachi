import type {
  ExtensionToWebviewMessage,
  WebviewToExtensionMessage,
} from '@tokotachi-designer/core';

// acquireVsCodeApi は 1 つの Webview につき 1 回しか呼べないので、モジュールで 1 度だけ取る
const vscode = acquireVsCodeApi();

export function postMessage(message: WebviewToExtensionMessage): void {
  vscode.postMessage(message);
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
