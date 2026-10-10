/**
 * インスペクタ(docs/designer/editor-design.md §5)。選んだノードの名前・プロパティ・ハンドラを編集する。
 */
import {
  defaultHandlerName,
  findClass,
  flattenOutline,
  l10n,
  type OutlineNode,
  type Properties,
} from '@tokotachi-designer/core';
import { useEffect, useState } from 'react';
import { selectedNode, sendEdit, useDesigner } from '../store.ts';
import { postMessage } from '../vscode.ts';
import { LayoutSection } from './LayoutSection.tsx';
import { PropertyField } from './PropertyField.tsx';

interface NodeJson {
  readonly name?: string;
  readonly title?: string;
  readonly properties?: Properties;
  readonly events?: Readonly<Record<string, string>>;
}

/** パスが指すドキュメントのノード。 */
function nodeJson(path: readonly (string | number)[]): NodeJson | undefined {
  let current: unknown = useDesigner.getState().document;
  for (const key of path) {
    current = (current as Record<string | number, unknown> | undefined)?.[key];
  }
  return current as NodeJson | undefined;
}

/** 名前の入力欄(Enter かフォーカスを外したときに確定)。 */
function NameField({ node, name }: { node: OutlineNode; name: string }) {
  const [text, setText] = useState(name);
  useEffect(() => {
    setText(name);
  }, [name]);
  const commit = (): void => {
    if (text !== name && text !== '') {
      sendEdit({ kind: 'rename', target: node.path, name: text });
    } else {
      setText(name);
    }
  };
  return (
    <input
      type="text"
      className="name-field"
      value={text}
      onChange={(e) => {
        setText(e.target.value);
      }}
      onBlur={commit}
      onKeyDown={(e) => {
        if (e.key === 'Enter') {
          commit();
        } else if (e.key === 'Escape') {
          setText(name);
        }
      }}
    />
  );
}

/** 位置と大きさのプロパティを出すか(ウィンドウは width・height、自由な配置の子はすべて)。 */
function showGeometry(node: OutlineNode, name: string): boolean {
  if (node.kind === 'window') {
    return name === 'width' || name === 'height';
  }
  return node.path[node.path.length - 2] === 'children';
}

function HandlerField({
  node,
  event,
  handler,
}: {
  node: OutlineNode;
  event: string;
  handler: string | undefined;
}) {
  const [text, setText] = useState(handler ?? '');
  useEffect(() => {
    setText(handler ?? '');
  }, [handler]);
  const commit = (): void => {
    if (text === (handler ?? '')) {
      return;
    }
    sendEdit({ kind: 'setEvent', target: node.path, event, handler: text === '' ? null : text });
  };
  const go = (): void => {
    const name =
      handler ?? defaultHandlerName(node.kind === 'window' ? undefined : node.label, event);
    if (handler === undefined) {
      sendEdit({ kind: 'setEvent', target: node.path, event, handler: name });
    }
    postMessage({ type: 'goToHandler', handler: name });
  };
  return (
    <div className="handler-field">
      <input
        type="text"
        value={text}
        onChange={(e) => {
          setText(e.target.value);
        }}
        onBlur={commit}
        onKeyDown={(e) => {
          if (e.key === 'Enter') {
            commit();
          } else if (e.key === 'Escape') {
            setText(handler ?? '');
          }
        }}
      />
      <button
        title={
          handler === undefined ? l10n.t('Add a handler and open it') : l10n.t('Open the handler')
        }
        onClick={go}
      >
        {handler === undefined ? '+' : '→'}
      </button>
    </div>
  );
}

