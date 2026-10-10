import { hasErrors, l10n, type OutlineNode } from '@tokotachi-designer/core';
import { CanvasView } from './components/CanvasView.tsx';
import { Inspector } from './components/Inspector.tsx';
import { Problems } from './components/Problems.tsx';
import { Tree } from './components/Tree.tsx';
import { useEffect } from 'react';
import { GhostView } from './components/GhostView.tsx';
import { Palette } from './components/Palette.tsx';
import { TreeToolbar } from './components/TreeToolbar.tsx';
import { ZoomControl } from './components/ZoomControl.tsx';
import { selectedNode, sendEdit, useDesigner } from './store.ts';
import { postMessage } from './vscode.ts';

/** 消せるノード(ウィンドウ・メニューの項目などは除く)。 */
function canRemove(node: OutlineNode | undefined): boolean {
  return (
    node !== undefined &&
    [
      'widget',
      'spacer',
      'tab',
      'pane',
      'action',
      'object',
      'menu_bar',
      'tool_bar',
      'status_bar',
      'menu',
      'menu_item',
      'separator',
      'tool_item',
    ].includes(node.kind)
  );
}

function removeSelected(): void {
  const node = selectedNode(useDesigner.getState());
  if (node && canRemove(node)) {
    // タブ・区画は、中の部品ごと消す
    sendEdit({ kind: 'remove', targets: [node.path] });
  }
}

export function App() {
  const selected = useDesigner(selectedNode);
  useEffect(() => {
    // Delete キーで消す(入力欄の中では消さない)
    const key = (e: KeyboardEvent): void => {
      const t = e.target as HTMLElement | null;
      if (t && ['INPUT', 'TEXTAREA', 'SELECT'].includes(t.tagName)) {
        return;
      }
      if (e.key === 'Delete') {
        removeSelected();
      }
    };
    window.addEventListener('keydown', key);
    return () => {
      window.removeEventListener('keydown', key);
    };
  }, []);
  const document = useDesigner((s) => s.document);
  const diagnostics = useDesigner((s) => s.diagnostics);
  const editError = useDesigner((s) => s.editError);
  const version = useDesigner((s) => s.version);

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
        <ZoomControl />
        {editError !== undefined && <span className="edit-error">{editError}</span>}
      </div>
      <GhostView />
      <div className="panes">
        <div className="pane tree-pane">
          <Palette />
          <TreeToolbar />
          <Tree />
        </div>
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
