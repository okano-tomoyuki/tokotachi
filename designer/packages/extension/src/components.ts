/**
 * 部品ファイル(docs/adr/0043)を読む。ファイルが有るか・根が Widget か・参照が循環していないかを調べ、
 * キャンバス・コード生成に読んだ中身を渡す(core・codegen はファイルを読まない)。
 */
import type { ComponentResolver } from '@tokotachi-designer/codegen';
import {
  componentReferences,
  hasErrors,
  imageReferences,
  isComponentDocument,
  normalizePath,
  parseDocument,
  relativePath,
  DOCUMENT_EXTENSION,
  withCustomClasses,
  type ClassInfo,
  type Diagnostic,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import * as vscode from 'vscode';
import { readText } from './projects.ts';

/** 読んだ部品ファイル(入れ子の部品ファイルも)。 */
export interface LoadedComponent {
  readonly uri: vscode.Uri;
  readonly document: TtuiDocument;
  /** 部品ファイルが置いている部品ファイル(書いたパス → 読んだもの)。 */
  readonly children: ReadonlyMap<string, LoadedComponent>;
}

export interface LoadedComponents {
  /** 定義ファイルが置いている部品ファイル(書いたパス → 読んだもの。読めないものは入れない)。 */
  readonly components: ReadonlyMap<string, LoadedComponent>;
  /** 読めない・部品ファイルでない・循環している参照(定義ファイルの中の位置)。 */
  readonly diagnostics: readonly Diagnostic[];
}

/** 入れ子の深さの上限。 */
const MAX_DEPTH = 16;

type Problem = { kind: 'missing' } | { kind: 'invalid' } | { kind: 'cycle'; chain: string };

/**
 * 定義ファイルが置いている部品ファイルを読む。
 * @param uri 定義ファイル
 * @param classes 自作部品(docs/adr/0044。定義ファイルが属するプロジェクトのもの)
 */
export async function loadComponents(
  uri: vscode.Uri,
  doc: TtuiDocument,
  classes: Readonly<Record<string, ClassInfo>> = {},
): Promise<LoadedComponents> {
  const diagnostics: Diagnostic[] = [];
  const components = await loadChildren(uri, doc, classes, [uri.toString()], (ref, problem) => {
    diagnostics.push({
      severity: 'error',
      code: problem.kind === 'missing' ? 'missing-component' : 'invalid-component',
      message:
        problem.kind === 'missing'
          ? vscode.l10n.t('The component file "{0}" is not found', ref.path)
          : problem.kind === 'invalid'
            ? vscode.l10n.t(
                '"{0}" is not a component file without errors (its root must be a Widget)',
                ref.path,
              )
            : vscode.l10n.t('The component files refer to each other: {0}', problem.chain),
      path: ref.at,
    });
  });
  return { components, diagnostics };
}

/**
 * @param stack 読んでいる途中の定義ファイル(循環を見つける)
 * @param report 直下の参照の問題を知らせる(入れ子の問題は、直下の参照の問題にする)
 */
async function loadChildren(
  uri: vscode.Uri,
  doc: TtuiDocument,
  classes: Readonly<Record<string, ClassInfo>>,
  stack: readonly string[],
  report: (ref: { path: string; at: readonly (string | number)[] }, problem: Problem) => void,
): Promise<Map<string, LoadedComponent>> {
  const out = new Map<string, LoadedComponent>();
  for (const ref of componentReferences(doc)) {
    if (out.has(ref.path)) {
      continue;
    }
    const target = vscode.Uri.joinPath(uri, '..', normalizePath(ref.path));
    const name = target.path.split('/').pop() ?? ref.path;
    if (stack.includes(target.toString()) || stack.length > MAX_DEPTH) {
      report(ref, { kind: 'cycle', chain: name });
      continue;
    }
    const text = await readText(target);
    if (text === undefined) {
      report(ref, { kind: 'missing' });
      continue;
    }
    const parsed = withCustomClasses(classes, () => parseDocument(text));
    if (
      !parsed.document ||
      hasErrors(parsed.diagnostics) ||
      !isComponentDocument(parsed.document)
    ) {
      report(ref, { kind: 'invalid' });
      continue;
    }
    let problem: Problem | undefined;
    const children = await loadChildren(
      target,
      parsed.document,
      classes,
      [...stack, target.toString()],
      (_, p) => {
        problem ??= p.kind === 'cycle' ? { kind: 'cycle', chain: `${name} → ${p.chain}` } : p;
      },
    );
    if (problem) {
      // 入れ子の部品ファイルの問題は、その部品ファイルを開いたときに出る(循環だけは、ここでも出す)
      if (problem.kind === 'cycle') {
        report(ref, problem);
        continue;
      }
    }
    out.set(ref.path, { uri: target, document: parsed.document, children });
  }
  return out;
}

/** キャンバス・コード生成に渡す、部品ファイルを読む関数。 */
export function componentResolver(
  components: ReadonlyMap<string, LoadedComponent>,
): ComponentResolver {
  return (path) => {
    const c = components.get(path);
    return c ? { document: c.document, resolve: componentResolver(c.children) } : undefined;
  };
}

/** 相対パスをつなぐ(dir が空なら path のまま。どちらも相対パス)。 */
function join(dir: string, path: string): string {
  return normalizePath(dir === '' ? path : `${dir}/${path}`);
}

/**
 * 置いた部品ファイルが参照する画像を読む(キャンバスに渡す。パスは定義ファイルのフォルダからの相対パスを正規化したもの。
 * 展開した操作の並びの画像のパスと同じ)。
 */
export async function readComponentImages(
  components: ReadonlyMap<string, LoadedComponent>,
  images: Map<string, Uint8Array>,
  prefix = '',
): Promise<void> {
  for (const [path, c] of components) {
    const dir = join(prefix, `${path}/..`);
    for (const ref of imageReferences(c.document)) {
      const key = join(dir, ref.path);
      if (images.has(key)) {
        continue;
      }
      try {
        images.set(
          key,
          await vscode.workspace.fs.readFile(
            vscode.Uri.joinPath(c.uri, '..', normalizePath(ref.path)),
          ),
        );
      } catch {
        // 読めない画像は描かない(部品ファイルを開くと問題パネルに出る)
      }
    }
    await readComponentImages(c.children, images, dir);
  }
}

/**
 * パレットに出す部品ファイル(ワークスペースの、根が Widget の定義ファイル)。uri 自身と、uri を置いている(循環する)ものは除く。
 * パスは uri のフォルダからの相対パス。
 */
export async function listComponents(
  uri: vscode.Uri,
  classes: Readonly<Record<string, ClassInfo>> = {},
): Promise<{ path: string; className: string }[]> {
  const files = await vscode.workspace.findFiles(
    `**/*${DOCUMENT_EXTENSION}`,
    '**/{node_modules,.git,.cache}/**',
  );
  const out: { path: string; className: string }[] = [];
  for (const file of files) {
    if (file.toString() === uri.toString() || file.scheme !== uri.scheme) {
      continue;
    }
    const text = await readText(file);
    const parsed = text === undefined ? undefined : parseDocument(text);
    if (!parsed?.document || !isComponentDocument(parsed.document)) {
      continue;
    }
    const { components } = await loadComponents(file, parsed.document, classes);
    if (refers(components, uri)) {
      continue;
    }
    out.push({
      path: relativePath(vscode.Uri.joinPath(uri, '..').path, file.path),
      className: parsed.document.window.name,
    });
  }
  return out.sort((a, b) => a.className.localeCompare(b.className));
}

/** 読んだ部品ファイルが(入れ子も含めて)uri を置いているか。 */
function refers(components: ReadonlyMap<string, LoadedComponent>, uri: vscode.Uri): boolean {
  return [...components.values()].some(
    (c) => c.uri.toString() === uri.toString() || refers(c.children, uri),
  );
}
