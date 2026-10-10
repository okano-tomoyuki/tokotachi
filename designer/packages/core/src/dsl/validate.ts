/**
 * 定義ファイルの意味の検証(docs/designer/dsl-spec.md §7)。構造(形)は schema.ts で検証済みのものを受け取る。
 */
import { findClass, findHandler, findProperty, isSubclassOf } from '../catalog/catalog.ts';
import type { ClassKind, PropertyInfo } from '../catalog/types.ts';
import { describeIdentifierProblem, isValidIdentifier, memberNameProblem } from '../identifier.ts';
import { l10n } from '../l10n.ts';
import type { Diagnostic, DiagnosticCode, JsonPath, Severity } from './diagnostics.ts';
import type {
  Events,
  LayoutItem,
  LayoutNode,
  Properties,
  TtuiDocument,
  WidgetNode,
} from './schema.ts';
import { layouts, menus, namedNodes, temporaryNames, widgets, type NamedNode } from './tree.ts';
import { bindingOf, isBindable, variableTypeOf } from './bindings.ts';
import { checkMembers, checkValue, isEnumValue } from './values.ts';

export function validateDocument(doc: TtuiDocument): Diagnostic[] {
  return new Validator(doc).run();
}

const PASCAL_CASE = /^[A-Z][A-Za-z0-9]*$/;
const SNAKE_CASE = /^[a-z][a-z0-9_]*$/;

/** BoxLayout・GridLayout・PlaceLayout の項目に書けるキー。 */
const ITEM_KEYS: Readonly<Record<string, readonly string[]>> = {
  BoxLayout: ['widget', 'stretch', 'alignment'],
  GridLayout: ['widget', 'row', 'column', 'row_span', 'column_span', 'horizontal', 'vertical'],
  PlaceLayout: ['widget', 'placement'],
};

class Validator {
  private readonly doc: TtuiDocument;
  private readonly diagnostics: Diagnostic[] = [];
  private readonly named: NamedNode[];
  private readonly byName = new Map<string, NamedNode>();
  /** メンバ関数の名前 → ハンドラの型と最初の位置。 */
  private readonly handlers = new Map<string, { type: string; path: JsonPath }>();

  constructor(doc: TtuiDocument) {
    this.doc = doc;
    this.named = namedNodes(doc);
  }

  run(): Diagnostic[] {
    this.checkNames();
    this.checkWindow();
    this.doc.actions?.forEach((a, i) => {
      const path = ['actions', i];
      this.checkProperties('Action', a.properties, path);
      this.checkEvents('Action', a.events, path);
    });
    this.doc.objects?.forEach((o, i) => {
      const path = ['objects', i];
      if (this.checkClass(o.class, 'object', [...path, 'class'])) {
        this.checkProperties(o.class, o.properties, path);
        this.checkEvents(o.class, o.events, path);
      }
    });
    this.doc.variables?.forEach((v, i) => {
      const path = ['variables', i];
      if (v.value !== undefined) {
        const problem = checkValue(v.type, v.value);
        if (problem) {
          this.error('invalid-property-value', problem, [...path, 'value']);
        }
      }
      this.checkEvents('Variable', v.events, path);
    });
    for (const e of widgets(this.doc)) {
      this.checkWidget(e.node, e.path, e.placement.kind === 'free');
    }
    for (const l of layouts(this.doc)) {
      this.checkLayout(l.node, l.path);
    }
    this.checkMenus();
    this.checkHandlerNames();
    return this.diagnostics;
  }

  private report(severity: Severity, code: DiagnosticCode, message: string, path: JsonPath): void {
    this.diagnostics.push({ severity, code, message, path });
  }

  private error(code: DiagnosticCode, message: string, path: JsonPath): void {
    this.report('error', code, message, path);
  }

  // ---- 名前 ----

