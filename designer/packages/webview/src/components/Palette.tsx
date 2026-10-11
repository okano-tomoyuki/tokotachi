/**
 * パレット(docs/designer/editor-design.md §4.1)。カタログの分類ごとに部品を並べる。
 * キャンバスへドラッグして入れるか、クリックで選んでいる場所に足す。名前で絞り込め、分類は畳める(畳んだものは覚えておく)。
 * 「部品」と「非ビジュアル」の 2 つのタブに分け、非ビジュアルのタブには画面に出ないもの(Action・オブジェクト・Variable)を並べて、
 * 行の「+」で足す。部品のタブの終わりには、ワークスペースの部品ファイル(docs/adr/0043)を並べる。
 */
import { getCatalog, l10n, type Category } from '@tokotachi-designer/core';
import { Fragment, useState, type ReactNode } from 'react';
import { beginDrag, clickAdd } from '../dragController.ts';
import type { Drag } from '../dropTarget.ts';
import { labelGhost } from '../ghost.ts';
import { useDesigner, type MenuItem } from '../store.ts';
import { nonVisualItems } from '../treeOps.ts';
import { ClassIcon } from './ClassIcon.tsx';

/** 画面に出る部品の分類(non_visual は構造の木の見出しから足す)。 */
type WidgetCategory = Exclude<Category, 'non_visual'>;

const CATEGORIES: readonly WidgetCategory[] = ['input', 'display', 'view', 'container', 'drawing'];

/** 非ビジュアルのタブの分類(構造の木の欄と同じ)。 */
type NonVisualGroup = 'actions' | 'objects' | 'variables';

const NON_VISUAL_GROUPS: readonly NonVisualGroup[] = ['actions', 'objects', 'variables'];

type GroupId = WidgetCategory | 'spacer' | 'components' | NonVisualGroup;

function categoryLabel(c: GroupId): string {
  switch (c) {
    case 'input':
      return l10n.t('Input');
    case 'display':
      return l10n.t('Display');
    case 'view':
      return l10n.t('Views');
    case 'container':
      return l10n.t('Containers');
    case 'drawing':
      return l10n.t('Drawing');
    case 'spacer':
      return l10n.t('Spacers');
    case 'components':
      return l10n.t('Component files');
    case 'actions':
      return l10n.t('Actions');
    case 'objects':
      return l10n.t('Objects');
    case 'variables':
      return l10n.t('Variables');
  }
}

function Item({
  label,
  icon,
  title,
  drag,
}: {
  label: string;
  icon: string;
  title?: string;
  drag: Drag;
}) {
  return (
    <div
      className="palette-item"
      title={title}
      onMouseDown={(e) => {
        if (e.button !== 0) {
          return;
        }
        e.preventDefault();
        beginDrag(
          drag,
          e,
          () => {
            clickAdd(drag);
          },
          labelGhost(label),
        );
      }}
    >
      <ClassIcon name={icon} />
      {label}
    </div>
  );
}

/** 分類の見出しと、その部品(絞り込みに合うものだけ。合うものが無ければ出さない)。 */
function Group({
  id,
  filter,
  items,
}: {
  id: GroupId;
  filter: string;
  items: readonly { key: string; label: string; element: ReactNode }[];
}) {
  const closed = useDesigner((s) => s.sidebar.closedCategories.includes(id));
  const setSidebar = useDesigner((s) => s.setSidebar);
  const shown = items.filter((c) => c.label.toLowerCase().includes(filter));
  if (shown.length === 0) {
    return null;
  }
  // 絞り込んでいる間は、畳んだ分類も開いて見せる
  const open = filter !== '' || !closed;
  return (
    <div className="palette-group">
      <div
        className="palette-category"
        onClick={() => {
          const list = useDesigner.getState().sidebar.closedCategories;
          setSidebar({
            closedCategories: closed ? list.filter((c) => c !== id) : [...list, id],
          });
        }}
      >
        <span className="twisty">{open ? '▾' : '▸'}</span>
        {categoryLabel(id)}
      </div>
      {open && shown.map((c) => <Fragment key={c.key}>{c.element}</Fragment>)}
    </div>
  );
}

