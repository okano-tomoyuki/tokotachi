import { getCatalog, outline, parseDocument, type OutlineNode } from '@tokotachi-designer/core';
import { describe, expect, it } from 'vitest';
import mainWindow from '../../../../samples/main_window.ttui.json?raw';
import settingsDialog from '../../../../samples/settings_dialog.ttui.json?raw';
import { hasClassIcon, nodeIconKey } from './ClassIcon.tsx';

function flatten(nodes: readonly OutlineNode[]): OutlineNode[] {
  return nodes.flatMap((n) => [n, ...flatten(n.children)]);
}

describe('ClassIcon', () => {
  it('カタログのすべてのクラスにアイコンがある', () => {
    const missing = Object.keys(getCatalog().classes).filter((name) => !hasClassIcon(name));
    expect(missing).toEqual([]);
  });

  it('見本の構造の木のすべてのノードにアイコンがある', () => {
    for (const text of [mainWindow, settingsDialog]) {
      const doc = parseDocument(text).document;
      const keys = doc ? flatten(outline(doc)).map(nodeIconKey) : ['(読めない)'];
      expect(keys.filter((k) => !hasClassIcon(k))).toEqual([]);
    }
  });
});
