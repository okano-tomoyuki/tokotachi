import { readCustomClasses, TtuiDocument, withCustomClasses } from '@tokotachi-designer/core';
import { describe, expect, it } from 'vitest';
import { canvasSetup } from './components.ts';
import { withPlaceholders } from './placeholders.ts';

const { classes } = readCustomClasses([
  {
    path: 'gauge.hpp',
    text: `class Gauge : public tokotachi::PaintBox { public: tokotachi::Property<double> level; };
class Meter : public tokotachi::ProgressBar { public: tokotachi::Property<int> ticks; };`,
  },
]);

const DOC = TtuiDocument.parse({
  format_version: 1,
  window: {
    name: 'MainWindow',
    class: 'Window',
    children: [
      { name: 'gauge_1', class: 'Gauge', properties: { level: 40, x: 8 } },
      { name: 'meter_1', class: 'Meter', properties: { ticks: 5, value: 30 } },
    ],
  },
});

describe('自作部品のキャンバス', () => {
  it('基底のカタログの部品として作り、自作のプロパティは設定しない', () => {
    const ops = withCustomClasses(classes, () => canvasSetup(DOC, undefined, undefined).ops);
    expect(ops.filter((op) => op.op !== 'blank')).toEqual([
      { op: 'new', variable: 'gauge_1', class: 'PaintBox', declare: false, parent: 'this' },
      { op: 'set', target: 'gauge_1', property: 'x', type: 'float', value: 8 },
      { op: 'new', variable: 'meter_1', class: 'ProgressBar', declare: false, parent: 'this' },
      { op: 'set', target: 'meter_1', property: 'value', type: 'double', value: 30 },
    ]);
  });

  it('基底が Widget・PaintBox の自作部品には、仮の大きさを与える', () => {
    const doc = withCustomClasses(classes, () => withPlaceholders(DOC));
    expect(doc.window.children?.[0]?.properties?.['minimum_size']).toEqual({
      width: 120,
      height: 80,
    });
    expect(doc.window.children?.[1]?.properties?.['minimum_size']).toBeUndefined();
  });
});
