import { describe, expect, it } from 'vitest';
import { readCustomClasses } from './customHeader.ts';

const GAUGE = `#pragma once
#include <tokotachi/tokotachi.hpp>

namespace app
{
namespace widgets
{

class Helper;

/// 値を円弧で表すゲージ。
class Gauge : public tokotachi::PaintBox
{
public:
    explicit Gauge(tokotachi::Widget* parent = nullptr);

    /// 今の値(0〜100)。
    tokotachi::Property<double> level{40};
    /// 単位の文字。
    tokotachi::Property<std::string> unit = "%";
    tokotachi::Property<bool> animated;
    tokotachi::Property<tokotachi::Align> alignment{tokotachi::Align::CENTER};
    tokotachi::Property<std::vector<std::string>> marks;
    tokotachi::Property<tokotachi::NotifyHandler> on_level_change;  ///< 値が変わったとき
    tokotachi::Property<std::map<int, int>> table;

protected:
    void paint(tokotachi::Painter& painter) override;
    tokotachi::Property<int> hidden_level;

private:
    double cache_ = 0;
};

/* ゲージの派生 */
struct RedGauge : Gauge
{
    tokotachi::Property<tokotachi::Widget*> buddy;
};

class NotWidget : public tokotachi::Object
{
};

}  // namespace widgets
}  // namespace app
`;

describe('readCustomClasses', () => {
  it('部品の派生クラスの public の Property を、プロパティ・ハンドラにする', () => {
    const { classes, problems } = readCustomClasses([{ path: 'src/gauge.hpp', text: GAUGE }]);
    const gauge = classes['Gauge'];
    expect(gauge).toMatchObject({
      bases: ['PaintBox', 'Widget', 'Object'],
      kind: 'widget',
      category: 'custom',
      doc: '値を円弧で表すゲージ。',
      custom: { header: 'src/gauge.hpp', qualified: 'app::widgets::Gauge', base: 'PaintBox' },
    });
    expect(gauge?.properties['level']).toEqual({
      type: 'double',
      declared_in: 'Gauge',
      doc: '今の値(0〜100)。',
      default: 40,
    });
    expect(gauge?.properties['unit']).toMatchObject({ type: 'string', default: '%' });
    expect(gauge?.properties['animated']).toEqual({ type: 'bool', declared_in: 'Gauge' });
    expect(gauge?.properties['alignment']).toMatchObject({
      type: { enum: 'Align' },
      default: 'CENTER',
    });
    expect(gauge?.properties['marks']?.type).toEqual({ array: 'string' });
    // 基底のプロパティ・ハンドラを受け継ぐ
    expect(gauge?.properties['enabled']?.declared_in).toBe('Widget');
    expect(gauge?.handlers['on_level_change']).toEqual({
      type: 'NotifyHandler',
      declared_in: 'Gauge',
      doc: '値が変わったとき',
    });
    expect(gauge?.properties['hidden_level']).toBeUndefined();
    // 自作部品の派生(struct は public)
    expect(classes['RedGauge']).toMatchObject({
      bases: ['Gauge', 'PaintBox', 'Widget', 'Object'],
      custom: { qualified: 'app::widgets::RedGauge', base: 'PaintBox' },
    });
    expect(classes['RedGauge']?.properties['buddy']?.type).toEqual({ ref: 'Widget' });
    expect(classes['RedGauge']?.properties['level']?.declared_in).toBe('Gauge');
    expect(classes['NotWidget']).toBeUndefined();
    // ライブラリの部品の派生でないクラスは黙って除く
    expect(problems.map((p) => [p.line, p.message])).toEqual([
      [
        25,
        'The property "table" of Gauge is not shown in the designer (unsupported type: std::map<int, int>)',
      ],
    ]);
  });

  it('ライブラリのクラスと同じ名前・ヘッダの間での重なりは問題にする', () => {
    const { classes, problems } = readCustomClasses([
      { path: 'a.hpp', text: 'class Button : public tokotachi::Widget {};' },
      { path: 'b.hpp', text: 'class Dial : public tokotachi::Widget {};' },
      { path: 'c.hpp', text: 'class Dial : public tokotachi::Widget {};' },
    ]);
    expect(Object.keys(classes)).toEqual(['Dial']);
    expect(problems.map((p) => p.header)).toEqual(['a.hpp', 'c.hpp']);
  });
});