/** ノードの名前・レイアウトの欄・プロパティ・ハンドラ。 */
function NodeEditor({ node }: { node: OutlineNode }) {
  const [tab, setTab] = useState<'properties' | 'events'>('properties');
  if (node.class === undefined) {
    return null;
  }
  const info = findClass(node.class);
  const json = nodeJson(node.path);
  if (!info || !json) {
    return null;
  }
  const props = Object.entries(info.properties).filter(
    ([name, p]) => p.geometry !== true || showGeometry(node, name),
  );
  const handlers = Object.entries(info.handlers);
  return (
    <>
      <div className="inspector-head">
        <span className="inspector-class">{node.class}</span>
        {json.name !== undefined && <NameField node={node} name={json.name} />}
      </div>
      <LayoutSection node={node} />
      <div className="tabs">
        <button
          className={tab === 'properties' ? 'active' : ''}
          onClick={() => {
            setTab('properties');
          }}
        >
          {l10n.t('Properties')}
        </button>
        <button
          className={tab === 'events' ? 'active' : ''}
          disabled={handlers.length === 0}
          onClick={() => {
            setTab('events');
          }}
        >
          {l10n.t('Handlers')}
        </button>
      </div>
      {tab === 'properties' || handlers.length === 0 ? (
        <table className="grid">
          <tbody>
            {props.map(([name, p]) => {
              const value = json.properties?.[name];
              return (
                <tr key={name} title={p.doc}>
                  <th className={value === undefined ? '' : 'set'}>{name}</th>
                  <td>
                    <PropertyField
                      name={name}
                      info={p}
                      value={value}
                      onCommit={(v) => {
                        sendEdit({
                          kind: 'setProperties',
                          target: node.path,
                          properties: { [name]: v },
                        });
                      }}
                    />
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      ) : (
        <table className="grid">
          <tbody>
            {handlers.map(([event, h]) => (
              <tr key={event} title={h.doc}>
                <th className={json.events?.[event] === undefined ? '' : 'set'}>{event}</th>
                <td>
                  <HandlerField node={node} event={event} handler={json.events?.[event]} />
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </>
  );
}

/** メニュー: 名前(省略できる)と見出し。 */
function MenuEditor({ node }: { node: OutlineNode }) {
  const json = nodeJson(node.path);
  return (
    <>
      <div className="inspector-head">
        <span className="inspector-class">Menu</span>
        <NameField node={node} name={json?.name ?? ''} />
      </div>
      <table className="grid">
        <tbody>
          <tr>
            <th className="set">title</th>
            <td>
              <PropertyField
                name="title"
                info={{ type: 'string', declared_in: 'Menu', editor: 'mnemonic' }}
                value={json?.title ?? ''}
                onCommit={(v) => {
                  sendEdit({
                    kind: 'setMenuTitle',
                    target: node.path,
                    title: typeof v === 'string' ? v : '',
                  });
                }}
              />
            </td>
          </tr>
        </tbody>
      </table>
    </>
  );
}

/** メニュー・ツールバーの項目: 参照する Action を選び、その Action を編集する。 */
function EntryEditor({ node }: { node: OutlineNode }) {
  const outline = useDesigner((s) => s.outline);
  const document = useDesigner((s) => s.document);
  const action = (nodeJson(node.path) as { action?: string } | undefined)?.action ?? '';
  const target = flattenOutline(outline).find((n) => n.kind === 'action' && n.label === action);
  return (
    <>
      <table className="grid">
        <tbody>
          <tr>
            <th className="set">action</th>
            <td>
              <select
                value={action}
                onChange={(e) => {
                  sendEdit({ kind: 'setEntryAction', target: node.path, action: e.target.value });
                }}
              >
                {(document?.actions ?? []).map((a) => (
                  <option key={a.name} value={a.name}>
                    {a.name}
                  </option>
                ))}
              </select>
            </td>
          </tr>
        </tbody>
      </table>
      {target && <NodeEditor node={target} />}
    </>
  );
}

export function Inspector() {
  const node = useDesigner(selectedNode);
  // ドキュメントが変わったら描き直す
  useDesigner((s) => s.version);
  let content;
  if (node?.kind === 'spacer') {
    content = <LayoutSection node={node} />;
  } else if (node?.kind === 'menu') {
    content = <MenuEditor node={node} />;
  } else if (node?.kind === 'menu_item' || node?.kind === 'tool_item') {
    content = <EntryEditor node={node} />;
  } else if (node?.editable && node.class !== undefined) {
    // 選んだノードが変わったら、タブを最初に戻す
    content = <NodeEditor key={node.id} node={node} />;
  } else {
    return (
      <div className="inspector-empty">{l10n.t('Select a widget to edit its properties.')}</div>
    );
  }
  return <div className="inspector">{content}</div>;
}
