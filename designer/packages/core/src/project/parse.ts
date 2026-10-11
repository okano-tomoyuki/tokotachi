/**
 * プロジェクトファイルの読み込み・検証・決まった形での書き出し(docs/designer/project-spec.md §3・§4)。
 */
import { describeIdentifierProblem, isValidIdentifier } from '../identifier.ts';
import { l10n } from '../l10n.ts';
import type { Diagnostic } from '../dsl/diagnostics.ts';
import { checkThemeOverride } from '../dsl/themeOverride.ts';
import { isValidWindowPath, normalizePath, sameWindowPath } from './paths.ts';
import { PROJECT_FORMAT_VERSION, TtprojDocument } from './schema.ts';

export interface ProjectParseResult {
  readonly project?: TtprojDocument;
  readonly diagnostics: readonly Diagnostic[];
}

/** フォルダのパスとして書けるか(相対パス・区切りは /)。 */
function isValidDirectory(path: string): boolean {
  return !path.startsWith('/') && !path.includes('\\') && !/^[A-Za-z]:/.test(path);
}

/** 自作部品のヘッダのパスとして書けるか(相対パス・区切りは /・C++ のヘッダの拡張子)。 */
function isValidHeaderPath(path: string): boolean {
  return isValidDirectory(path) && /[^/]\.(h|hpp|hh|hxx)$/.test(path);
}

export function validateProject(project: TtprojDocument): Diagnostic[] {
  const out: Diagnostic[] = [];
  const error = (message: string, path: (string | number)[]): void => {
    out.push({ severity: 'error', code: 'schema', message, path });
  };
  const windows = project.windows ?? [];
  windows.forEach((w, i) => {
    if (!isValidWindowPath(w)) {
      error(l10n.t('Write a relative path ending with ".ttui.json" (separator: /)'), [
        'windows',
        i,
      ]);
    } else if (windows.slice(0, i).some((x) => sameWindowPath(x, w))) {
      error(l10n.t('"{0}" is listed twice', w), ['windows', i]);
    }
  });
  const headers = project.custom_widgets ?? [];
  headers.forEach((h, i) => {
    if (!isValidHeaderPath(h)) {
      error(l10n.t('Write a relative path of a C++ header (.h, .hpp, .hh or .hxx; separator: /)'), [
        'custom_widgets',
        i,
      ]);
    } else if (headers.slice(0, i).some((x) => normalizePath(x) === normalizePath(h))) {
      error(l10n.t('"{0}" is listed twice', h), ['custom_widgets', i]);
    }
  });
  if (
    project.main_window !== undefined &&
    !windows.some((w) => sameWindowPath(w, project.main_window ?? ''))
  ) {
    error(l10n.t('The main window must be one of the windows'), ['main_window']);
  }
  const codegen = project.codegen;
  if (codegen?.namespace !== undefined && codegen.namespace !== '') {
    if (codegen.namespace.split('::').some((n) => isValidIdentifier(n) !== null)) {
      error(l10n.t('Write a namespace such as "app" or "app::ui"'), ['codegen', 'namespace']);
    }
  }
  if (
    codegen?.include_guard_prefix !== undefined &&
    !/^[A-Za-z_][A-Za-z0-9_]*$|^$/.test(codegen.include_guard_prefix)
  ) {
    error(l10n.t('Only letters, digits and _ can be used, and it cannot start with a digit'), [
      'codegen',
      'include_guard_prefix',
    ]);
  }
  for (const key of ['header_dir', 'source_dir'] as const) {
    const dir = codegen?.[key];
    if (dir !== undefined && !isValidDirectory(dir)) {
      error(l10n.t('Write a relative path (separator: /)'), ['codegen', key]);
    }
  }
  const override = project.theme?.override;
  if (override !== undefined) {
    const problem = checkThemeOverride(override, false);
    if (problem !== null) {
      error(problem, ['theme', 'override']);
    }
  }
  for (const [name, preset] of Object.entries(project.theme?.presets ?? {})) {
    // 名前は生成する関数の名前(<名前>_theme)にもなる
    const invalid = isValidIdentifier(name);
    if (invalid !== null) {
      error(describeIdentifierProblem(invalid), ['theme', 'presets', name]);
      continue;
    }
    const problem = checkThemeOverride(preset, false);
    if (problem !== null) {
      error(problem, ['theme', 'presets', name]);
    }
  }
  // ヘッダ・ソースのフォルダを 1 つにまとめるとき、同じ名前のウィンドウがあると出力先が重なる
  if (codegen?.header_dir !== undefined || codegen?.source_dir !== undefined) {
    const names = windows.map((w) => normalizePath(w).split('/').pop() ?? w);
    names.forEach((n, i) => {
      if (names.indexOf(n) !== i) {
        error(
          l10n.t('Windows with the same file name cannot share header_dir / source_dir: {0}', n),
          ['windows', i],
        );
      }
    });
  }
  return out;
}

