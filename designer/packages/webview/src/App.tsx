import { hasErrors, l10n } from '@tokotachi-designer/core';
import { CanvasView } from './components/CanvasView.tsx';
import { Inspector } from './components/Inspector.tsx';
import { Problems } from './components/Problems.tsx';
import { Tree } from './components/Tree.tsx';
import { useDesigner } from './store.ts';
import { postMessage } from './vscode.ts';

export function App() {
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
        {editError !== undefined && <span className="edit-error">{editError}</span>}
      </div>
      <div className="panes">
        <div className="pane tree-pane">
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
