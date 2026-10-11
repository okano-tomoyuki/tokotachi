/**
 * テーマの上書き(theme_override。docs/adr/0042)の入力欄(docs/designer/editor-design.md §5)。
 * 上書きする役割ごとに、ライト・ダークの色を選ぶ(同じなら 1 つの色として書く)。寸法は数値で、空なら継ぐ。
 */
import {
  colorRoles,
  l10n,
  parseThemeOverride,
  THEME_DIMENSIONS,
  type PropertyValue,
} from '@tokotachi-designer/core';

interface Props {
  /** 書いた値(無ければ undefined)。 */
  readonly value: PropertyValue | undefined;
  /** 確定した値(null なら消す)。 */
  readonly onCommit: (value: PropertyValue | null) => void;
}

interface Entry {
  readonly role: string;
  readonly light: string;
  readonly dark: string;
}

/** 新しく上書きする役割の最初の色 */
const INITIAL_COLOR = '#808080';

function build(
  colors: readonly Entry[],
  dimensions: Readonly<Record<string, number>>,
): PropertyValue | null {
  const out: Record<string, PropertyValue> = {};
  if (colors.length > 0) {
    out['colors'] = Object.fromEntries(
      colors.map((c) => [c.role, c.light === c.dark ? c.light : { light: c.light, dark: c.dark }]),
    );
  }
  for (const d of THEME_DIMENSIONS) {
    const v = dimensions[d];
    if (v !== undefined) {
      out[d] = v;
    }
  }
  return Object.keys(out).length === 0 ? null : out;
}

export function ThemeOverrideField({ value, onCommit }: Props) {
  const parsed = parseThemeOverride(value);
  const colors: Entry[] = parsed.colors.map((c) => ({ ...c }));
  const dimensions: Record<string, number> = Object.fromEntries(
    parsed.dimensions.map((d) => [d.name, d.value]),
  );
  const unused = colorRoles().filter((r) => !colors.some((c) => c.role === r));
  const commit = (nextColors: readonly Entry[], nextDimensions: Record<string, number>): void => {
    onCommit(build(nextColors, nextDimensions));
  };
  const setColor = (role: string, kind: 'light' | 'dark', color: string): void => {
    commit(
      colors.map((c) => (c.role === role ? { ...c, [kind]: color.toUpperCase() } : c)),
      dimensions,
    );
  };
  return (
    <div className="theme-override">
      {colors.map((c) => (
        <div key={c.role} className="theme-override-row">
          <span className="theme-override-role">{c.role}</span>
          <input
            type="color"
            title={l10n.t('Light')}
            value={c.light.slice(0, 7)}
            onChange={(e) => {
              setColor(c.role, 'light', e.target.value);
            }}
          />
          <input
            type="color"
            title={l10n.t('Dark')}
            value={c.dark.slice(0, 7)}
            onChange={(e) => {
              setColor(c.role, 'dark', e.target.value);
            }}
          />
          <button
            title={l10n.t('Stop overriding (inherit)')}
            onClick={() => {
              commit(
                colors.filter((x) => x.role !== c.role),
                dimensions,
              );
            }}
          >
            ×
          </button>
        </div>
      ))}
      {unused.length > 0 && (
        <select
          value=""
          onChange={(e) => {
            const role = e.target.value;
            if (role !== '') {
              commit([...colors, { role, light: INITIAL_COLOR, dark: INITIAL_COLOR }], dimensions);
            }
          }}
        >
          <option value="">{l10n.t('+ Override a color...')}</option>
          {unused.map((r) => (
            <option key={r} value={r}>
              {r}
            </option>
          ))}
        </select>
      )}
      {THEME_DIMENSIONS.map((d) => (
        <label key={d} className="theme-override-row">
          <span className="theme-override-role">{d}</span>
          <input
            type="number"
            min={0}
            placeholder={l10n.t('(inherit)')}
            // 値が外から変わったら作り直す(確定前の入力は残さない)
            key={String(dimensions[d])}
            defaultValue={dimensions[d] === undefined ? '' : String(dimensions[d])}
            onBlur={(e) => {
              const text = e.target.value.trim();
              const v = Number(text);
              const next = { ...dimensions };
              if (text === '' || !Number.isFinite(v) || v < 0) {
                Reflect.deleteProperty(next, d);
              } else {
                next[d] = v;
              }
              if (next[d] !== dimensions[d]) {
                commit(colors, next);
              }
            }}
            onKeyDown={(e) => {
              if (e.key === 'Enter') {
                e.currentTarget.blur();
              }
            }}
          />
        </label>
      ))}
    </div>
  );
}
