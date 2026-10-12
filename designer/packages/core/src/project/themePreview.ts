/**
 * プロジェクトの設定画面で、テーマを確かめるために描く見本のウィンドウ(docs/designer/project-spec.md §6)。
 * 色の役割と寸法が現れる主な部品を並べる。プレビューするテーマは、ウィンドウの theme_override に当てて描く。
 */
import type { TtuiDocument } from '../dsl/schema.ts';

function row(name: string, widgets: readonly unknown[]): unknown {
  return {
    widget: {
      name,
      class: 'Widget',
      layout: {
        class: 'BoxLayout',
        properties: { orientation: 'HORIZONTAL', spacing: 8 },
        items: widgets.map((widget) => ({ widget })),
      },
    },
  };
}

const PREVIEW = {
  format_version: 1,
  window: {
    name: 'ThemePreview',
    class: 'Window',
    properties: { width: 360, height: 300, title: 'Preview' },
    layout: {
      class: 'BoxLayout',
      properties: { margins: 12, spacing: 8 },
      items: [
        { widget: { name: 'label_preview', class: 'Label', properties: { text: 'Label' } } },
        {
          widget: {
            name: 'line_edit_preview',
            class: 'LineEdit',
            properties: { text: 'LineEdit' },
          },
        },
        {
          widget: {
            name: 'combo_box_preview',
            class: 'ComboBox',
            properties: { items: ['ComboBox'], current_index: 0 },
          },
        },
        row('row_check', [
          {
            name: 'check_box_preview',
            class: 'CheckBox',
            properties: { text: 'CheckBox', checked: true },
          },
          {
            name: 'radio_button_preview',
            class: 'RadioButton',
            properties: { text: 'RadioButton', value: 1, group_value: 1 },
          },
        ]),
        row('row_value', [
          { name: 'slider_preview', class: 'Slider', properties: { value: 40 } },
          { name: 'spin_box_preview', class: 'SpinBox', properties: { value: 40 } },
        ]),
        {
          widget: { name: 'progress_bar_preview', class: 'ProgressBar', properties: { value: 60 } },
        },
        { stretch: 1 },
        row('row_buttons', [
          { name: 'button_primary', class: 'Button', properties: { text: 'OK' } },
          {
            name: 'button_secondary',
            class: 'Button',
            properties: { text: 'Cancel', style: 'SECONDARY' },
          },
          {
            name: 'button_disabled',
            class: 'Button',
            properties: { text: 'Disabled', enabled: false },
          },
        ]),
      ],
    },
  },
};

/**
 * テーマの見本のウィンドウ。
 * @param themeOverride ウィンドウに当てるテーマの上書き(部品の theme_override と同じ書き方。無ければ当てない)
 */
export function themePreviewDocument(
  themeOverride?: Readonly<Record<string, unknown>>,
): TtuiDocument {
  const doc = JSON.parse(JSON.stringify(PREVIEW)) as {
    window: { properties: Record<string, unknown> };
  };
  if (themeOverride !== undefined) {
    doc.window.properties['theme_override'] = themeOverride;
  }
  return doc as unknown as TtuiDocument;
}
