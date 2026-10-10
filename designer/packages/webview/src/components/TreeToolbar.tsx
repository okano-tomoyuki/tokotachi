/**
 * 構造の木の上のボタン(docs/designer/editor-design.md §5)。選んだノードに合わせて、足す・並べ替える操作を出す。
 */
import {
  findClass,
  l10n,
  newName,
  valueAt,
  type EditCommand,
  type JsonPath,
  type OutlineNode,
} from '@tokotachi-designer/core';
import { selectedNode, sendEdit, useDesigner } from '../store.ts';

interface Json {
  readonly [key: string]: unknown;
}

interface Op {
  readonly label: string;
  readonly title: string;
  readonly run: () => void;
}

/** 新しい Action を作って、メニュー・ツールバーの index の位置から参照する。 */
function addItem(container: JsonPath, index: number): void {
  const doc = useDesigner.getState().document;
  if (!doc) {
    return;
  }
  const name = newName(doc, 'action');
  sendEdit(
    {
      kind: 'batch',
      commands: [
        { kind: 'addAction', name },
        { kind: 'addEntry', container, index, entry: { action: name } },
      ],
    },
    name,
  );
}

function length(path: JsonPath, key: string): number {
  const list = (valueAt(useDesigner.getState().document, path) as Json | undefined)?.[key];
  return Array.isArray(list) ? list.length : 0;
}

/** 同じ並びの中で動かせるノードか(並びの中の位置を持つもの)。 */
function movable(node: OutlineNode): boolean {
  const p = node.path;
  const last = p[p.length - 1];
  const target = last === 'widget' || last === 'page' || last === 'menu' ? p.slice(0, -1) : p;
  return typeof target[target.length - 1] === 'number' && node.kind !== 'window';
}

function opsFor(node: OutlineNode): Op[] {
  const doc = useDesigner.getState().document;
  if (!doc) {
    return [];
  }
  const ops: Op[] = [];
  const send = (command: EditCommand, select?: string) => () => {
    sendEdit(command, select);
  };
  const p = node.path;
  switch (node.kind) {
    case 'window':
      for (const [bar, label] of [
        ['menu_bar', l10n.t('Menu bar')],
        ['tool_bar', l10n.t('Tool bar')],
        ['status_bar', l10n.t('Status bar')],
      ] as const) {
        if (doc.window[bar] === undefined) {
          ops.push({
            label: `+ ${label}`,
            title: l10n.t('Add a {0} to the window', label),
            run: send({ kind: 'addBar', bar }),
          });
        }
      }
      break;
    case 'widget':
    case 'column': {
      // 一覧の部品(ListView・TreeView)と、その列: 列を足す(列を選んでいれば、その後ろに)
      const view = node.kind === 'column' ? p.slice(0, -2) : p;
      const cls = (valueAt(doc, view) as Json | undefined)?.['class'];
      if (typeof cls === 'string' && (findClass(cls)?.child_keys ?? []).includes('columns')) {
        const index =
          node.kind === 'column' ? Number(p[p.length - 1]) + 1 : length(view, 'columns');
        ops.push({
          label: `+ ${l10n.t('Column')}`,
          title: l10n.t('Add a column'),
          run: send({ kind: 'addColumn', target: view, title: l10n.t('Column'), index }),
        });
      }
      break;
    }
    case 'menu_bar':
      ops.push({
        label: `+ ${l10n.t('Menu')}`,
        title: l10n.t('Add a menu'),
        run: send({ kind: 'addMenu', parent: p, index: length(p, 'menus'), title: 'Menu' }),
      });
      break;
    case 'menu':
    case 'tool_bar': {
      ops.push({
        label: `+ ${l10n.t('Item')}`,
        title: l10n.t('Add an item with a new action'),
        run: () => {
          addItem(p, length(p, 'items'));
        },
      });
      ops.push({
        label: `+ ${l10n.t('Separator')}`,
        title: l10n.t('Add a separator'),
        run: send({
          kind: 'addEntry',
          container: p,
          index: length(p, 'items'),
          entry: { separator: true },
        }),
      });
      if (node.kind === 'menu') {
        ops.push({
          label: `+ ${l10n.t('Submenu')}`,
          title: l10n.t('Add a submenu'),
          run: send({ kind: 'addMenu', parent: p, index: length(p, 'items'), title: 'Submenu' }),
        });
      }
      break;
    }
    case 'menu_item':
    case 'tool_item':
    case 'separator': {
      // 後ろに足す
      const container = p.slice(0, -2);
      const index = Number(p[p.length - 1]) + 1;
      ops.push({
        label: `+ ${l10n.t('Item')}`,
        title: l10n.t('Add an item with a new action after this'),
        run: () => {
          addItem(container, index);
        },
      });
      ops.push({
        label: `+ ${l10n.t('Separator')}`,
        title: l10n.t('Add a separator after this'),
        run: send({ kind: 'addEntry', container, index, entry: { separator: true } }),
      });
      break;
    }
    default:
      break;
  }
  if (
    node.kind === 'window' ||
    node.kind === 'action' ||
    (node.kind === 'group' && node.label === 'actions')
  ) {
    const name = newName(doc, 'action');
    ops.push({
      label: `+ ${l10n.t('Action')}`,
      title: l10n.t('Add an action'),
      run: send({ kind: 'addAction', name }, name),
    });
  }
  if (
    node.kind === 'window' ||
    node.kind === 'variable' ||
    (node.kind === 'group' && node.label === 'variables')
  ) {
    const name = newName(doc, 'variable');
    ops.push({
      label: `+ ${l10n.t('Variable')}`,
      title: l10n.t('Add a variable'),
      run: send({ kind: 'addVariable', name, type: 'string' }, name),
    });
  }
  if (movable(node)) {
    ops.push({
      label: '↑',
      title: l10n.t('Move up'),
      run: send({ kind: 'moveSibling', target: p, delta: -1 }),
    });
    ops.push({
      label: '↓',
      title: l10n.t('Move down'),
      run: send({ kind: 'moveSibling', target: p, delta: 1 }),
    });
  }
  return ops;
}

export function TreeToolbar() {
  const node = useDesigner(selectedNode);
  // ドキュメントが変わったら出すボタンも変わる
  useDesigner((s) => s.version);
  const ops = node ? opsFor(node) : [];
  if (ops.length === 0) {
    return null;
  }
  return (
    <div className="tree-toolbar">
      {ops.map((op) => (
        <button key={op.label} title={op.title} onClick={op.run}>
          {op.label}
        </button>
      ))}
    </div>
  );
}
