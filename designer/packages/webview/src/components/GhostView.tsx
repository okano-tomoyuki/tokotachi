/** ドラッグ中にカーソルについて動かす像。 */
import { useDesigner } from '../store.ts';

export function GhostView() {
  const ghost = useDesigner((s) => s.ghost);
  const pointer = useDesigner((s) => s.pointer);
  if (!ghost || !pointer) {
    return null;
  }
  const style = { left: pointer.x - ghost.dx, top: pointer.y - ghost.dy };
  return ghost.image !== undefined ? (
    <img
      className="ghost"
      src={ghost.image}
      style={{ ...style, width: ghost.width, height: ghost.height }}
      alt=""
    />
  ) : (
    <div className="ghost ghost-label" style={style}>
      {ghost.label}
    </div>
  );
}
