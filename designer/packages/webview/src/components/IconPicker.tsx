/**
 * よく使うアイコン(docs/adr/0054)を選ぶ。画像のプロパティの欄の横のボタンで開き、分類ごとに並べた中から選ぶと `"icon:<名前>"` にする。
 * 名前(定義ファイルの名前・Lucide の名前)で絞り込める。開いた画面はペインに見切れないよう、画面全体に重ねて画面の中に収める。
 */
import { ICONS, iconOf, l10n, type IconInfo } from '@tokotachi-designer/core';
import { useEffect, useRef, useState } from 'react';
import { createPortal } from 'react-dom';

/** アイコンの線画(24 × 24 の格子。文字の色で描く)。 */
export function IconGlyph({ icon, size = 16 }: { icon: IconInfo; size?: number }) {
  return (
    <svg
      className="icon-glyph"
      width={size}
      height={size}
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth={2}
      strokeLinecap="round"
      strokeLinejoin="round"
      aria-hidden="true"
    >
      <path d={icon.path} />
    </svg>
  );
}

/** 分類の表示(icons.json の分類は英語)。 */
function categoryLabel(category: string): string {
  switch (category) {
    case 'File':
      return l10n.t('File');
    case 'Edit':
      return l10n.t('Edit');
    case 'Navigation':
      return l10n.t('Navigation');
    case 'View':
      return l10n.t('View');
    case 'Status':
      return l10n.t('Status');
    case 'Media':
      return l10n.t('Media');
    default:
      return l10n.t('Other');
  }
}

/** 開いた画面の大きさ(px)。画面に収まらなければ縮める。 */
const PANEL_WIDTH = 280;
const PANEL_HEIGHT = 360;
const MARGIN = 8;

/** 開く位置(画面の座標)。ボタンの下に右端を揃えて置き、下に収まらなければ上に置く。画面の外には出さない。 */
function panelPlace(button: DOMRect): { left: number; top: number; width: number; height: number } {
  const vw = window.innerWidth;
  const vh = window.innerHeight;
  const width = Math.min(PANEL_WIDTH, vw - 2 * MARGIN);
  const left = Math.max(MARGIN, Math.min(button.right - width, vw - MARGIN - width));
  const below = vh - MARGIN - button.bottom - 2;
  const above = button.top - MARGIN - 2;
  if (below >= Math.min(PANEL_HEIGHT, 200) || below >= above) {
    return { left, top: button.bottom + 2, width, height: Math.min(PANEL_HEIGHT, below) };
  }
  const height = Math.min(PANEL_HEIGHT, above);
  return { left, top: button.top - 2 - height, width, height };
}

export function IconPicker({
  value,
  onPick,
}: {
  /** 今の値(アイコンなら、そのアイコンを選んだ印にする)。 */
  value: unknown;
  onPick: (value: string) => void;
}) {
  // 開いているときの位置(ペインに見切れないよう、画面全体に重ねて置く)
  const [place, setPlace] = useState<ReturnType<typeof panelPlace> | undefined>(undefined);
  const [filter, setFilter] = useState('');
  const buttonRef = useRef<HTMLButtonElement>(null);
  const panelRef = useRef<HTMLDivElement>(null);
  const current = iconOf(value);
  const open = place !== undefined;

  useEffect(() => {
    if (!open) {
      return;
    }
    // 外を押す・Esc・画面の大きさが変わる・ペインをスクロールすると閉じる
    const close = (): void => {
      setPlace(undefined);
    };
    const down = (e: MouseEvent): void => {
      const t = e.target as Node;
      if (!panelRef.current?.contains(t) && !buttonRef.current?.contains(t)) {
        close();
      }
    };
    const key = (e: KeyboardEvent): void => {
      if (e.key === 'Escape') {
        close();
      }
    };
    const scroll = (e: Event): void => {
      if (!panelRef.current?.contains(e.target as Node)) {
        close();
      }
    };
    window.addEventListener('mousedown', down);
    window.addEventListener('keydown', key);
    window.addEventListener('resize', close);
    window.addEventListener('scroll', scroll, true);
    return () => {
      window.removeEventListener('mousedown', down);
      window.removeEventListener('keydown', key);
      window.removeEventListener('resize', close);
      window.removeEventListener('scroll', scroll, true);
    };
  }, [open]);

  const words = filter.trim().toLowerCase();
  const shown = ICONS.filter(
    (i) => words === '' || i.name.includes(words.replace(/-/g, '_')) || i.lucide.includes(words),
  );
  const categories = [...new Set(shown.map((i) => i.category))];
  return (
    <div className="icon-picker">
      <button
        ref={buttonRef}
        title={l10n.t('Choose an icon')}
        className={current ? 'icon-picker-button on' : 'icon-picker-button'}
        onClick={() => {
          const r = buttonRef.current?.getBoundingClientRect();
          setPlace(open || !r ? undefined : panelPlace(r));
          setFilter('');
        }}
      >
        {current ? <IconGlyph icon={current} /> : '☆'}
      </button>
      {place &&
        createPortal(
          <div
            ref={panelRef}
            className="icon-picker-panel"
            style={{ left: place.left, top: place.top, width: place.width, height: place.height }}
          >
            <input
              type="text"
              autoFocus
              placeholder={l10n.t('Filter icons')}
              value={filter}
              onChange={(e) => {
                setFilter(e.target.value);
              }}
            />
            <div className="icon-picker-list">
              {categories.map((category) => (
                <div key={category}>
                  <div className="icon-picker-category">{categoryLabel(category)}</div>
                  <div className="icon-picker-grid">
                    {shown
                      .filter((i) => i.category === category)
                      .map((i) => (
                        <button
                          key={i.name}
                          className={i === current ? 'icon-cell selected' : 'icon-cell'}
                          title={`${i.name} (${i.lucide})`}
                          onClick={() => {
                            onPick(`icon:${i.name}`);
                            setPlace(undefined);
                          }}
                        >
                          <IconGlyph icon={i} size={20} />
                        </button>
                      ))}
                  </div>
                </div>
              ))}
              {shown.length === 0 && <div className="icon-picker-empty">{l10n.t('No icons')}</div>}
            </div>
          </div>,
          document.body,
        )}
    </div>
  );
}
