/** ドラッグ中にカーソルについて動かす像。 */
import { useDesigner } from '../store.ts';

export function GhostView() {
  const ghost = useDesigner((s) => s.ghost);
  const pointer = useDesigner((s) => s.pointer);
  const zoom = useDesigner((s) => s.zoom);
  if (!ghost || !pointer) {
    return null;
  }
  // 部品の像はキャンバスの倍率で出す
  const k = ghost.image !== undefined ? zoom : 1;
  const style = { left: pointer.x - ghost.dx * k, top: pointer.y - ghost.dy * k };
  return ghost.image !== undefined ? (
    <img
      className="ghost"
      src={ghost.image}
      style={{ ...style, width: ghost.width * k, height: ghost.height * k }}
      alt=""
    />
  ) : (
    <div className="ghost ghost-label" style={style}>
      {ghost.label}
    </div>
  );
}
