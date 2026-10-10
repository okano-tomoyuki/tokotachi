import { describe, expect, it } from 'vitest';
import { validateDocument } from '../dsl/validate.ts';
import { SAMPLE } from '../testing.ts';
import { applyCommand, type EditCommand } from './commands.ts';

const SEARCH_BUTTON = ['window', 'layout', 'items', 0, 'widget', 'layout', 'items', 1, 'widget'];
const NAME_EDIT_TEXT = (doc: any) =>
  doc.window.layout.items[1].widget.panes[1].widget.tabs[1].page.layout.items[1].widget.properties;

function apply(command: EditCommand) {
  const r = applyCommand(SAMPLE, command);
  if (!r.ok) {
    throw new Error(r.error);
  }
  expect(validateDocument(r.document)).toEqual([]);
  return r.document as any;
}

function error(command: EditCommand): string {
  const r = applyCommand(SAMPLE, command);
  expect(r.ok).toBe(false);
  return r.ok ? '' : r.error;
}

describe('Variable の編集', () => {
  it('見本の結びつけ', () => {
    expect(NAME_EDIT_TEXT(SAMPLE)).toEqual({ text: { bind: 'file_name' } });
  });

  it('Variable を足して結びつける(batch)', () => {
    const doc = apply({
      kind: 'batch',
      commands: [
        { kind: 'addVariable', name: 'search_text', type: 'string', value: '検索' },
        {
          kind: 'setProperties',
          target: SEARCH_BUTTON,
          properties: { text: { bind: 'search_text' } },
        },
      ],
    });
    expect(doc.variables[2]).toEqual({ name: 'search_text', type: 'string', value: '検索' });
  });

  it('型の合わない結びつけ・位置と大きさ・無い Variable は拒む', () => {
    const bind = (properties: Record<string, unknown>) =>
      error({ kind: 'setProperties', target: SEARCH_BUTTON, properties: properties as never });
    expect(bind({ enabled: { bind: 'file_name' } })).toContain('file_name');
    expect(bind({ text: { bind: 'no_such' } })).toContain('no_such');
    expect(bind({ font: { bind: 'file_name' } })).not.toBe('');
  });

  it('名前を変えると結びつけも書き換え、型を変えて合わなくなるなら拒む', () => {
    const doc = apply({
      kind: 'rename',
      target: ['variables', 0],
      name: 'path',
    });
    expect(NAME_EDIT_TEXT(doc)).toEqual({ text: { bind: 'path' } });
    expect(error({ kind: 'setVariable', target: ['variables', 0], type: 'int' })).toContain(
      'file_name',
    );
    const changed = apply({ kind: 'setVariable', target: ['variables', 1], value: true });
    expect(changed.variables[1]).toEqual({ name: 'show_hidden', type: 'bool', value: true });
  });

  it('Variable を消すと、結びつけたプロパティはその初期値になる', () => {
    const doc = apply({ kind: 'remove', targets: [['variables', 0]] });
    expect(NAME_EDIT_TEXT(doc)).toEqual({ text: 'メモ.txt' });
    expect(doc.variables).toHaveLength(1);
    const both = apply({
      kind: 'remove',
      targets: [
        ['variables', 0],
        ['variables', 1],
      ],
    });
    expect(both.variables).toBeUndefined();
  });

  it('初期値は型に合わなければならない', () => {
    expect(error({ kind: 'setVariable', target: ['variables', 1], value: 'yes' })).not.toBe('');
  });
});
