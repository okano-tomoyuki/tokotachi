import { configureL10n, valueAt, type L10nBundle } from '@tokotachi-designer/core';
import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { App } from './App.tsx';
import { startProject } from './project/ProjectApp.tsx';
import { DEFAULT_SIDEBAR, handleMessage, selectedNode, useDesigner } from './store.ts';
import { loadState, onMessage, postMessage, saveState } from './vscode.ts';
import './style.css';

const container = document.getElementById('root');
if (!container) {
  throw new Error('#root not found');
}

// 拡張が HTML に埋め込んだ訳を、描画の前に読む
configureL10n(
  container.dataset['language'] ?? 'en',
  JSON.parse(container.dataset['l10n'] ?? '{}') as L10nBundle,
);

if (container.dataset['view'] === 'project') {
  // プロジェクトの設定画面(*.ttproj.json)
  startProject(container);
} else {
  startDesigner(container);
}
postMessage({ type: 'ready' });

/** ウィンドウのデザイナー(*.ttui.json)。 */
function startDesigner(root: HTMLElement): void {
  onMessage(handleMessage);

  // 倍率と左のペインの配置は Webview の状態に保存し、開き直したときに戻す
  const { zoom, sidebar } = loadState();
  if (typeof zoom === 'number' && zoom >= 0.5 && zoom <= 2) {
    useDesigner.setState({ zoom });
  }
  if (sidebar) {
    useDesigner.setState({ sidebar: { ...DEFAULT_SIDEBAR, ...sidebar } });
  }
  useDesigner.subscribe((state, previous) => {
    if (state.zoom !== previous.zoom || state.sidebar !== previous.sidebar) {
      saveState({ ...loadState(), zoom: state.zoom, sidebar: state.sidebar });
    }
  });

  // 選んだ部品が入っているタブを、キャンバスで表示する(定義ファイルは変えない)
  let lastTabs = '{}';
  useDesigner.subscribe((state, previous) => {
    if (state.selection === previous.selection && state.document === previous.document) {
      return;
    }
    const node = selectedNode(state);
    const tabs: Record<string, number> = {};
    const path = node?.path ?? [];
    path.forEach((key, i) => {
      const index = path[i + 1];
      if (key === 'tabs' && typeof index === 'number') {
        const owner = valueAt(state.document, path.slice(0, i)) as { name?: string } | undefined;
        if (owner?.name !== undefined) {
          tabs[owner.name] = index;
        }
      }
    });
    const text = JSON.stringify(tabs);
    if (text !== lastTabs) {
      lastTabs = text;
      postMessage({ type: 'view', tabs });
    }
  });
  createRoot(root).render(
    <StrictMode>
      <App />
    </StrictMode>,
  );
}
