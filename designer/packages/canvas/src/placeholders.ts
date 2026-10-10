/**
 * デザイナーの中でだけ、中身が空の入れ物に最小の大きさを与える(docs/designer/editor-design.md §4)。
 * 空の入れ物はレイアウトの中で大きさが 0 になり、見えず、部品を入れる場所にもならないため。
 * 書いた minimum_size が無いものだけに与えるので、中身を入れれば実物と同じ大きさになる。
 */
import { findClass, widgets, type TtuiDocument, type WidgetNode } from '@tokotachi-designer/core';

export const PLACEHOLDER_SIZE = { width: 80, height: 48 };

function isEmptyContainer(node: WidgetNode): boolean {
  const info = findClass(node.class);
  if (!info?.accepts_children && !info?.child_keys) {
    return false;
  }
  return (
    (node.layout?.items ?? []).length === 0 &&
    (node.children ?? []).length === 0 &&
    (node.tabs ?? []).length === 0 &&
    (node.panes ?? []).length === 0 &&
    node.content === undefined
  );
}

/** 空の入れ物に minimum_size を足した写し。 */
export function withPlaceholders(doc: TtuiDocument): TtuiDocument {
  const copy = JSON.parse(JSON.stringify(doc)) as TtuiDocument;
  for (const e of widgets(copy)) {
    const node = e.node as WidgetNode & { properties?: Record<string, unknown> };
    if (isEmptyContainer(node) && node.properties?.['minimum_size'] === undefined) {
      (node as { properties?: Record<string, unknown> }).properties = {
        ...(node.properties ?? {}),
        minimum_size: PLACEHOLDER_SIZE,
      };
    }
  }
  return copy;
}
