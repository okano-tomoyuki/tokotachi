import { describe, expect, it } from 'vitest';
import { hasErrors } from '../dsl/diagnostics.ts';
import { parseDocument } from '../dsl/parse.ts';
import { themePreviewDocument } from './themePreview.ts';

describe('themePreviewDocument', () => {
  it('検証を通る(テーマの上書きを当てても)', () => {
    for (const doc of [
      themePreviewDocument(),
      themePreviewDocument({ colors: { ACCENT: '#DC2626' }, radius: 0 }),
    ]) {
      const { document, diagnostics } = parseDocument(JSON.stringify(doc));
      expect(document).toBeDefined();
      expect(hasErrors(diagnostics)).toBe(false);
    }
  });
});