  private checkNames(): void {
    const w = this.doc.window;
    const classProblem = isValidIdentifier(w.name);
    if (classProblem) {
      this.error('invalid-identifier', describeIdentifierProblem(classProblem), ['window', 'name']);
    } else if (!PASCAL_CASE.test(w.name)) {
      this.report(
        'warning',
        'naming-style',
        l10n.t('Class names are recommended to be PascalCase (e.g. {0})', 'MainWindow'),
        ['window', 'name'],
      );
    }
    for (const n of this.named) {
      const problem = memberNameProblem(n.name);
      if (problem) {
        this.error('invalid-identifier', problem, n.path);
        continue;
      }
      const first = this.byName.get(n.name);
      if (first) {
        this.error('duplicate-name', l10n.t('The name "{0}" is already used', n.name), n.path);
        continue;
      }
      if (n.name === w.name) {
        this.error(
          'duplicate-name',
          l10n.t('The name "{0}" is the name of the generated class', n.name),
          n.path,
        );
        continue;
      }
      this.byName.set(n.name, n);
      if (!SNAKE_CASE.test(n.name)) {
        this.report(
          'warning',
          'naming-style',
          l10n.t('Member names are recommended to be snake_case (e.g. {0})', 'ok_button'),
          n.path,
        );
      }
    }
    for (const t of temporaryNames(this.doc)) {
      const member = this.byName.get(t.name);
      if (member) {
        this.error(
          'reserved-name',
          l10n.t(
            'The generated code uses "{0}" as a temporary variable, so it cannot be used as a name',
            t.name,
          ),
          member.path,
        );
      }
    }
  }

  // ---- クラス・プロパティ・ハンドラ ----

  /** クラスがカタログにあり、種類が合うか。 */
  private checkClass(cls: string, kind: ClassKind, path: JsonPath): boolean {
    const info = findClass(cls);
    if (!info) {
      this.error('unknown-class', l10n.t('Unknown class "{0}"', cls), path);
      return false;
    }
    if (info.kind !== kind) {
      this.error(
        'wrong-class-kind',
        l10n.t('"{0}" cannot be used here (expected: {1})', cls, kind),
        path,
      );
      return false;
    }
    return true;
  }

  /**
   * @param geometry 書ける位置と大きさのプロパティ(ウィンドウは width・height、自由な配置の子はすべて)
   */
  private checkProperties(
    cls: string,
    props: Properties | undefined,
    path: JsonPath,
    geometry: readonly string[] = [],
  ): void {
    const info = findClass(cls);
    if (!info || !props) {
      return;
    }
    for (const [name, value] of Object.entries(props)) {
      const p = [...path, 'properties', name];
      const prop = findProperty(info, name);
      if (!prop) {
        this.error(
          'unknown-property',
          l10n.t('{0} has no property "{1}" that can be set at design time', cls, name),
          p,
        );
        continue;
      }
      if (prop.geometry && !geometry.includes(name)) {
        this.error(
          'geometry-not-allowed',
          l10n.t(
            '"{0}" is decided by the layout. It can be set only for the window and for children placed freely',
            name,
          ),
          p,
        );
        continue;
      }
      const variable = bindingOf(value);
      if (variable !== undefined) {
        this.checkBinding(variable, prop, p);
        continue;
      }
      const problem = checkValue(prop.type, value);
      if (problem) {
        this.error('invalid-property-value', problem, p);
        continue;
      }
      if (typeof prop.type === 'object' && 'ref' in prop.type && typeof value === 'string') {
        this.checkReference(value, prop.type.ref, p);
      }
    }
  }

  /** Variable との結びつけ(dsl-spec.md §5.2)。 */
  private checkBinding(variable: string, prop: PropertyInfo, path: JsonPath): void {
    if (!isBindable(prop)) {
      this.error(
        'binding-not-allowed',
        l10n.t(
          'Only properties of type bool, int, float, double or string (other than the position and size) can be bound to a variable',
        ),
        path,
      );
      return;
    }
    const target = this.doc.variables?.find((v) => v.name === variable);
    if (!target) {
      this.error('unknown-reference', l10n.t('The variable "{0}" is not found', variable), path);
      return;
    }
    const expected = variableTypeOf(prop.type);
    if (target.type !== expected) {
      this.error(
        'reference-type-mismatch',
        l10n.t(
          'The variable "{0}" is of type {1}, but the property is of type {2}',
          variable,
          target.type,
          expected ?? '',
        ),
        path,
      );
    }
  }

  private checkReference(name: string, cls: string, path: JsonPath): void {
    const target = this.byName.get(name);
    if (!target) {
      this.error('unknown-reference', l10n.t('"{0}" is not found', name), path);
    } else if (!isSubclassOf(target.class, cls)) {
      this.error(
        'reference-type-mismatch',
        l10n.t('"{0}" is a {1}, but a {2} is expected', name, target.class, cls),
        path,
      );
    }
  }

