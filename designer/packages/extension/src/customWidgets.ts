/**
 * 自作部品(docs/adr/0044)。プロジェクトファイルの custom_widgets に登録したヘッダを読み、カタログに足すクラスの情報にする。
 * 自作部品はプロジェクトに属するウィンドウでだけ使える。
 */
import {
  readCustomClasses,
  type ClassInfo,
  type CustomHeaderProblem,
  type TtprojDocument,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { projectOf, readText } from './projects.ts';

export interface ProjectCustomClasses {
  readonly classes: Record<string, ClassInfo>;
  /** 読めないヘッダ(custom_widgets の位置)と、ヘッダの中の問題。 */
  readonly missing: readonly number[];
  readonly problems: readonly CustomHeaderProblem[];
}

const NONE: ProjectCustomClasses = { classes: {}, missing: [], problems: [] };

/** プロジェクトに登録した自作部品を読む。 */
export async function readProjectCustomClasses(
  projectUri: vscode.Uri,
  project: TtprojDocument,
): Promise<ProjectCustomClasses> {
  const headers = project.custom_widgets ?? [];
  if (headers.length === 0) {
    return NONE;
  }
  const texts: { path: string; text: string }[] = [];
  const missing: number[] = [];
  for (const [i, path] of headers.entries()) {
    const text = await readText(vscode.Uri.joinPath(projectUri, '..', path));
    if (text === undefined) {
      missing.push(i);
    } else {
      texts.push({ path, text });
    }
  }
  return { ...readCustomClasses(texts), missing };
}

/** 定義ファイルが属するプロジェクトの自作部品(属さなければ空)。 */
export async function customClassesOf(uri: vscode.Uri): Promise<Record<string, ClassInfo>> {
  const owner = await projectOf(uri);
  return owner
    ? (await readProjectCustomClasses(owner.project.uri, owner.project.doc)).classes
    : {};
}

/** 自作部品のヘッダか(変わったら、検証・描画をやり直す)。 */
export function isHeaderFile(uri: vscode.Uri): boolean {
  return /\.(h|hpp|hh|hxx)$/.test(uri.path);
}
