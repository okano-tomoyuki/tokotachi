/** 構造の木(docs/designer/editor-design.md §5)。キャンバスと選択を共有する。 */
import { l10n, type OutlineNode } from '@tokotachi-designer/core';
import { useState } from 'react';
import { useDesigner } from '../store.ts';
import { ClassIcon, nodeIconKey } from './ClassIcon.tsx';

/** 木の 1 行の表示(名前と、括弧の中にクラス)。 */
function label(node: OutlineNode): string {
  switch (node.kind) {
    case 'group':
      switch (node.label) {
        case 'actions':
          return l10n.t('Actions');
        case 'variables':
          return l10n.t('Variables');
        default:
          return l10n.t('Objects');
      }
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
  const selected = useDesigner((s) => s.selection === node.id);
  const select = useDesigner((s) => s.select);
  const [open, setOpen] = useState(true);
  const hasChildren = node.children.length > 0;
  return (
    <>
      <div
        className={`tree-row${selected ? ' selected' : ''}`}
        style={{ paddingLeft: depth * 12 + 4 }}
        onClick={() => {
          select(node.id);
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
        {node.class !== undefined && <span className="tree-class">{node.class}</span>}
      </div>
      {open && node.children.map((c) => <Row key={c.id} node={c} depth={depth + 1} />)}
    </>
  );
}

export function Tree() {
  const roots = useDesigner((s) => s.outline);
  return (
    <div className="tree">
      {roots.map((r) => (
        <Row key={r.id} node={r} depth={0} />
      ))}
    </div>
  );
}
