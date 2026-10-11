import { hasErrors, l10n } from '@tokotachi-designer/core';
import { useEffect } from 'react';
import { duplicateSelection, installClipboard } from './clipboard.ts';
import { AlignBar } from './components/AlignBar.tsx';
import { CanvasView } from './components/CanvasView.tsx';
import { ContextMenu } from './components/ContextMenu.tsx';
import { GhostView } from './components/GhostView.tsx';
import { Inspector } from './components/Inspector.tsx';
import { Problems } from './components/Problems.tsx';
import { Sidebar } from './components/Sidebar.tsx';
import { ZoomControl } from './components/ZoomControl.tsx';
import { selectedNode, useDesigner } from './store.ts';
import { canRemove, moveSelected, removeSelected } from './treeOps.ts';
import { postMessage } from './vscode.ts';

export function App() {
  const selected = useDesigner(selectedNode);
  useEffect(() => {
    // Delete キーで消し、Alt+↑・↓ で同じ並びの中を動かす(入力欄の中では何もしない)
    const key = (e: KeyboardEvent): void => {
      const t = e.target as HTMLElement | null;
      if (t && ['INPUT', 'TEXTAREA', 'SELECT'].includes(t.tagName)) {
        return;
      }
      if (e.key === 'Delete') {
        removeSelected();
      } else if (
        (e.ctrlKey || e.metaKey) &&
        !e.shiftKey &&
        !e.altKey &&
        e.key.toLowerCase() === 'd'
      ) {
        e.preventDefault();
        duplicateSelection();
      } else if (e.altKey && (e.key === 'ArrowUp' || e.key === 'ArrowDown')) {
        e.preventDefault();
        moveSelected(e.key === 'ArrowUp' ? -1 : 1);
      }
    };
    window.addEventListener('keydown', key);
    const uninstall = installClipboard();
    return () => {
      window.removeEventListener('keydown', key);
      uninstall();
    };
  }, []);
  const document = useDesigner((s) => s.document);
  const diagnostics = useDesigner((s) => s.diagnostics);
  const editError = useDesigner((s) => s.editError);
  const version = useDesigner((s) => s.version);
  const sidebarHidden = useDesigner((s) => s.sidebar.hidden);
  const setSidebar = useDesigner((s) => s.setSidebar);

  if (version < 0) {
    return <div className="message">{l10n.t('Loading...')}</div>;
  }
  if (!document) {
    // JSON として読めない・形が違うときは編集できない(テキストで直してもらう)
    return (
      <div className="message">
        <p>
          {l10n.t(
            'The window definition cannot be shown in the designer because its content is invalid.',
          )}
        </p>
        <Problems />
        <button
          onClick={() => {
            postMessage({ type: 'openAsText' });
          }}
        >
          {l10n.t('Open as Text')}
        </button>
      </div>
    );
  }
  return (
    <div className="designer">
      <div className="toolbar">
        <button
          className={`toggle${sidebarHidden ? '' : ' on'}`}
          title={
            sidebarHidden
              ? l10n.t('Show the palette and structure')
              : l10n.t('Hide the palette and structure')
          }
          onClick={() => {
            setSidebar({ hidden: !sidebarHidden });
          }}
        >
          ◧
        </button>
        <button
          title={l10n.t('Generate the C++ code')}
          disabled={hasErrors(diagnostics)}
          onClick={() => {
            postMessage({ type: 'generateCode' });
          }}
        >
          {l10n.t('Generate Code')}
        </button>
        <button
          onClick={() => {
            postMessage({ type: 'openAsText' });
          }}
        >
          {l10n.t('Open as Text')}
        </button>
        <button
          title={l10n.t('Delete the selected item (Delete)')}
          disabled={!canRemove(selected)}
          onClick={removeSelected}
        >
          {l10n.t('Delete')}
        </button>
        <AlignBar />
        <ZoomControl />
        {editError !== undefined && <span className="edit-error">{editError}</span>}
      </div>
      <GhostView />
      <ContextMenu />
      <div className="panes">
        <Sidebar />
        <div className="pane canvas-pane">
          <CanvasView />
          <Problems />
        </div>
        <div className="pane inspector-pane">
          <Inspector />
        </div>
      </div>
    </div>
  );
}
