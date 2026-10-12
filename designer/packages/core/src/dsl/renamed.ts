/**
 * Tokotachi の API の改名で名前の変わったプロパティ・ハンドラを、定義ファイルを読むときに新しい名前に読み替える。
 * 古い名前で書いたファイルもそのまま開け、保存すると新しい名前で書き出す(形式の版は変えない)。
 */

/** クラスごとの、古い名前から新しい名前への対応(プロパティとハンドラ)。 */
const RENAMED: Readonly<
  Record<
    string,
    {
      readonly properties?: Readonly<Record<string, string>>;
      readonly events?: Readonly<Record<string, string>>;
    }
  >
> = {
  // Label の文字の色(Item::text_color と揃えた)
  Label: { properties: { color: 'text_color' } },
  // Timer の間隔が経ったときのハンドラ(上書きする仮想関数 timeout と揃えた)
  Timer: { events: { on_timer: 'on_timeout' } },
};

/** JSON の中の部品・オブジェクト(class を持つもの)の古い名前を、その場で新しい名前にする。読み替えたら true。 */
export function renameOldMembers(json: unknown): boolean {
  let changed = false;
  const visit = (value: unknown): void => {
    if (Array.isArray(value)) {
      value.forEach(visit);
      return;
    }
    if (typeof value !== 'object' || value === null) {
      return;
    }
    const node = value as Record<string, unknown>;
    const renames = typeof node['class'] === 'string' ? RENAMED[node['class']] : undefined;
    if (renames) {
      for (const [key, names] of [
        ['properties', renames.properties],
        ['events', renames.events],
      ] as const) {
        const renamed = renameKeys(node[key], names);
        if (renamed) {
          node[key] = renamed;
          changed = true;
        }
      }
    }
    Object.values(node).forEach(visit);
  };
  visit(json);
  return changed;
}

/** names の古い名前を新しい名前にした写し(キーの順は保つ)。読み替えるものが無ければ undefined。 */
function renameKeys(
  target: unknown,
  names: Readonly<Record<string, string>> | undefined,
): Record<string, unknown> | undefined {
  if (!names || typeof target !== 'object' || target === null || Array.isArray(target)) {
    return undefined;
  }
  const record = target as Record<string, unknown>;
  const keys = Object.keys(record);
  if (!keys.some((k) => k in names && !((names[k] as string) in record))) {
    return undefined;
  }
  return Object.fromEntries(
    keys.map((k) => [
      k in names && !((names[k] as string) in record) ? (names[k] as string) : k,
      record[k],
    ]),
  );
}
