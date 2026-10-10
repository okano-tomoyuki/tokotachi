import { configureL10n, type L10nBundle } from '@tokotachi-designer/core';
import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { App } from './App.tsx';
import { handleMessage } from './store.ts';
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
createRoot(container).render(
  <StrictMode>
    <App />
  </StrictMode>,
);
postMessage({ type: 'ready' });
