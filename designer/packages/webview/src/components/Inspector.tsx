/**
 * インスペクタ(docs/designer/editor-design.md §5)。選んだノードの名前・プロパティ・ハンドラを編集する。
 */
import {
  propertyRange,
  bindingOf,
  checkValue,
  defaultHandlerName,
  findClass,
  flattenOutline,
  isBindable,
  l10n,
  namedNodes,
  newName,
  variableTypeOf,
  variableValue,
  VariableType,
  type OutlineNode,
  type Properties,
  type PropertyInfo,
  type PropertyValue,
  type VariableNode,
} from '@tokotachi-designer/core';
import { useEffect, useState } from 'react';
import { selectedNode, selectedNodes, sendEdit, useDesigner } from '../store.ts';
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

/** 新しい Variable の名前(`<部品の名前>_<プロパティ>`。使われていれば番号を付ける)。 */
function variableNameFor(node: OutlineNode, property: string): string {
  const doc = useDesigner.getState().document;
  if (!doc) {
    return property;
  }
  const base = `${node.kind === 'window' ? 'window' : node.label}_${property}`;
  return namedNodes(doc).some((n) => n.name === base) ? newName(doc, base) : base;
}

/**
 * プロパティを Variable に結びつける選択肢(同じ型の Variable と「新しい Variable」。dsl-spec.md §5.2)。
 * 外すと、プロパティは Variable の値になる(見た目は変わらない)。
 */
function BindSelect({
  node,
  name,
  info,
  value,
}: {
  node: OutlineNode;
  name: string;
  info: PropertyInfo;
  value: PropertyValue | undefined;
}) {
  const doc = useDesigner((s) => s.document);
  const type = variableTypeOf(info.type);
  const bound = bindingOf(value);
  const variables = (doc?.variables ?? []).filter((v) => v.type === type);
  const NEW = '\u0000new';
  const set = (v: PropertyValue | null): void => {
    sendEdit({ kind: 'setProperties', target: node.path, properties: { [name]: v } });
  };
  const change = (selected: string): void => {
    if (selected === '') {
      const v = doc?.variables?.find((x) => x.name === bound);
      set(v ? variableValue(v.type, v.value) : null);
    } else if (selected === NEW && type !== undefined) {
      // 今の値を新しい Variable の初期値にする
      const variable = variableNameFor(node, name);
      const initial =
        value !== undefined && checkValue(info.type, value) === null ? value : undefined;
      sendEdit({
        kind: 'batch',
        commands: [
          { kind: 'addVariable', name: variable, type, value: initial },
          { kind: 'setProperties', target: node.path, properties: { [name]: { bind: variable } } },
        ],
      });
    } else {
      set({ bind: selected });
    }
  };
  return (
    <select
      className={bound === undefined ? 'bind-select' : 'bind-select bound'}
      title={l10n.t('Bind to a variable')}
      value={bound ?? ''}
      onChange={(e) => {
        change(e.target.value);
      }}
    >
      <option value="">{bound === undefined ? '⛓' : l10n.t('(Unbind)')}</option>
      {variables.map((v) => (
        <option key={v.name} value={v.name}>
          {v.name}
        </option>
      ))}
      <option value={NEW}>{l10n.t('New variable')}</option>
    </select>
  );
}

/** 結びつけたプロパティの値(Variable の初期値)。 */
function BoundValue({ variable }: { variable: string }) {
  const doc = useDesigner((s) => s.document);
  const v = doc?.variables?.find((x) => x.name === variable);
  const text = v ? JSON.stringify(variableValue(v.type, v.value)) : '';
  return (
    <span className="bound-value" title={l10n.t('Bound to the variable {0}', variable)}>
      {`${variable} = ${text}`}
    </span>
  );
}

