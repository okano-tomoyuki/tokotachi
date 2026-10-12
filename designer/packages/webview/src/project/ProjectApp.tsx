/**
 * プロジェクトの設定画面(*.ttproj.json。docs/designer/project-spec.md §6)。ウィンドウ・コード生成・テーマの 3 つの欄を並べる。
 * テーマの欄では、プロジェクトのテーマと登録したテーマを選んで編集し、見本のウィンドウをライト・ダークで描いて見せる。
 */
import {
  describeIdentifierProblem,
  getCatalog,
  isValidIdentifier,
  l10n,
  type CanvasFrame,
  type Diagnostic,
  type PropertyValue,
  type TtprojDocument,
} from '@tokotachi-designer/core';
import { StrictMode, useEffect, useRef, useState, type ReactNode } from 'react';
import { createRoot } from 'react-dom/client';
import { ThemeOverrideField } from '../components/ThemeOverrideField.tsx';
import { onMessage, postMessage } from '../vscode.ts';
import {
  editProject,
  handleProjectMessage,
  renamePreset,
  selectTheme,
  useProject,
} from './projectStore.ts';

type Codegen = NonNullable<TtprojDocument['codegen']>;
type Theme = NonNullable<TtprojDocument['theme']>;

/** プロジェクトの設定画面を始める(main.tsx から)。 */
export function startProject(root: HTMLElement): void {
  onMessage(handleProjectMessage);
  createRoot(root).render(
    <StrictMode>
      <ProjectApp />
    </StrictMode>,
  );
}

/** codegen を変える(値が undefined のキーは消し、空になったら codegen ごと消す)。 */
function setCodegen(
  change: Readonly<Partial<Record<keyof Codegen, string | boolean | undefined>>>,
): void {
  editProject((p) => {
    const codegen: Record<string, unknown> = { ...p.codegen };
    for (const [key, value] of Object.entries(change) as [string, string | boolean | undefined][]) {
      if (value === undefined) {
        Reflect.deleteProperty(codegen, key);
      } else {
        codegen[key] = value;
      }
    }
    const next: TtprojDocument = { ...p, codegen };
    if (Object.keys(codegen).length === 0) {
      Reflect.deleteProperty(next, 'codegen');
    }
    return next;
  });
}

/** theme を変える(空になったら theme ごと消す)。 */
function setTheme(change: (theme: Theme) => Theme): void {
  editProject((p) => {
    const theme = change({ ...p.theme });
    const next: TtprojDocument = { ...p, theme };
    if (Object.keys(theme).length === 0) {
      Reflect.deleteProperty(next, 'theme');
    }
    return next;
  });
}

/** path の位置(か、その下)の検証のエラー。 */
function problemsAt(
  diagnostics: readonly Diagnostic[],
  path: readonly (string | number)[],
): string[] {
  return diagnostics.filter((d) => path.every((key, i) => d.path[i] === key)).map((d) => d.message);
}

function Section({ title, children }: { title: string; children: ReactNode }) {
  return (
    <section className="settings-section">
      <h2>{title}</h2>
      {children}
    </section>
  );
}

function Field({
  label,
  hint,
  problems = [],
  children,
}: {
  label: string;
  hint?: string;
  problems?: readonly string[];
  children: ReactNode;
}) {
  return (
    <div className="settings-field">
      <label className="settings-label" title={hint}>
        {label}
      </label>
      <div className="settings-value">
        {children}
        {problems.map((p) => (
          <div key={p} className="settings-problem">
            {p}
          </div>
        ))}
      </div>
    </div>
  );
}

/** 文字の設定。確定(Enter・フォーカスを外す)したら送る。空なら書かない(既定に戻す)。 */
function TextSetting({
  value,
  placeholder,
  onCommit,
}: {
  value: string | undefined;
  placeholder?: string;
  onCommit: (value: string | undefined) => void;
}) {
  return (
    <input
      type="text"
      // 値が外から変わったら作り直す(確定前の入力は残さない)
      key={value ?? ''}
      defaultValue={value ?? ''}
      placeholder={placeholder}
      onBlur={(e) => {
        const text = e.target.value.trim();
        const next = text === '' ? undefined : text;
        if (next !== value) {
          onCommit(next);
        }
      }}
      onKeyDown={(e) => {
        if (e.key === 'Enter') {
          e.currentTarget.blur();
        }
      }}
    />
  );
}

