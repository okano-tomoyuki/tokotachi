/**
 * 右クリックのメニュー(docs/designer/editor-design.md §5)。開いたときに渡した項目(treeOps.ts)を並べる。
 * 外側のクリック・Esc・項目の実行で閉じる。上下の矢印で項目を移り、Enter で実行する。
 */
import { useEffect, useLayoutEffect, useRef, useState, type KeyboardEvent } from 'react';
import { closeContextMenu, useDesigner } from '../store.ts';

export function ContextMenu() {
  const menu = useDesigner((s) => s.contextMenu);
  const ref = useRef<HTMLDivElement>(null);
  const [position, setPosition] = useState<{ x: number; y: number } | undefined>(undefined);

  // 画面の端からはみ出さないよう、開いた後に位置を直す
  useLayoutEffect(() => {
    const element = ref.current;
    if (!menu || !element) {
      setPosition(undefined);
      return;
    }
    const r = element.getBoundingClientRect();
    setPosition({
      x: Math.max(0, Math.min(menu.x, window.innerWidth - r.width - 4)),
      y: Math.max(0, Math.min(menu.y, window.innerHeight - r.height - 4)),
    });
    element.querySelector<HTMLButtonElement>('button:not(:disabled)')?.focus();
  }, [menu]);

  useEffect(() => {
    if (!menu) {
      return;
    }
    const onPointerDown = (e: PointerEvent): void => {
      if (!ref.current?.contains(e.target as Node)) {
        closeContextMenu();
      }
    };
    const onKeyDown = (e: globalThis.KeyboardEvent): void => {
      if (e.key === 'Escape') {
        e.stopPropagation();
        closeContextMenu();
      }
    };
    window.addEventListener('pointerdown', onPointerDown, true);
    window.addEventListener('keydown', onKeyDown, true);
    window.addEventListener('blur', closeContextMenu);
    return () => {
      window.removeEventListener('pointerdown', onPointerDown, true);
      window.removeEventListener('keydown', onKeyDown, true);
      window.removeEventListener('blur', closeContextMenu);
    };
  }, [menu]);

  if (!menu) {
    return null;
  }
  const shown = position ?? menu;
  const onKeyDown = (e: KeyboardEvent<HTMLDivElement>): void => {
    if (e.key !== 'ArrowDown' && e.key !== 'ArrowUp') {
      return;
    }
    e.preventDefault();
    const items = [
      ...(ref.current?.querySelectorAll<HTMLButtonElement>('button:not(:disabled)') ?? []),
    ];
    const current = items.indexOf(document.activeElement as HTMLButtonElement);
    const next = (current + (e.key === 'ArrowDown' ? 1 : -1) + items.length) % items.length;
    items[next]?.focus();
  };
  return (
    <div
      ref={ref}
      className="context-menu"
      role="menu"
      style={{ left: shown.x, top: shown.y }}
      onKeyDown={onKeyDown}
      onContextMenu={(e) => {
        e.preventDefault();
      }}
    >
      {menu.items.map((item, i) => (
        <div key={`${String(i)}:${item.label}`}>
          {item.separator && <hr />}
          <button
            type="button"
            role="menuitem"
            title={item.title}
            disabled={item.disabled}
            onClick={() => {
              closeContextMenu();
              item.run();
            }}
          >
            <span>{item.label}</span>
            {item.shortcut !== undefined && (
              <span className="context-menu-hint">{item.shortcut}</span>
            )}
          </button>
        </div>
      ))}
    </div>
  );
}
