import { configureL10n, valueAt, type L10nBundle } from '@tokotachi-designer/core';
import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { App } from './App.tsx';
import { handleMessage, selectedNode, useDesigner } from './store.ts';
import { onMessage, postMessage } from './vscode.ts';
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

onMessage(handleMessage);

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
createRoot(container).render(
  <StrictMode>
    <App />
  </StrictMode>,
);
postMessage({ type: 'ready' });