/** 非ビジュアルの項目。右の「+」で足す(キャンバスには置かないので、ドラッグはしない。行のダブルクリックでも足す)。 */
function NonVisualItem({ item, icon }: { item: MenuItem; icon: string }) {
  return (
    <div
      className="palette-item non-visual"
      title={item.title}
      onDoubleClick={() => {
        item.run();
      }}
    >
      <ClassIcon name={icon} />
      <span className="palette-label">{item.label}</span>
      <button
        className="palette-add"
        title={item.title}
        onClick={() => {
          item.run();
        }}
      >
        +
      </button>
    </div>
  );
}

export function Palette() {
  const [filter, setFilter] = useState('');
  const tab = useDesigner((s) => s.sidebar.paletteTab);
  const setSidebar = useDesigner((s) => s.setSidebar);
  // 足すときの名前(action_1 など)をドキュメントから決めるので、変わったら作り直す
  useDesigner((s) => s.version);
  const components = useDesigner((s) => s.components);
  const classes = Object.entries(getCatalog().classes);
  const needle = filter.trim().toLowerCase();
  const spacers = [
    {
      key: 'stretch',
      label: l10n.t('Stretch'),
      title: l10n.t('A stretch in a BoxLayout (add_stretch)'),
      drag: { kind: 'spacer', spacer: { stretch: 1 } } as const,
    },
    {
      key: 'spacing',
      label: l10n.t('Spacing'),
      title: l10n.t('A fixed spacing in a BoxLayout (add_spacing)'),
      drag: { kind: 'spacer', spacer: { spacing: 12 } } as const,
    },
  ];
  return (
    <div className="palette">
      <div className="section-head">
        <span>{l10n.t('Palette')}</span>
        <input
          type="search"
          className="palette-filter"
          placeholder={l10n.t('Filter')}
          value={filter}
          onChange={(e) => {
            setFilter(e.target.value);
          }}
          onKeyDown={(e) => {
            if (e.key === 'Escape') {
              setFilter('');
            }
          }}
        />
      </div>
      <div className="tabs palette-tabs">
        {(
          [
            ['visual', l10n.t('Widgets')],
            ['non_visual', l10n.t('Non-visual')],
          ] as const
        ).map(([id, label]) => (
          <button
            key={id}
            className={tab === id ? 'active' : ''}
            onClick={() => {
              setSidebar({ paletteTab: id });
            }}
          >
            {label}
          </button>
        ))}
      </div>
      <div className="palette-body">
        {tab === 'visual' ? (
          <>
            {CATEGORIES.map((c) => (
              <Group
                key={c}
                id={c}
                filter={needle}
                items={classes
                  .filter(([, info]) => info.kind === 'widget' && info.category === c)
                  .map(([cls, info]) => ({
                    key: cls,
                    label: cls,
                    element: (
                      <Item
                        label={cls}
                        icon={cls}
                        title={info.doc}
                        drag={{ kind: 'new', class: cls }}
                      />
                    ),
                  }))}
              />
            ))}
            <Group
              id="spacer"
              filter={needle}
              items={spacers.map((sp) => ({
                key: sp.key,
                label: sp.label,
                element: <Item label={sp.label} icon={sp.key} title={sp.title} drag={sp.drag} />,
              }))}
            />
            <Group
              id="components"
              filter={needle}
              items={components.map((c) => ({
                key: c.path,
                label: c.className,
                element: (
                  <Item
                    label={c.className}
                    icon="Widget"
                    title={c.path}
                    drag={{ kind: 'new', class: 'Widget', component: c.path }}
                  />
                ),
              }))}
            />
          </>
        ) : (
          NON_VISUAL_GROUPS.map((g) => (
            <Group
              key={g}
              id={g}
              filter={needle}
              items={nonVisualItems(g).map((item) => ({
                key: item.label,
                label: item.label,
                element: (
                  <NonVisualItem
                    item={item}
                    icon={g === 'actions' ? 'Action' : g === 'variables' ? 'Variable' : item.label}
                  />
                ),
              }))}
            />
          ))
        )}
      </div>
    </div>
  );
}
