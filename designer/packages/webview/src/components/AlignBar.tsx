/**
 * 整列のボタン(docs/designer/editor-design.md §3.8)。自由な配置の部品を 2 つ以上選んでいるときに押せる。
 * 基準は主に選んでいる部品。位置と大きさはキャンバスの描画の結果を使う。
 */
import {
  ALIGN_OPERATIONS,
  alignCommands,
  canAlign,
  l10n,
  outermostPaths,
  type AlignOperation,
  type AlignTarget,
} from '@tokotachi-designer/core';
import type { ReactNode } from 'react';
import { selectedNodes, sendEdit, useDesigner } from '../store.ts';

function label(operation: AlignOperation): string {
  switch (operation) {
    case 'left':
      return l10n.t('Align left edges');
    case 'center_horizontal':
      return l10n.t('Align horizontal centers');
    case 'right':
      return l10n.t('Align right edges');
    case 'top':
      return l10n.t('Align top edges');
    case 'center_vertical':
      return l10n.t('Align vertical centers');
    case 'bottom':
      return l10n.t('Align bottom edges');
    case 'distribute_horizontal':
      return l10n.t('Distribute horizontally (3 or more)');
    case 'distribute_vertical':
      return l10n.t('Distribute vertically (3 or more)');
    case 'same_width':
      return l10n.t('Make the same width');
    case 'same_height':
      return l10n.t('Make the same height');
  }
}

/** 16×16 の線画のアイコン(線は文字の色)。 */
const ICONS: Readonly<Record<AlignOperation, ReactNode>> = {
  left: (
    <>
      <path d="M2.5 1.5v13" />
      <rect x="4.5" y="3.5" width="8" height="3" />
      <rect x="4.5" y="9.5" width="5" height="3" />
    </>
  ),
  center_horizontal: (
    <>
      <path d="M8 1.5v13" />
      <rect x="3.5" y="3.5" width="9" height="3" />
      <rect x="5.5" y="9.5" width="5" height="3" />
    </>
  ),
  right: (
    <>
      <path d="M13.5 1.5v13" />
      <rect x="3.5" y="3.5" width="8" height="3" />
      <rect x="6.5" y="9.5" width="5" height="3" />
    </>
  ),
  top: (
    <>
      <path d="M1.5 2.5h13" />
      <rect x="3.5" y="4.5" width="3" height="8" />
      <rect x="9.5" y="4.5" width="3" height="5" />
    </>
  ),
  center_vertical: (
    <>
      <path d="M1.5 8h13" />
      <rect x="3.5" y="3.5" width="3" height="9" />
      <rect x="9.5" y="5.5" width="3" height="5" />
    </>
  ),
  bottom: (
    <>
      <path d="M1.5 13.5h13" />
      <rect x="3.5" y="3.5" width="3" height="8" />
      <rect x="9.5" y="6.5" width="3" height="5" />
    </>
  ),
  distribute_horizontal: (
    <>
      <path d="M1.5 2v12M14.5 2v12" />
      <rect x="3.5" y="5.5" width="2" height="5" />
      <rect x="7" y="5.5" width="2" height="5" />
      <rect x="10.5" y="5.5" width="2" height="5" />
    </>
  ),
  distribute_vertical: (
    <>
      <path d="M2 1.5h12M2 14.5h12" />
      <rect x="5.5" y="3.5" width="5" height="2" />
      <rect x="5.5" y="7" width="5" height="2" />
      <rect x="5.5" y="10.5" width="5" height="2" />
    </>
  ),
  same_width: (
    <>
      <rect x="3.5" y="2.5" width="9" height="4" />
      <rect x="3.5" y="9.5" width="9" height="4" />
      <path d="M1.5 1.5v13M14.5 1.5v13" />
    </>
  ),
  same_height: (
    <>
      <rect x="2.5" y="3.5" width="4" height="9" />
      <rect x="9.5" y="3.5" width="4" height="9" />
      <path d="M1.5 1.5h13M1.5 14.5h13" />
    </>
  ),
};

/** 区切りを入れる位置(左右・上下・等間隔・大きさのまとまり)。 */
const GROUP_START: ReadonlySet<AlignOperation> = new Set([
  'top',
  'distribute_horizontal',
  'same_width',
]);

/** 選んでいる部品と、その位置と大きさ。基準(主に選んでいる部品)の位置も返す。 */
function targets(): { targets: AlignTarget[]; anchor: number } {
  const state = useDesigner.getState();
  const { document, frame } = state;
  if (!document || !frame) {
    return { targets: [], anchor: 0 };
  }
  const widgets = selectedNodes(state).filter((n) => n.kind === 'widget');
  const paths = new Set(
    outermostPaths(
      document,
      widgets.map((n) => n.path),
    ).map((p) => p.join('/')),
  );
  const out: AlignTarget[] = [];
  let anchor = 0;
  for (const n of widgets) {
    const r = n.variable === undefined ? undefined : frame.rects[n.variable];
    if (!r || !paths.has(n.path.join('/'))) {
      continue;
    }
    if (n.id === state.selection) {
      anchor = out.length;
    }
    out.push({ path: n.path, rect: r });
  }
  return { targets: out, anchor };
}

export function AlignBar() {
  // 選択・ドキュメント・描画が変わったら描き直す
  useDesigner((s) => s.selected);
  useDesigner((s) => s.frame);
  const document = useDesigner((s) => s.document);
  const current = targets();
  const paths = current.targets.map((t) => t.path);
  return (
    <div className="align-bar" role="toolbar" aria-label={l10n.t('Align')}>
      {ALIGN_OPERATIONS.map((op) => {
        const enabled = document !== undefined && canAlign(document, paths, op);
        return (
          <button
            key={op}
            className={`icon-button${GROUP_START.has(op) ? ' group-start' : ''}`}
            title={
              enabled
                ? label(op)
                : `${label(op)} — ${l10n.t('Select two or more freely placed widgets')}`
            }
            disabled={!enabled}
            onClick={() => {
              const { targets: t, anchor } = targets();
              if (!document) {
                return;
              }
              const commands = alignCommands(document, t, anchor, op);
              if (commands.length > 0) {
                // 揃えた後も同じ部品を選ぶ(主に選んでいた部品を最初に)
                const state = useDesigner.getState();
                const nodes = selectedNodes(state);
                const primary = nodes.find((n) => n.id === state.selection);
                const names = [
                  ...new Set([...(primary ? [primary.label] : []), ...nodes.map((n) => n.label)]),
                ];
                sendEdit({ kind: 'batch', commands }, names);
              }
            }}
          >
            <svg
              width="16"
              height="16"
              viewBox="0 0 16 16"
              fill="none"
              stroke="currentColor"
              strokeWidth="1"
            >
              {ICONS[op]}
            </svg>
          </button>
        );
      })}
    </div>
  );
}
