/**
 * 構造の木(docs/designer/editor-design.md §5)。キャンバスと選択を共有する。
 * ウィンドウの木の下に、画面に出ないもの(Action・オブジェクト・変数)の欄を分けて常に並べる(足すのはパレットの非ビジュアルのタブから)。
 * 足す・並べ替える・消す操作は、行の右クリックのメニューに出す(treeOps.ts)。
 */
import { flattenOutline, l10n, type OutlineNode } from '@tokotachi-designer/core';
import { useEffect, useRef, useState } from 'react';
import { dragWith } from '../pointerDrag.ts';
import { nodeMenu } from '../treeOps.ts';
import { openContextMenu, useDesigner } from '../store.ts';
import { ClassIcon, nodeIconKey } from './ClassIcon.tsx';

type Group = 'actions' | 'objects' | 'variables';

/** 見出しの順(構造の木の outline と同じ順)。 */
const GROUPS: readonly Group[] = ['actions', 'objects', 'variables'];

/** 部品ファイルを置いた部品の、木に出す種類(部品ファイルの名前。`parts/search_bar.ttui.json` → `search_bar.ttui.json`)。 */
function componentName(path: string): string {
  return path.split('/').pop() ?? path;
}

function groupLabel(group: Group): string {
  switch (group) {
    case 'actions':
      return l10n.t('Actions');
    case 'variables':
      return l10n.t('Variables');
    case 'objects':
      return l10n.t('Objects');
  }
}

/** 木の 1 行の表示(名前と、括弧の中にクラス)。 */
function label(node: OutlineNode): string {
  switch (node.kind) {
    case 'separator':
      return '———';
    case 'menu_item':
    case 'tool_item':
      return `→ ${node.label}`;
    default:
      return node.label;
  }
}

function Row({ node, depth }: { node: OutlineNode; depth: number }) {
  const selected = useDesigner((s) => s.selected.includes(node.id));
  const primary = useDesigner((s) => s.selection === node.id);
  const select = useDesigner((s) => s.select);
  const toggleSelect = useDesigner((s) => s.toggleSelect);
  const selectRange = useDesigner((s) => s.selectRange);
  const hidden = useDesigner((s) => s.hidden.get(node.id));
  const [open, setOpen] = useState(true);
  const ref = useRef<HTMLDivElement>(null);
  const hasChildren = node.children.length > 0;
  useEffect(() => {
    // キャンバスで選んだときも、選んだ行が見えるようにする
    if (primary) {
      ref.current?.scrollIntoView({ block: 'nearest' });
    }
  }, [primary]);
  return (
    <>
      <div
        ref={ref}
        className={`tree-row${selected ? ' selected' : ''}${hidden ? ' hidden' : ''}`}
        title={
          hidden === 'self'
            ? l10n.t('Hidden (visible is false)')
            : hidden === 'ancestor'
              ? l10n.t('Not shown because a parent is hidden')
              : undefined
        }
        style={{ paddingLeft: depth * 12 + 4 }}
        onClick={(e) => {
          // Ctrl+クリックで選択に足す・外す、Shift+クリックで範囲を選ぶ
          if (e.ctrlKey || e.metaKey) {
            toggleSelect(node.id);
          } else if (e.shiftKey) {
            selectRange(
              node.id,
              flattenOutline(useDesigner.getState().outline).map((n) => n.id),
            );
          } else {
            select(node.id);
          }
        }}
        onContextMenu={(e) => {
          e.preventDefault();
          // 選んでいない行なら、その行だけを選ぶ(選んでいる行なら、選択のまま操作する)
          if (!useDesigner.getState().selected.includes(node.id)) {
            select(node.id);
          }
          openContextMenu(e.clientX, e.clientY, nodeMenu(node));
        }}
      >
        <span
          className="twisty"
          onClick={(e) => {
            e.stopPropagation();
            setOpen(!open);
          }}
        >
          {hasChildren ? (open ? '▾' : '▸') : ''}
        </span>
        <ClassIcon name={nodeIconKey(node)} />
        <span className="tree-label">{label(node)}</span>
        {node.class !== undefined && (
          <span className="tree-class">
            {node.component === undefined ? node.class : componentName(node.component)}
          </span>
        )}
        {hidden === 'self' && <span className="tree-hidden">{l10n.t('Hidden')}</span>}
      </div>
      {open && node.children.map((c) => <Row key={c.id} node={c} depth={depth + 1} />)}
    </>
  );
}

function GroupSection({ group, node }: { group: Group; node: OutlineNode | undefined }) {
  const [open, setOpen] = useState(true);
  const children = node?.children ?? [];
  return (
    <>
      <div
        className="tree-group-head"
        onClick={() => {
          setOpen(!open);
        }}
      >
        <span className="twisty">{children.length > 0 ? (open ? '▾' : '▸') : ''}</span>
        <span className="tree-label">{groupLabel(group)}</span>
        {children.length > 0 && <span className="tree-count">{children.length}</span>}
      </div>
      {open && children.map((c) => <Row key={c.id} node={c} depth={1} />)}
    </>
  );
}

const MIN_RATIO = 0.1;
const MAX_RATIO = 0.9;

export function Tree() {
  const roots = useDesigner((s) => s.outline);
  const ratio = useDesigner((s) => s.sidebar.groupsRatio);
  const setSidebar = useDesigner((s) => s.setSidebar);
  const ref = useRef<HTMLDivElement>(null);
  return (
    <div ref={ref} className="tree-area">
      <div className="tree">
        {roots
          .filter((r) => r.kind !== 'group')
          .map((r) => (
            <Row key={r.id} node={r} depth={0} />
          ))}
      </div>
      {/* ウィンドウの木が長くても隠れないよう、下に分けて置く。間の仕切りで高さの割合を変える */}
      <div
        className="sidebar-split"
        title={l10n.t('Drag to resize')}
        onPointerDown={(e) => {
          dragWith(e, (_x, y) => {
            const r = ref.current?.getBoundingClientRect();
            if (r && r.height > 0) {
              const next = (r.bottom - y) / r.height;
              setSidebar({ groupsRatio: Math.min(MAX_RATIO, Math.max(MIN_RATIO, next)) });
            }
          });
        }}
      />
      <div className="tree-groups" style={{ flexBasis: `${String(ratio * 100)}%` }}>
        {GROUPS.map((g) => (
          <GroupSection
            key={g}
            group={g}
            node={roots.find((r) => r.kind === 'group' && r.label === g)}
          />
        ))}
      </div>
    </div>
  );
}
