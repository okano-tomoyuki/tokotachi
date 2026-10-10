/**
 * 生成したソースの中のハンドラの位置(デザイナーからハンドラのコードへ移動するときに使う。Bethany から流用)。
 * 定義の見つけ方は、雛形を足すかの判定(cpp/syntax.ts の hasHandler)と同じ。
 */

/** 0 から数えた行と桁。 */
export interface HandlerPosition {
  readonly line: number;
  readonly character: number;
}

/**
 * ハンドラの本体の最初の行の末尾(カーソルを置く位置)。定義が無ければ undefined。
 * @param className 生成するクラスの名前(`MainWindow::search_button_click(` の判定に使う)
 */
export function findHandler(
  text: string,
  className: string,
  name: string,
): HandlerPosition | undefined {
  const lines = text.split(/\r?\n/);
  // className とハンドラの名前は識別子の検証を通っているので、正規表現の特殊文字を含まない
  const definition = new RegExp(`\\b${className}::${name}\\s*\\(`);
  const start = lines.findIndex((line) => definition.test(line));
  if (start < 0) {
    return undefined;
  }
  // 本体の始まり(`{`)。引数が複数行にわたることがある
  let head = start;
  while (head < lines.length && !/\{/.test(lines[head] ?? '')) {
    head++;
  }
  if (head >= lines.length) {
    return { line: start, character: 0 };
  }
  const headLine = lines[head] ?? '';
  const body = lines[head + 1];
  // `{` の後ろに本体が続いている、または次の行が無ければ、その行の末尾
  if (body === undefined || /\{\s*\S/.test(headLine)) {
    return { line: head, character: headLine.length };
  }
  // 本体が空(`{` の次の行が `}`)なら `{` の後ろ
  if (body.trim() === '}') {
    return { line: head, character: headLine.length };
  }
  return { line: head + 1, character: body.trimEnd().length };
}