  private checkEvents(cls: string, events: Events | undefined, path: JsonPath): void {
    const info = findClass(cls);
    if (!info || !events) {
      return;
    }
    for (const [name, handler] of Object.entries(events)) {
      const p = [...path, 'events', name];
      const h = findHandler(info, name);
      if (!h) {
        this.error('unknown-event', l10n.t('{0} has no handler "{1}"', cls, name), p);
        continue;
      }
      const first = this.handlers.get(handler);
      if (!first) {
        this.handlers.set(handler, { type: h.type, path: p });
      } else if (first.type !== h.type) {
        this.error(
          'handler-signature-conflict',
          l10n.t(
            '"{0}" is also used for a handler of type {1}, but this handler is of type {2}',
            handler,
            first.type,
            h.type,
          ),
          p,
        );
      }
    }
  }

  /** メンバ関数の名前が識別子として正しく、メンバ(部品等)の名前と重ならないか。 */
  private checkHandlerNames(): void {
    for (const [name, { path }] of this.handlers) {
      const problem = memberNameProblem(name);
      if (problem) {
        this.error('invalid-identifier', problem, path);
      } else if (this.byName.has(name) || name === this.doc.window.name) {
        this.error(
          'duplicate-name',
          l10n.t('The handler name "{0}" is also the name of a member', name),
          path,
        );
      }
    }
  }

  private checkActionReference(name: string, path: JsonPath): void {
    this.checkReference(name, 'Action', path);
  }

  // ---- ウィンドウ・部品・レイアウト ----

  private checkWindow(): void {
    const w = this.doc.window;
    const path = ['window'];
    this.checkProperties(w.class, w.properties, path, ['width', 'height']);
    this.checkEvents(w.class, w.events, path);
    this.checkChildrenKeys(w, path, w.class);
    if (w.buttons) {
      if (w.class !== 'Dialog') {
        this.error('buttons-not-allowed', l10n.t('"buttons" can be written only for a Dialog'), [
          ...path,
          'buttons',
        ]);
      } else {
        w.buttons.forEach((b, i) => {
          if (!isEnumValue('DialogButton', b.button) || b.button === 'NONE') {
            this.error(
              'invalid-dialog-button',
              l10n.t('Expected one of the values of {0} except NONE', 'DialogButton'),
              [...path, 'buttons', i, 'button'],
            );
          }
        });
      }
    }
    if (w.menu_bar) {
      this.checkProperties('MenuBar', w.menu_bar.properties, [...path, 'menu_bar']);
    }
    if (w.tool_bar) {
      const tp = [...path, 'tool_bar'];
      this.checkProperties('ToolBar', w.tool_bar.properties, tp);
      w.tool_bar.items?.forEach((item, i) => {
        const ip = [...tp, 'items', i];
        const count = [item.action, item.separator, item.widget].filter(
          (x) => x !== undefined,
        ).length;
        if (count !== 1) {
          this.error(
            'invalid-tool-item',
            l10n.t('Write exactly one of "action", "separator" and "widget"'),
            ip,
          );
        } else if (item.action !== undefined) {
          this.checkActionReference(item.action, [...ip, 'action']);
        }
      });
    }
    if (w.status_bar) {
      this.checkProperties('StatusBar', w.status_bar.properties, [...path, 'status_bar']);
    }
  }

  /** layout・children・tabs・panes・content を書けるか。 */
  private checkChildrenKeys(
    node: {
      readonly layout?: LayoutNode | undefined;
      readonly children?: readonly WidgetNode[] | undefined;
      readonly tabs?: readonly unknown[] | undefined;
      readonly panes?: readonly unknown[] | undefined;
      readonly content?: WidgetNode | undefined;
    },
    path: JsonPath,
    cls: string,
  ): void {
    const info = findClass(cls);
    if (!info) {
      return;
    }
    if (!info.accepts_children) {
      for (const key of ['layout', 'children'] as const) {
        if (node[key] !== undefined) {
          this.error('children-not-allowed', l10n.t('{0} cannot have "{1}"', cls, key), [
            ...path,
            key,
          ]);
        }
      }
    } else if (node.layout !== undefined && node.children !== undefined) {
      this.error(
        'layout-and-children',
        l10n.t('"layout" and "children" cannot be written together'),
        [...path, 'children'],
      );
    }
    for (const key of ['tabs', 'panes', 'content'] as const) {
      if (node[key] !== undefined && !(info.child_keys ?? []).includes(key)) {
        this.error('child-key-not-allowed', l10n.t('{0} cannot have "{1}"', cls, key), [
          ...path,
          key,
        ]);
      }
    }
  }

