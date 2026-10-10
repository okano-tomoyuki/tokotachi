/**
 * インスペクタのレイアウトの欄(docs/designer/editor-design.md §5)。
 * - 入れ物(部品・ウィンドウ): レイアウトの種類(無し・BoxLayout・GridLayout・PlaceLayout)。
 * - レイアウトの中の部品・空き: 項目の設定(BoxLayout は伸びる割合・そろえ方、GridLayout は行と列・またがる数・そろえ方、
 *   PlaceLayout は置き方)。
 */
import {
  findClass,
  getCatalog,
  l10n,
  valueAt,
  type OutlineNode,
  type PropertyInfo,
  type PropertyType,
  type PropertyValue,
} from '@tokotachi-designer/core';
import { sendEdit, useDesigner } from '../store.ts';
import { PropertyField } from './PropertyField.tsx';

interface Json {
  readonly [key: string]: unknown;
}

const LAYOUTS = ['BoxLayout', 'GridLayout', 'PlaceLayout'];

/** 項目の設定の型と既定値(add の既定の引数)。 */
const ITEM_FIELDS: Readonly<Record<string, readonly [string, PropertyType, PropertyValue][]>> = {
  BoxLayout: [
    ['stretch', 'int', 0],
    ['alignment', { enum: 'Align' }, 'FILL'],
  ],
  GridLayout: [
    ['row', 'int', 0],
    ['column', 'int', 0],
    ['row_span', 'int', 1],
    ['column_span', 'int', 1],
    ['horizontal', { enum: 'Align' }, 'FILL'],
    ['vertical', { enum: 'Align' }, 'CENTER'],
  ],
};

function info(type: PropertyType, value: PropertyValue): PropertyInfo {
  return { type, declared_in: '', default: value };
}

/** 入れ物のレイアウトの種類。 */
function LayoutKind({ node }: { node: OutlineNode }) {
  const document = useDesigner((s) => s.document);
  const frame = useDesigner((s) => s.frame);
  const json = valueAt(document, node.path) as Json | undefined;
  const layout = json?.['layout'] as Json | undefined;
  const current = typeof layout?.['class'] === 'string' ? layout['class'] : '';
  return (
    <tr>
      <th>layout</th>
      <td>
        <select
          value={current}
          onChange={(e) => {
            const cls = e.target.value === '' ? null : e.target.value;
            // 自由な配置・PlaceLayout には、今描いている位置で置く(入れ物の左上から)
            const origin =
              node.variable === undefined
                ? { x: 0, y: 0 }
                : (frame?.rects[node.variable] ?? { x: 0, y: 0 });
            const positions: Record<string, { x: number; y: number }> = {};
            for (const [name, r] of Object.entries(frame?.rects ?? {})) {
              positions[name] = { x: r.x - origin.x, y: r.y - origin.y };
            }
            sendEdit({ kind: 'setLayout', target: node.path, layout: cls, positions });
          }}
        >
          <option value="">{l10n.t('(none: place freely)')}</option>
          {LAYOUTS.map((l) => (
            <option key={l} value={l}>
              {l}
            </option>
          ))}
        </select>
      </td>
    </tr>
  );
}

/** レイアウトの項目の設定。 */
function ItemFields({ itemPath }: { itemPath: readonly (string | number)[] }) {
  const document = useDesigner((s) => s.document);
  const item = valueAt(document, itemPath) as Json | undefined;
  const layout = valueAt(document, itemPath.slice(0, -2)) as Json | undefined;
  if (!item || !layout) {
    return null;
  }
  const cls = String(layout['class']);
  const set = (name: string, v: PropertyValue | null): void => {
    sendEdit({ kind: 'setItem', target: itemPath, values: { [name]: v } });
  };
  // 空き
  if (item['widget'] === undefined) {
    const key = item['spacing'] !== undefined ? 'spacing' : 'stretch';
    return (
      <tr>
        <th className="set">{key}</th>
        <td>
          <PropertyField
            name={key}
            info={info(key === 'spacing' ? 'float' : 'int', key === 'spacing' ? 12 : 1)}
            value={item[key] as PropertyValue | undefined}
            onCommit={(v) => {
              if (v !== null) {
                set(key, v);
              }
            }}
          />
        </td>
      </tr>
    );
  }
  if (cls === 'PlaceLayout') {
    const placement = (item['placement'] as Record<string, PropertyValue> | undefined) ?? {};
    const members = getCatalog().values['Placement']?.members ?? {};
    return (
      <>
        {Object.entries(members).map(([name, t]) => (
          <tr key={name}>
            <th className={placement[name] === undefined ? '' : 'set'}>{name}</th>
            <td>
              <PropertyField
                name={name}
                info={info(t, name === 'anchor' ? 'TOP_LEFT' : 0)}
                value={placement[name]}
                onCommit={(v) => {
                  const next = Object.fromEntries(
                    Object.entries({ ...placement, [name]: v }).filter(([, x]) => x !== null),
                  );
                  set('placement', Object.keys(next).length > 0 ? next : null);
                }}
              />
            </td>
          </tr>
        ))}
      </>
    );
  }
  return (
    <>
      {(ITEM_FIELDS[cls] ?? []).map(([name, type, fallback]) => (
        <tr key={name}>
          <th className={item[name] === undefined ? '' : 'set'}>{name}</th>
          <td>
            <PropertyField
              name={name}
              info={info(type, fallback)}
              value={item[name] as PropertyValue | undefined}
              onCommit={(v) => {
                // GridLayout の行と列は消せない(既定に戻すと 0 にする)
                set(name, v === null && (name === 'row' || name === 'column') ? 0 : v);
              }}
            />
          </td>
        </tr>
      ))}
    </>
  );
}

/** 選んだノードに合わせた、レイアウトの欄(無ければ何も出さない)。 */
export function LayoutSection({ node }: { node: OutlineNode }) {
  const path = node.path;
  const inItem =
    (node.kind === 'widget' &&
      path[path.length - 1] === 'widget' &&
      path[path.length - 3] === 'items') ||
    node.kind === 'spacer';
  const itemPath = node.kind === 'spacer' ? path : path.slice(0, -1);
  const container =
    node.kind === 'window' ||
    (node.kind === 'widget' &&
      node.class !== undefined &&
      findClass(node.class)?.accepts_children === true);
  if (!inItem && !container) {
    return null;
  }
  return (
    <>
      <div className="section-head">{l10n.t('Layout')}</div>
      <table className="grid">
        <tbody>
          {container && <LayoutKind node={node} />}
          {inItem && <ItemFields itemPath={itemPath} />}
        </tbody>
      </table>
    </>
  );
}
