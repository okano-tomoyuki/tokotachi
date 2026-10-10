import { describe, expect, it } from 'vitest';
import { findClass, findHandler, findProperty, getCatalog, isSubclassOf } from './catalog.ts';
import type { PropertyType } from './types.ts';

const catalog = getCatalog();

/** 型が参照する列挙型・値の型・クラスが、カタログにあるか。 */
function resolves(type: PropertyType): boolean {
  if (typeof type === 'string') {
    return true;
  }
  if ('enum' in type) {
    return type.enum in catalog.enums;
  }
  if ('value' in type) {
    return type.value in catalog.values;
  }
  if ('ref' in type) {
    return true;
  }
  return true;
}

describe('catalog', () => {
  it('プロパティの型が参照するものは、すべてカタログにある', () => {
    for (const [cls, info] of Object.entries(catalog.classes)) {
      for (const [name, p] of Object.entries(info.properties)) {
        expect(resolves(p.type), `${cls}.${name}`).toBe(true);
      }
      for (const [name, h] of Object.entries(info.handlers)) {
        expect(h.type in catalog.handler_types, `${cls}.${name}`).toBe(true);
      }
    }
  });

  it('値の型のメンバの列挙型も、カタログにある', () => {
    for (const [name, v] of Object.entries(catalog.values)) {
      for (const [member, t] of Object.entries(v.members ?? {})) {
        if (typeof t === 'object') {
          expect(t.enum in catalog.enums, `${name}.${member}`).toBe(true);
        }
      }
    }
  });

  it('継承したプロパティ・ハンドラを含む', () => {
    const button = findClass('Button');
    expect(button?.kind).toBe('widget');
    expect(button?.category).toBe('input');
    expect(button && findProperty(button, 'text')?.declared_in).toBe('ButtonBase');
    expect(button && findProperty(button, 'enabled')?.declared_in).toBe('Widget');
    expect(button && findHandler(button, 'on_click')?.type).toBe('NotifyHandler');
    expect(isSubclassOf('Button', 'Widget')).toBe(true);
    expect(isSubclassOf('Timer', 'Widget')).toBe(false);
  });

  it('デザイン時に設定できないプロパティは含めない', () => {
    const widget = findClass('Widget');
    expect(widget && findProperty(widget, 'parent')).toBeUndefined();
    expect(widget && findProperty(widget, 'bounds')).toBeUndefined();
    // 読み取り専用
    expect(widget && findProperty(widget, 'showing')).toBeUndefined();
    const window = findClass('Window');
    expect(window && findProperty(window, 'menu_bar')).toBeUndefined();
  });

  it('補足の印(位置と大きさ・子の後に設定する・編集の種類)が付く', () => {
    const label = findClass('Label');
    expect(label && findProperty(label, 'width')?.geometry).toBe(true);
    expect(label && findProperty(label, 'text')?.editor).toBe('multiline');
    const splitter = findClass('Splitter');
    expect(splitter && findProperty(splitter, 'sizes')?.after_children).toBe(true);
    expect(splitter?.child_keys).toEqual(['panes']);
  });

  it('ハンドラの型の引数', () => {
    expect(catalog.handler_types['CloseHandler']).toEqual([
      { name: 'sender', type: 'Object*' },
      { name: 'can_close', type: 'bool&' },
    ]);
  });
});