function WindowsSection({ project }: { project: TtprojDocument }) {
  const candidates = useProject((s) => s.candidates);
  const components = useProject((s) => s.components);
  const diagnostics = useProject((s) => s.diagnostics);
  const [adding, setAdding] = useState('');
  const windows = project.windows ?? [];
  return (
    <Section title={l10n.t('Windows')}>
      {windows.length === 0 ? (
        <p className="settings-note">{l10n.t('The project has no windows yet.')}</p>
      ) : (
        <table className="settings-table">
          <thead>
            <tr>
              <th title={l10n.t('The window opened at startup')}>{l10n.t('Main')}</th>
              <th>{l10n.t('Window definition')}</th>
              <th />
            </tr>
          </thead>
          <tbody>
            {windows.map((w, i) => (
              <tr key={w}>
                <td className="settings-center">
                  <input
                    type="radio"
                    name="main_window"
                    disabled={components.includes(w) && project.main_window !== w}
                    title={
                      components.includes(w)
                        ? l10n.t('A component file cannot be the main window')
                        : undefined
                    }
                    checked={project.main_window === w}
                    onChange={() => {
                      editProject((p) => ({ ...p, main_window: w }));
                    }}
                  />
                </td>
                <td>
                  <a
                    href="#"
                    title={l10n.t('Open in the designer')}
                    onClick={(e) => {
                      e.preventDefault();
                      postMessage({ type: 'openWindow', path: w });
                    }}
                  >
                    {w}
                  </a>
                  {problemsAt(diagnostics, ['windows', i]).map((p) => (
                    <div key={p} className="settings-problem">
                      {p}
                    </div>
                  ))}
                </td>
                <td className="settings-center">
                  <button
                    className="secondary"
                    title={l10n.t('Remove from the project (the file is kept)')}
                    onClick={() => {
                      editProject((p) => {
                        const next: TtprojDocument = {
                          ...p,
                          windows: (p.windows ?? []).filter((x) => x !== w),
                        };
                        if (p.main_window === w) {
                          Reflect.deleteProperty(next, 'main_window');
                        }
                        return next;
                      });
                    }}
                  >
                    ×
                  </button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
      {problemsAt(diagnostics, ['main_window']).map((p) => (
        <div key={p} className="settings-problem">
          {p}
        </div>
      ))}
      <div className="settings-row">
        <select
          value={adding}
          onChange={(e) => {
            setAdding(e.target.value);
          }}
          disabled={candidates.length === 0}
        >
          <option value="">
            {candidates.length === 0
              ? l10n.t('(no other windows in the workspace)')
              : l10n.t('Choose a window to add...')}
          </option>
          {candidates.map((c) => (
            <option key={c} value={c}>
              {c}
            </option>
          ))}
        </select>
        <button
          disabled={adding === ''}
          onClick={() => {
            const path = adding;
            setAdding('');
            editProject((p) => ({
              ...p,
              windows: [...(p.windows ?? []), path],
              ...(p.main_window === undefined && !components.includes(path)
                ? { main_window: path }
                : {}),
            }));
          }}
        >
          {l10n.t('Add')}
        </button>
      </div>
    </Section>
  );
}

function CodegenSection({ project }: { project: TtprojDocument }) {
  const diagnostics = useProject((s) => s.diagnostics);
  const c = project.codegen ?? {};
  const text = (key: 'namespace' | 'include_guard_prefix' | 'header_dir' | 'source_dir') => (
    <TextSetting
      value={c[key]}
      onCommit={(value) => {
        setCodegen({ [key]: value });
      }}
    />
  );
  // 対象のライブラリの版(カタログを取り出した版の major.minor。docs/adr/0064)
  const library = getCatalog().tokotachi_version.split('.').slice(0, 2).join('.');
  return (
    <Section title={l10n.t('Code generation')}>
      <Field
        label={l10n.t('Tokotachi version')}
        hint={l10n.t(
          'The library version the generated code is for. The generated CMakeLists.txt requires it (find_package(tokotachi {0}))',
          library,
        )}
      >
        <span>Tokotachi {library}</span>
      </Field>
      <Field
        label={l10n.t('Namespace')}
        hint={l10n.t('The namespace of the generated classes (e.g. app or app::ui)')}
        problems={problemsAt(diagnostics, ['codegen', 'namespace'])}
      >
        {text('namespace')}
      </Field>
      <Field
        label={l10n.t('Include guard prefix')}
        problems={problemsAt(diagnostics, ['codegen', 'include_guard_prefix'])}
      >
        {text('include_guard_prefix')}
      </Field>
      <Field
        label={l10n.t('Header folder')}
        hint={l10n.t('Relative to the project folder. Empty: next to each window definition')}
        problems={problemsAt(diagnostics, ['codegen', 'header_dir'])}
      >
        {text('header_dir')}
      </Field>
      <Field
        label={l10n.t('Source folder')}
        hint={l10n.t('Relative to the project folder. Empty: next to each window definition')}
        problems={problemsAt(diagnostics, ['codegen', 'source_dir'])}
      >
        {text('source_dir')}
      </Field>
      <Field label={l10n.t('Comment language')}>
        <select
          value={c.comment_locale ?? ''}
          onChange={(e) => {
            const v = e.target.value;
            setCodegen({ comment_locale: v === 'en' || v === 'ja' ? v : undefined });
          }}
        >
          <option value="">{l10n.t('Default ({0})', 'English')}</option>
          <option value="en">English</option>
          {/* l10n-ignore */}
          <option value="ja">日本語</option>
        </select>
      </Field>
      <Field label={l10n.t('Generate on save')}>
        <input
          type="checkbox"
          checked={c.generate_on_save !== false}
          onChange={(e) => {
            // 既定(true)は書かない
            setCodegen({ generate_on_save: e.target.checked ? undefined : false });
          }}
        />
      </Field>
    </Section>
  );
}

/** 登録したテーマの名前の問題(無ければ null)。 */
function presetNameProblem(
  name: string,
  presets: Readonly<Record<string, unknown>>,
): string | null {
  const invalid = isValidIdentifier(name);
  if (invalid !== null) {
    return describeIdentifierProblem(invalid);
  }
  return Object.hasOwn(presets, name) ? l10n.t('The theme "{0}" already exists', name) : null;
}

/** 名前の入力(登録・改名)。Enter で確定、Esc でやめる。 */
function NameInput({
  initial,
  presets,
  onDone,
}: {
  initial: string;
  presets: Readonly<Record<string, unknown>>;
  onDone: (name: string | undefined) => void;
}) {
  const [name, setName] = useState(initial);
  const ref = useRef<HTMLInputElement>(null);
  // Enter で確定した後、消えるときのフォーカスの移動で 2 度確定しないようにする
  const done = useRef(false);
  const finish = (result: string | undefined): void => {
    if (!done.current) {
      done.current = true;
      onDone(result);
    }
  };
  useEffect(() => {
    ref.current?.select();
  }, []);
  const problem = name === initial && initial !== '' ? null : presetNameProblem(name, presets);
  return (
    <div className="theme-name-input">
      <input
        ref={ref}
        type="text"
        value={name}
        placeholder={l10n.t('Theme name (e.g. danger)')}
        className={problem !== null && name !== '' ? 'invalid' : ''}
        onChange={(e) => {
          setName(e.target.value.trim());
        }}
        onKeyDown={(e) => {
          if (e.key === 'Enter' && problem === null) {
            finish(name === initial ? undefined : name);
          } else if (e.key === 'Escape') {
            finish(undefined);
          }
        }}
        onBlur={() => {
          finish(problem === null && name !== initial ? name : undefined);
        }}
      />
      {problem !== null && name !== '' && <div className="settings-problem">{problem}</div>}
    </div>
  );
}

/** 描いた見本の画像。 */
function FrameImage({ frame }: { frame: CanvasFrame }) {
  const ref = useRef<HTMLCanvasElement>(null);
  useEffect(() => {
    const canvas = ref.current;
    if (!canvas) {
      return;
    }
    const image = new ImageData(new Uint8ClampedArray(frame.pixels), frame.width, frame.height);
    canvas.getContext('2d')?.putImageData(image, 0, 0);
  }, [frame]);
  return (
    <canvas
      ref={ref}
      className="theme-preview-image"
      width={frame.width}
      height={frame.height}
      style={{ width: frame.width / frame.scale, height: frame.height / frame.scale }}
    />
  );
}

function ThemePreview() {
  const preview = useProject((s) => s.preview);
  if (!preview) {
    return <div className="settings-note">{l10n.t('Drawing...')}</div>;
  }
  if (preview.error !== undefined) {
    return <div className="settings-problem">{preview.error}</div>;
  }
  return (
    <div className="theme-preview">
      {(
        [
          [l10n.t('Light'), preview.light],
          [l10n.t('Dark'), preview.dark],
        ] as const
      ).map(([label, frame]) => (
        <figure key={label}>
          {frame && <FrameImage frame={frame} />}
          <figcaption>{label}</figcaption>
        </figure>
      ))}
    </div>
  );
}

function ThemeSection({ project }: { project: TtprojDocument }) {
  const target = useProject((s) => s.themeTarget);
  const usage = useProject((s) => s.presetUsage);
  const diagnostics = useProject((s) => s.diagnostics);
  const [renaming, setRenaming] = useState<string | undefined>(undefined);
  const [adding, setAdding] = useState(false);
  const theme = project.theme ?? {};
  const presets = theme.presets ?? {};
  const value = (target === null ? theme.override : presets[target]) as PropertyValue | undefined;
  return (
    <Section title={l10n.t('Theme')}>
      <Field label={l10n.t('Color scheme')} hint={l10n.t('Color scheme of the application')}>
        <select
          value={theme.color_scheme ?? 'SYSTEM'}
          onChange={(e) => {
            const v = e.target.value;
            setTheme((t) => {
              if (v === 'LIGHT' || v === 'DARK') {
                return { ...t, color_scheme: v };
              }
              // 既定(SYSTEM)は書かない
              const next = { ...t };
              Reflect.deleteProperty(next, 'color_scheme');
              return next;
            });
          }}
        >
          <option value="SYSTEM">SYSTEM — {l10n.t('Follow the OS setting')}</option>
          <option value="LIGHT">LIGHT — {l10n.t('Always light')}</option>
          <option value="DARK">DARK — {l10n.t('Always dark')}</option>
        </select>
      </Field>
      <div className="theme-layout">
        <div className="theme-list">
          <div
            className={`theme-list-item${target === null ? ' selected' : ''}`}
            onClick={() => {
              selectTheme(null);
            }}
          >
            <span className="theme-list-name">{l10n.t('Project theme')}</span>
          </div>
          <div className="theme-list-head">{l10n.t('Registered themes')}</div>
          {Object.keys(presets).map((name) =>
            renaming === name ? (
              <NameInput
                key={name}
                initial={name}
                presets={presets}
                onDone={(to) => {
                  setRenaming(undefined);
                  if (to !== undefined) {
                    renamePreset(name, to);
                  }
                }}
              />
            ) : (
              <div
                key={name}
                className={`theme-list-item${target === name ? ' selected' : ''}`}
                onClick={() => {
                  selectTheme(name);
                }}
              >
                <span className="theme-list-name">{name}</span>
                {(usage[name] ?? 0) > 0 && (
                  <span
                    className="tree-count"
                    title={l10n.t('Used by {0} widget(s)', usage[name] ?? 0)}
                  >
                    {usage[name]}
                  </span>
                )}
                <button
                  className="secondary"
                  title={l10n.t('Rename (the references in the windows are also renamed)')}
                  onClick={(e) => {
                    e.stopPropagation();
                    setRenaming(name);
                  }}
                >
                  ✎
                </button>
                <button
                  className="secondary"
                  title={l10n.t('Delete')}
                  onClick={(e) => {
                    e.stopPropagation();
                    renamePreset(name, null);
                  }}
                >
                  ×
                </button>
              </div>
            ),
          )}
          {adding ? (
            <NameInput
              initial=""
              presets={presets}
              onDone={(name) => {
                setAdding(false);
                if (name !== undefined) {
                  setTheme((t) => ({ ...t, presets: { ...t.presets, [name]: {} } }));
                  selectTheme(name);
                }
              }}
            />
          ) : (
            <button
              className="secondary theme-add"
              onClick={() => {
                setAdding(true);
              }}
            >
              + {l10n.t('Register a theme')}
            </button>
          )}
        </div>
        <div className="theme-editor">
          <p className="settings-note">
            {target === null
              ? l10n.t('Overrides the standard light and dark themes for the whole application.')
              : l10n.t(
                  'Widgets use this theme with "{0}" in theme_override. It is generated as {1}_theme().',
                  target,
                  target,
                )}
          </p>
          <ThemeOverrideField
            // テーマを切り替えたら入力欄を作り直す
            key={target ?? ''}
            value={value}
            onCommit={(next) => {
              setTheme((t) => {
                if (target !== null) {
                  // 登録したテーマは、空になっても登録したまま残す
                  return {
                    ...t,
                    presets: { ...t.presets, [target]: next ?? {} } as Theme['presets'],
                  };
                }
                const out = { ...t };
                if (next === null) {
                  Reflect.deleteProperty(out, 'override');
                } else {
                  out.override = next as Theme['override'];
                }
                return out;
              });
            }}
          />
          {problemsAt(
            diagnostics,
            target === null ? ['theme', 'override'] : ['theme', 'presets', target],
          ).map((p) => (
            <div key={p} className="settings-problem">
              {p}
            </div>
          ))}
        </div>
        <ThemePreview />
      </div>
    </Section>
  );
}

export function ProjectApp() {
  const version = useProject((s) => s.version);
  const project = useProject((s) => s.project);
  const diagnostics = useProject((s) => s.diagnostics);
  const fileName = useProject((s) => s.fileName);
  const editError = useProject((s) => s.editError);
  if (version < 0) {
    return <div className="message">{l10n.t('Loading...')}</div>;
  }
  const openAsText = (
    <button
      className="secondary"
      onClick={() => {
        postMessage({ type: 'openAsText' });
      }}
    >
      {l10n.t('Open as Text')}
    </button>
  );
  if (!project) {
    // JSON として読めない・形が違うときは編集できない(テキストで直してもらう)
    return (
      <div className="message">
        <p>{l10n.t('The project file cannot be shown because its content is invalid.')}</p>
        <ul className="settings-problems">
          {diagnostics.map((d, i) => (
            <li key={i}>{d.message}</li>
          ))}
        </ul>
        {openAsText}
      </div>
    );
  }
  return (
    <div className="project">
      <header className="project-header">
        <h1>{l10n.t('Project Settings: {0}', fileName.replace(/\.ttproj\.json$/, ''))}</h1>
        <button
          title={l10n.t('Generate the code of all windows and the startup code')}
          onClick={() => {
            postMessage({ type: 'generateProject' });
          }}
        >
          {l10n.t('Generate Project Code')}
        </button>
        {openAsText}
        {editError !== undefined && <span className="edit-error">{editError}</span>}
      </header>
      <WindowsSection project={project} />
      <CodegenSection project={project} />
      <ThemeSection project={project} />
    </div>
  );
}
