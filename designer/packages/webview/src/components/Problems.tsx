/** 検証の結果の一覧(問題パネルと同じ内容を、デザイナーの中にも出す)。 */
import { useDesigner } from '../store.ts';

export function Problems() {
  const diagnostics = useDesigner((s) => s.diagnostics);
  if (diagnostics.length === 0) {
    return null;
  }
  return (
    <ul className="problems">
      {diagnostics.map((d, i) => (
        <li key={i} className={d.severity}>
          <span className="problem-path">{d.path.join('.')}</span> {d.message}
        </li>
      ))}
    </ul>
  );
}
