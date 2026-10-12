import { describe, expect, it } from 'vitest';

import { parseDocument } from './parse.ts';
import { renameOldMembers } from './renamed.ts';

describe('renameOldMembers', () => {
  it('Label の color と Timer の on_timer を、新しい名前に読み替える(キーの順は保つ)', () => {
    const json = {
      window: {
        name: 'MainWindow',
        class: 'Window',
        children: [
          {
            name: 'label_title',
            class: 'Label',
            properties: { text: 'a', color: 'TEXT_SECONDARY', word_wrap: true },
          },
        ],
      },
      objects: [{ name: 'timer_refresh', class: 'Timer', events: { on_timer: 'refresh' } }],
    };
    expect(renameOldMembers(json)).toBe(true);
    expect(json.window.children.map((c) => Object.keys(c.properties))).toEqual([
      ['text', 'text_color', 'word_wrap'],
    ]);
    expect(json.objects.map((o) => o.events)).toEqual([{ on_timeout: 'refresh' }]);
    expect(renameOldMembers(json)).toBe(false);
  });

  it('ほかのクラスの同じ名前(ColorButton の color)は変えない', () => {
    const json = { class: 'ColorButton', properties: { color: '#FF0000' } };
    expect(renameOldMembers(json)).toBe(false);
    expect(json.properties).toEqual({ color: '#FF0000' });
  });

  it('古い名前で書いた定義ファイルも読める', () => {
    const text = JSON.stringify({
      format_version: 1,
      window: {
        name: 'MainWindow',
        class: 'Window',
        properties: { title: 't' },
        layout: {
          class: 'BoxLayout',
          items: [
            {
              widget: {
                name: 'label_title',
                class: 'Label',
                properties: { text: 'a', color: 'TEXT_SECONDARY' },
              },
            },
          ],
        },
      },
      objects: [{ name: 'timer_refresh', class: 'Timer', events: { on_timer: 'refresh' } }],
    });
    const result = parseDocument(text);
    expect(result.diagnostics.filter((d) => d.severity === 'error')).toEqual([]);
    expect(JSON.stringify(result.document)).toContain('"text_color":"TEXT_SECONDARY"');
    expect(JSON.stringify(result.document)).toContain('"on_timeout":"refresh"');
  });
});
