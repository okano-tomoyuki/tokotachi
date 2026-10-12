// 見本(designer/samples)をプロジェクト(samples.ttproj.json)として扱うための共通の処理。
import { readFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseProject, readCustomClasses, setCustomClasses } from '@tokotachi-designer/core';

export const SAMPLES_DIR = resolve(dirname(fileURLToPath(import.meta.url)), '..', 'samples');

/** 見本のプロジェクトに登録した自作部品(docs/adr/0044)を読んで、カタログに足す。 */
export function useSampleCustomClasses(): void {
  const project = parseProject(readFileSync(join(SAMPLES_DIR, 'samples.ttproj.json'), 'utf8'));
  if (!project.project) {
    throw new Error('samples.ttproj.json に誤りがある');
  }
  const { classes, problems } = readCustomClasses(
    (project.project.custom_widgets ?? []).map((path) => ({
      path,
      text: readFileSync(join(SAMPLES_DIR, path), 'utf8'),
    })),
  );
  if (problems.length > 0) {
    throw new Error(problems.map((p) => `${p.header}:${String(p.line)}: ${p.message}`).join('\n'));
  }
  setCustomClasses(classes);
}

/** 見本の定義ファイル(samples/ からの相対パス)のフォルダから、プロジェクトのフォルダへの相対パス。 */
export function sampleProjectDir(file: string): string {
  const depth = file.split('/').length - 1;
  return Array<string>(depth).fill('..').join('/');
}