/** Variable: 名前・型・初期値・ハンドラ。 */
function VariableEditor({ node }: { node: OutlineNode }) {
  const json = nodeJson(node.path) as VariableNode | undefined;
  const info = findClass('Variable');
  if (!json || !info) {
    return null;
  }
  return (
    <>
      <div className="inspector-head">
        <span className="inspector-class">{`Variable<${json.type}>`}</span>
        <NameField node={node} name={json.name} />
      </div>
      <table className="grid">
        <tbody>
          <tr title={l10n.t('The type of the value (T of tokotachi::Variable<T>)')}>
            <th className="set">type</th>
            <td>
              <select
                value={json.type}
                onChange={(e) => {
                  sendEdit({
                    kind: 'setVariable',
                    target: node.path,
                    type: e.target.value as VariableType,
                  });
                }}
              >
                {VariableType.options.map((t) => (
                  <option key={t} value={t}>
                    {t}
                  </option>
                ))}
              </select>
            </td>
          </tr>
          <tr title={l10n.t('The initial value')}>
            <th className={json.value === undefined ? '' : 'set'}>value</th>
            <td>
              <PropertyField
                name="value"
                info={{
                  type: json.type,
                  declared_in: 'Variable',
                  default: variableValue(json.type, undefined),
                }}
                value={json.value}
                onCommit={(v) => {
                  sendEdit({ kind: 'setVariable', target: node.path, value: v });
                }}
              />
            </td>
          </tr>
          {Object.entries(info.handlers).map(([event, h]) => (
            <tr key={event} title={h.doc}>
              <th className={json.events?.[event] === undefined ? '' : 'set'}>{event}</th>
              <td>
                <HandlerField node={node} event={event} handler={json.events?.[event]} />
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </>
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
              const bound = bindingOf(value);
              return (
                <tr key={name} title={p.doc}>
                  <th className={value === undefined ? '' : 'set'}>{name}</th>
                  <td className="value-cell">
                    {bound !== undefined ? (
                      <BoundValue variable={bound} />
                    ) : (
                      <PropertyField
                        name={name}
                        info={p}
                        value={value}
                        range={propertyRange(node.class ?? '', p, json)}
                        onCommit={(v) => {
                          sendEdit({
                            kind: 'setProperties',
                            target: node.path,
                            properties: { [name]: v },
                          });
                        }}
                        onPickImage={() => {
                          postMessage({ type: 'pickImage', target: node.path, property: name });
                        }}
                      />
                    )}
                    {isBindable(p) && <BindSelect node={node} name={name} info={p} value={value} />}
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

/**
 * 複数選択のインスペクタ。どのノードにもある(同じ型の)プロパティだけを出し、確定した値をすべてに設定する(1 回の編集)。
 * 値が揃っていなければ「複数の値」と出す。位置と大きさ・名前・ハンドラ・Variable との結びつけは、1 つずつ編集する。
 */
function MultiEditor({ nodes }: { nodes: readonly OutlineNode[] }) {
  const editable = nodes.filter((n) => n.editable && n.class !== undefined);
  const jsons = editable.map((n) => nodeJson(n.path));
  const infos = editable.map((n) => findClass(n.class ?? ''));
  const first = infos[0];
  const props = first
    ? Object.entries(first.properties).filter(
        ([name, p]) =>
          p.geometry !== true &&
          infos.every(
            (info) => JSON.stringify(info?.properties[name]?.type) === JSON.stringify(p.type),
          ),
      )
    : [];
  const classes = [...new Set(editable.map((n) => n.class))];
  return (
    <>
      <div className="inspector-head">
        <span className="inspector-class">{l10n.t('{0} items selected', nodes.length)}</span>
        <span className="tree-class">{classes.join(', ')}</span>
      </div>
      {editable.length < nodes.length && (
        <div className="inspector-empty">
          {l10n.t('Only the common properties of the widgets and objects are shown.')}
        </div>
      )}
      <table className="grid">
        <tbody>
          {props.map(([name, p]) => {
            const values = jsons.map((j) => j?.properties?.[name]);
            const bound = values.some((v) => bindingOf(v) !== undefined);
            // 書いていなければ既定値(クラスで違うことがある)で比べる
            const effective = values.map((v, i) => v ?? infos[i]?.properties[name]?.default);
            const same = effective.every((v) => JSON.stringify(v) === JSON.stringify(effective[0]));
            const written = values.every((v) => JSON.stringify(v) === JSON.stringify(values[0]));
            const value = !same
              ? undefined
              : written
                ? values[0]
                : (effective[0] as PropertyValue | undefined);
            return (
              <tr key={name} title={p.doc}>
                <th className={values.some((v) => v !== undefined) ? 'set' : ''}>{name}</th>
                <td className="value-cell">
                  {bound ? (
                    <div className="bound-value">{l10n.t('Some are bound to a variable')}</div>
                  ) : (
                    <PropertyField
                      name={name}
                      info={p}
                      value={value}
                      mixed={!same}
                      onCommit={(v) => {
                        sendEdit(
                          {
                            kind: 'batch',
                            commands: editable.map((n) => ({
                              kind: 'setProperties' as const,
                              target: n.path,
                              properties: { [name]: v },
                            })),
                          },
                          editable.flatMap((n) => (n.label === '' ? [] : [n.label])),
                        );
                      }}
                    />
                  )}
                </td>
              </tr>
            );
          })}
        </tbody>
      </table>
    </>
  );
}

export function Inspector() {
  const node = useDesigner(selectedNode);
  const count = useDesigner((s) => s.selected.length);
  // ドキュメントが変わったら描き直す
  useDesigner((s) => s.version);
  if (count > 1) {
    return (
      <div className="inspector">
        <MultiEditor nodes={selectedNodes(useDesigner.getState())} />
      </div>
    );
  }
  let content;
  if (node?.kind === 'spacer') {
    content = <LayoutSection node={node} />;
  } else if (node?.kind === 'menu') {
    content = <MenuEditor node={node} />;
  } else if (node?.kind === 'menu_item' || node?.kind === 'tool_item') {
    content = <EntryEditor node={node} />;
  } else if (node?.kind === 'variable') {
    content = <VariableEditor node={node} />;
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
