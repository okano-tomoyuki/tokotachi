/**
 * プロパティの入力欄(docs/designer/editor-design.md §5)。型(カタログ)に合わせた入力欄を出す。
 * 文字の入力は Enter かフォーカスを外したときに確定し、Esc で戻す。選択肢・チェックは変えた時点で確定する。
 * 空にすると(または × で)書いた値を消して既定値に戻す。
 */
import {
  checkValue,
  COLOR_PATTERN,
  getCatalog,
  l10n,
  type PropertyInfo,
  type PropertyValue,
} from '@tokotachi-designer/core';
import { useEffect, useState } from 'react';

interface Props {
  readonly name: string;
  readonly info: PropertyInfo;
  /** 書いた値(無ければ undefined)。 */
  readonly value: PropertyValue | undefined;
  /** 確定した値(null なら消す)。 */
  readonly onCommit: (value: PropertyValue | null) => void;
  /** 画像のファイルを選んでもらう(画像のプロパティだけ)。 */
  readonly onPickImage?: (() => void) | undefined;
}

interface Codec {
  readonly format: (v: PropertyValue) => string;
  readonly parse: (text: string) => PropertyValue | undefined;
}

/** 値の文字(文字列・数値・真偽はそのまま、それ以外は JSON)。 */
function valueText(v: PropertyValue | undefined): string {
  return typeof v === 'string' || typeof v === 'number' || typeof v === 'boolean'
    ? String(v)
    : JSON.stringify(v ?? null);
}

/** 文字の入力欄で扱う型の、表示と読み取り。読めなければ undefined。 */
function codec(info: PropertyInfo): Codec {
  const t = info.type;
  const number = (text: string): number | undefined => {
    const v = Number(text.trim());
    return text.trim() === '' || !Number.isFinite(v) ? undefined : v;
  };
  if (t === 'int' || t === 'float' || t === 'double') {
    return { format: valueText, parse: (s) => number(s) };
  }
  if (typeof t === 'object' && 'array' in t) {
    if (t.array === 'string') {
      return {
        format: (v) => (Array.isArray(v) ? v.map(valueText).join('\n') : ''),
        parse: (s) => s.split('\n'),
      };
    }
    return {
      format: (v) => (Array.isArray(v) ? v.map(valueText).join(', ') : ''),
      parse: (s) => {
        const items = s.split(',').map(number);
        return items.every((x) => x !== undefined) ? items : undefined;
      },
    };
  }
  if (typeof t === 'object' && 'value' in t) {
    const members = Object.keys(getCatalog().values[t.value]?.members ?? {});
    if (members.length > 0) {
      // Margins は数値 1 つ(4 辺とも同じ)か、メンバの順にカンマで区切る(12, 8, 12, 8)
      return {
        format: (v) =>
          typeof v === 'number'
            ? String(v)
            : members
                .map((m) => valueText((v as Record<string, PropertyValue>)[m] ?? 0))
                .join(', '),
        parse: (s) => {
          const parts = s.split(',').map(number);
          if (parts.some((p) => p === undefined)) {
            return undefined;
          }
          if (t.value === 'Margins' && parts.length === 1) {
            return parts[0];
          }
          if (parts.length !== members.length) {
            return undefined;
          }
          return Object.fromEntries(members.map((m, i) => [m, parts[i] ?? 0]));
        },
      };
    }
  }
  return { format: valueText, parse: (s) => s };
}

function placeholderOf(info: PropertyInfo, c: Codec): string {
  return info.default === undefined ? '' : c.format(info.default as PropertyValue);
}

/** 文字の入力欄(1 行か複数行)。 */
function TextField({ info, value, onCommit, multiline }: Props & { multiline: boolean }) {
  const c = codec(info);
  const shown = value === undefined ? '' : c.format(value);
  const [text, setText] = useState(shown);
  const [error, setError] = useState<string | undefined>();
  useEffect(() => {
    setText(shown);
    setError(undefined);
  }, [shown]);
  const commit = (): void => {
    if (text === shown) {
      return;
    }
    if (text === '' && !(info.type === 'string' && multiline)) {
      onCommit(null);
      return;
    }
    const parsed = c.parse(text);
    const problem = parsed === undefined ? l10n.t('Invalid value') : checkValue(info.type, parsed);
    if (problem !== null) {
      setError(problem);
      return;
    }
    setError(undefined);
    onCommit(parsed ?? null);
  };
  const common = {
    value: text,
    placeholder: placeholderOf(info, c),
    className: error === undefined ? '' : 'invalid',
    title: error,
    onChange: (e: React.ChangeEvent<HTMLInputElement | HTMLTextAreaElement>) => {
      setText(e.target.value);
    },
    onBlur: commit,
    onKeyDown: (e: React.KeyboardEvent) => {
      if (e.key === 'Escape') {
        setText(shown);
        setError(undefined);
      } else if (e.key === 'Enter' && !(multiline && e.shiftKey)) {
        e.preventDefault();
        commit();
      }
    },
  };
  return multiline ? <textarea rows={3} {...common} /> : <input type="text" {...common} />;
}

export function PropertyField(props: Props) {
  const { info, value, onCommit } = props;
  const t = info.type;
  const catalog = getCatalog();
  if (t === 'bool') {
    const checked = (value ?? info.default) === true;
    return (
      <input
        type="checkbox"
        checked={checked}
        onChange={(e) => {
          onCommit(e.target.checked === info.default ? null : e.target.checked);
        }}
      />
    );
  }
  if (typeof t === 'object' && 'enum' in t) {
    const values = catalog.enums[t.enum]?.values ?? [];
    return (
      <select
        value={typeof value === 'string' ? value : ''}
        onChange={(e) => {
          onCommit(e.target.value === '' ? null : e.target.value);
        }}
      >
        <option value="">{`(${typeof info.default === 'string' ? info.default : ''})`}</option>
        {values.map((v) => (
          <option key={v} value={v}>
            {v}
          </option>
        ))}
      </select>
    );
  }
  if (typeof t === 'object' && 'value' in t && t.value === 'ThemeColor') {
    // 役割(テーマに従う)か、具体的な色(#RRGGBB)
    const roles = catalog.enums['ColorRole']?.values ?? [];
    const text = typeof value === 'string' ? value : '';
    const isColor = COLOR_PATTERN.test(text);
    return (
      <div className="color-field">
        <select
          value={isColor ? '#' : text}
          onChange={(e) => {
            const v = e.target.value;
            onCommit(v === '' ? null : v === '#' ? '#000000' : v);
          }}
        >
          <option value="">{`(${typeof info.default === 'string' ? info.default : ''})`}</option>
          {roles.map((r) => (
            <option key={r} value={r}>
              {r}
            </option>
          ))}
          <option value="#">{l10n.t('Color...')}</option>
        </select>
        {isColor && (
          <input
            type="color"
            value={text.slice(0, 7)}
            onChange={(e) => {
              onCommit(e.target.value.toUpperCase());
            }}
          />
        )}
      </div>
    );
  }
  if (typeof t === 'object' && 'value' in t && t.value === 'Image') {
    // 定義ファイルからの相対パス。「…」でファイルを選ぶ
    return (
      <div className="image-field">
        <TextField {...props} multiline={false} />
        {props.onPickImage && (
          <button title={l10n.t('Choose an image file')} onClick={props.onPickImage}>
            …
          </button>
        )}
      </div>
    );
  }
  const multiline =
    info.editor === 'multiline' || (typeof t === 'object' && 'array' in t && t.array === 'string');
  return <TextField {...props} multiline={multiline} />;
}
