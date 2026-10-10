/**
 * パレット(docs/designer/editor-design.md §4.1)。カタログの分類ごとに部品を並べる。
 * キャンバスへドラッグして入れるか、クリックで選んでいる場所に足す。画面に出ないオブジェクト(Timer)はクリックで足す。
 */
import { getCatalog, l10n, type Category } from '@tokotachi-designer/core';
import { useState } from 'react';
import { beginDrag, clickAdd } from '../dragController.ts';
import type { Drag } from '../dropTarget.ts';
import { labelGhost } from '../ghost.ts';
import { sendEdit } from '../store.ts';
import { ClassIcon } from './ClassIcon.tsx';

const CATEGORIES: readonly Category[] = ['input', 'display', 'view', 'container', 'drawing'];

function categoryLabel(c: Category | 'spacer' | 'non_visual'): string {
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
    case 'non_visual':
      return l10n.t('Objects');
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

export function Palette() {
  const [open, setOpen] = useState(true);
  const classes = Object.entries(getCatalog().classes);
  return (
    <div className="palette">
      <div
        className="section-head"
        onClick={() => {
          setOpen(!open);
        }}
      >
        {open ? '▾' : '▸'} {l10n.t('Palette')}
      </div>
      {open && (
        <>
          {CATEGORIES.map((c) => (
            <div key={c} className="palette-group">
              <div className="palette-category">{categoryLabel(c)}</div>
              {classes
                .filter(([, info]) => info.kind === 'widget' && info.category === c)
                .map(([cls, info]) => (
                  <Item
                    key={cls}
                    label={cls}
                    icon={cls}
                    title={info.doc}
                    drag={{ kind: 'new', class: cls }}
                  />
                ))}
            </div>
          ))}
          <div className="palette-group">
            <div className="palette-category">{categoryLabel('spacer')}</div>
            <Item
              label={l10n.t('Stretch')}
              icon="stretch"
              title={l10n.t('A stretch in a BoxLayout (add_stretch)')}
              drag={{ kind: 'spacer', spacer: { stretch: 1 } }}
            />
            <Item
              label={l10n.t('Spacing')}
              icon="spacing"
              title={l10n.t('A fixed spacing in a BoxLayout (add_spacing)')}
              drag={{ kind: 'spacer', spacer: { spacing: 12 } }}
            />
          </div>
          <div className="palette-group">
            <div className="palette-category">{categoryLabel('non_visual')}</div>
            {classes
              .filter(([, info]) => info.kind === 'object')
              .map(([cls, info]) => (
                <div
                  key={cls}
                  className="palette-item"
                  title={info.doc}
                  onClick={() => {
                    sendEdit({ kind: 'addObject', class: cls });
                  }}
                >
                  <ClassIcon name={cls} />
                  {cls}
                </div>
              ))}
          </div>
        </>
      )}
    </div>
  );
}