export function parseProject(text: string): ProjectParseResult {
  let json: unknown;
  try {
    json = JSON.parse(text);
  } catch (e) {
    return {
      diagnostics: [
        {
          severity: 'error',
          code: 'json-syntax',
          message: e instanceof Error ? e.message : String(e),
          path: [],
        },
      ],
    };
  }
  const version =
    typeof json === 'object' && json !== null
      ? (json as Record<string, unknown>)['format_version']
      : undefined;
  if (typeof version === 'number' && version !== PROJECT_FORMAT_VERSION) {
    return {
      diagnostics: [
        {
          severity: 'error',
          code: 'unsupported-version',
          message: l10n.t(
            'format_version {0} is not supported (supported: {1})',
            String(version),
            PROJECT_FORMAT_VERSION,
          ),
          path: ['format_version'],
        },
      ],
    };
  }
  const result = TtprojDocument.safeParse(json);
  if (!result.success) {
    return {
      diagnostics: result.error.issues.map((issue) => ({
        severity: 'error',
        code: 'schema',
        message: issue.message,
        path: issue.path.map((key) => (typeof key === 'symbol' ? String(key) : key)),
      })),
    };
  }
  return { project: result.data, diagnostics: validateProject(result.data) };
}

/** JSON の値を、深さ depth(2 文字ずつ)の位置に書く形にする。 */
function indented(value: unknown, depth: number): string {
  return JSON.stringify(value, null, 2)
    .split('\n')
    .join(`\n${' '.repeat(depth * 2)}`);
}

/**
 * 決まった形で書き出す(キーの順は $schema・format_version・codegen・theme・main_window・windows・custom_widgets。
 * 一覧は 1 行に 1 つ)。
 */
export function serializeProject(project: TtprojDocument): string {
  const lines: string[] = ['{'];
  const entries: string[] = [];
  if (project.$schema !== undefined) {
    entries.push(`  "$schema": ${JSON.stringify(project.$schema)}`);
  }
  entries.push(`  "format_version": ${String(project.format_version)}`);
  if (project.codegen !== undefined) {
    const order = [
      'comment_locale',
      'generate_on_save',
      'namespace',
      'include_guard_prefix',
      'header_dir',
      'source_dir',
    ] as const;
    const codegen = order.flatMap((k) =>
      project.codegen?.[k] === undefined
        ? []
        : [`    ${JSON.stringify(k)}: ${JSON.stringify(project.codegen[k])}`],
    );
    entries.push(
      codegen.length === 0 ? '  "codegen": {}' : `  "codegen": {\n${codegen.join(',\n')}\n  }`,
    );
  }
  if (project.theme !== undefined) {
    const theme = {
      ...(project.theme.color_scheme !== undefined
        ? { color_scheme: project.theme.color_scheme }
        : {}),
      ...(project.theme.override !== undefined ? { override: project.theme.override } : {}),
      ...(project.theme.presets !== undefined ? { presets: project.theme.presets } : {}),
    };
    entries.push(`  "theme": ${indented(theme, 1)}`);
  }
  if (project.main_window !== undefined) {
    entries.push(`  "main_window": ${JSON.stringify(project.main_window)}`);
  }
  if (project.windows !== undefined) {
    entries.push(
      project.windows.length === 0
        ? '  "windows": []'
        : `  "windows": [\n${project.windows.map((w) => `    ${JSON.stringify(w)}`).join(',\n')}\n  ]`,
    );
  }
  if (project.custom_widgets !== undefined) {
    entries.push(
      project.custom_widgets.length === 0
        ? '  "custom_widgets": []'
        : `  "custom_widgets": [\n${project.custom_widgets.map((w) => `    ${JSON.stringify(w)}`).join(',\n')}\n  ]`,
    );
  }
  lines.push(entries.join(',\n'), '}');
  return `${lines.join('\n')}\n`;
}