  private checkWidget(node: WidgetNode, path: JsonPath, free: boolean): void {
    if (!this.checkClass(node.class, 'widget', [...path, 'class'])) {
      return;
    }
    this.checkProperties(
      node.class,
      node.properties,
      path,
      free ? ['x', 'y', 'width', 'height'] : [],
    );
    this.checkEvents(node.class, node.events, path);
    node.actions?.forEach((a, i) => {
      this.checkActionReference(a, [...path, 'actions', i]);
    });
    this.checkChildrenKeys(node, path, node.class);
    node.tabs?.forEach((t, i) => {
      this.checkProperties('Tab', t.properties, [...path, 'tabs', i]);
    });
    node.panes?.forEach((p, i) => {
      this.checkProperties('SplitterPane', p.properties, [...path, 'panes', i]);
    });
  }

  private checkLayout(layout: LayoutNode, path: JsonPath): void {
    if (!this.checkClass(layout.class, 'layout', [...path, 'class'])) {
      return;
    }
    this.checkProperties(layout.class, layout.properties, path);
    if (layout.class !== 'GridLayout') {
      for (const key of ['rows', 'columns'] as const) {
        if (layout[key] !== undefined) {
          this.error(
            'tracks-not-allowed',
            l10n.t('"{0}" can be written only for a GridLayout', key),
            [...path, key],
          );
        }
      }
    }
    const cells: { row: number; column: number; rows: number; columns: number; index: number }[] =
      [];
    layout.items?.forEach((item, i) => {
      const ip = [...path, 'items', i];
      const problem = this.itemProblem(layout.class, item);
      if (problem) {
        this.error('invalid-layout-item', problem, ip);
        return;
      }
      if (layout.class === 'GridLayout' && item.row !== undefined && item.column !== undefined) {
        const cell = {
          row: item.row,
          column: item.column,
          rows: item.row_span ?? 1,
          columns: item.column_span ?? 1,
          index: i,
        };
        const other = cells.find(
          (c) =>
            c.row < cell.row + cell.rows &&
            cell.row < c.row + c.rows &&
            c.column < cell.column + cell.columns &&
            cell.column < c.column + c.columns,
        );
        if (other) {
          this.error(
            'grid-cell-overlap',
            l10n.t('Overlaps the cell of item {0}', String(other.index)),
            ip,
          );
        }
        cells.push(cell);
      }
    });
  }

  /** レイアウトの項目の組み合わせ。正しければ undefined。 */
  private itemProblem(cls: string, item: LayoutItem): string | undefined {
    const keys = Object.keys(item).filter((k) => item[k as keyof LayoutItem] !== undefined);
    const allowed = ITEM_KEYS[cls] ?? [];
    if (cls === 'BoxLayout' && item.widget === undefined) {
      // 空き: stretch か spacing のどちらか 1 つだけ
      if (keys.length === 1 && (keys[0] === 'stretch' || keys[0] === 'spacing')) {
        return undefined;
      }
      return l10n.t(
        'A BoxLayout item is a widget ("widget"), a stretch ("stretch" only) or a spacing ("spacing" only)',
      );
    }
    if (item.widget === undefined) {
      return l10n.t('"widget" is required');
    }
    const extra = keys.filter((k) => !allowed.includes(k));
    if (extra.length > 0) {
      return l10n.t('{0} items cannot have: {1}', cls, extra.join(', '));
    }
    for (const key of ['alignment', 'horizontal', 'vertical'] as const) {
      const v = item[key];
      if (v !== undefined && !isEnumValue('Align', v)) {
        return l10n.t('"{0}" expects one of the values of {1}', key, 'Align');
      }
    }
    if (cls === 'GridLayout' && (item.row === undefined || item.column === undefined)) {
      return l10n.t('A GridLayout item requires "row" and "column"');
    }
    if (item.placement !== undefined) {
      return checkMembers('Placement', item.placement) ?? undefined;
    }
    return undefined;
  }

  private checkMenus(): void {
    for (const m of menus(this.doc)) {
      m.node.items?.forEach((item, i) => {
        const ip = [...m.path, 'items', i];
        const count = [item.action, item.separator, item.menu].filter(
          (x) => x !== undefined,
        ).length;
        if (count !== 1) {
          this.error(
            'invalid-menu-item',
            l10n.t('Write exactly one of "action", "separator" and "menu"'),
            ip,
          );
        } else if (item.action !== undefined) {
          this.checkActionReference(item.action, [...ip, 'action']);
        }
      });
    }
  }
}
