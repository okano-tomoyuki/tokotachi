/**
 * 構造の木のノードに対してできる操作(docs/designer/editor-design.md §5)。右クリックのメニューに、ノードに合わせて
 * 足す・並べ替える・消す操作を出す。
 */
import {
  canCopy,
  findClass,
  getCatalog,
  l10n,
  newName,
  valueAt,
  type EditCommand,
  type JsonPath,
  type OutlineNode,
} from '@tokotachi-designer/core';
import { copySelection, duplicateSelection, pasteFromClipboard } from './clipboard.ts';
import { selectedNode, sendEdit, useDesigner, type MenuItem } from './store.ts';

interface Json {
  readonly [key: string]: unknown;
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

function addOps(node: OutlineNode): MenuItem[] {
  const doc = useDesigner.getState().document;
  if (!doc) {
    return [];
  }
  const ops: MenuItem[] = [];
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
  return ops;
}

/** 消せるノード(ウィンドウ・メニューの項目などは除く)。 */
export function canRemove(node: OutlineNode | undefined): boolean {
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
      'column',
      'variable',
    ].includes(node.kind)
  );
}

/** 選んでいるノードを消す(タブ・区画は、中の部品ごと消す)。 */
export function removeSelected(): void {
  const node = selectedNode(useDesigner.getState());
  if (node && canRemove(node)) {
    sendEdit({ kind: 'remove', targets: [node.path] });
  }
}

/** 選んでいるノードを、同じ並びの中で動かす(delta は -1 で前へ、1 で後ろへ)。 */
export function moveSelected(delta: -1 | 1): void {
  const node = selectedNode(useDesigner.getState());
  if (node && movable(node)) {
    sendEdit({ kind: 'moveSibling', target: node.path, delta });
  }
}

/** ノードの右クリックのメニュー(足す操作、並べ替え、削除)。 */
export function nodeMenu(node: OutlineNode): MenuItem[] {
  const items = addOps(node);
  const doc = useDesigner.getState().document;
  const copyable = doc !== undefined && canCopy(doc, node.path);
  items.push(
    {
      label: l10n.t('Cut'),
      shortcut: 'Ctrl+X',
      separator: items.length > 0,
      disabled: !copyable || !canRemove(node),
      run: () => {
        copySelection(true);
      },
    },
    {
      label: l10n.t('Copy'),
      shortcut: 'Ctrl+C',
      disabled: !copyable,
      run: () => {
        copySelection(false);
      },
    },
    {
      label: l10n.t('Paste'),
      shortcut: 'Ctrl+V',
      run: pasteFromClipboard,
    },
    {
      label: l10n.t('Duplicate'),
      shortcut: 'Ctrl+D',
      disabled: !copyable,
      run: duplicateSelection,
    },
  );
  if (movable(node)) {
    items.push(
      {
        label: l10n.t('Move up'),
        shortcut: 'Alt+↑',
        separator: items.length > 0,
        run: () => {
          sendEdit({ kind: 'moveSibling', target: node.path, delta: -1 });
        },
      },
      {
        label: l10n.t('Move down'),
        shortcut: 'Alt+↓',
        run: () => {
          sendEdit({ kind: 'moveSibling', target: node.path, delta: 1 });
        },
      },
    );
  }
  if (canRemove(node)) {
    items.push({
      label: l10n.t('Delete'),
      shortcut: 'Delete',
      separator: items.length > 0,
      run: () => {
        sendEdit({ kind: 'remove', targets: [node.path] });
      },
    });
  }
  return items;
}

/** パレットの非ビジュアルのタブで足すもの(オブジェクトはクラスごと)。 */
export function nonVisualItems(group: 'actions' | 'variables' | 'objects'): MenuItem[] {
  const doc = useDesigner.getState().document;
  if (!doc) {
    return [];
  }
  switch (group) {
    case 'actions': {
      const name = newName(doc, 'action');
      return [
        {
          label: 'Action',
          title: l10n.t('Add an action'),
          run: () => {
            sendEdit({ kind: 'addAction', name }, name);
          },
        },
      ];
    }
    case 'variables': {
      const name = newName(doc, 'variable');
      return [
        {
          label: 'Variable',
          title: l10n.t('Add a variable'),
          run: () => {
            sendEdit({ kind: 'addVariable', name, type: 'string' }, name);
          },
        },
      ];
    }
    case 'objects':
      return Object.entries(getCatalog().classes)
        .filter(([, info]) => info.kind === 'object')
        .map(([cls, info]) => ({
          label: cls,
          title: info.doc,
          run: () => {
            sendEdit({ kind: 'addObject', class: cls });
          },
        }));
  }
}
