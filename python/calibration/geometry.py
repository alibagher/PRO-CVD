"""
Geometry calibration — runs on a keypress.

Projects a ChArUco board, detects it in the camera, and computes a single
homography mapping projector pixels -> camera pixels for the current surface
pose. ChArUco corner IDs make the board orientation unambiguous.

This is the ONLY geometry method. The old Gray-code / structured-light package
and the radiometric solver were removed when the pipeline was simplified to
"project diagonal stripes onto red regions".

Output (homography H) feeds the runtime: a camera-space pattern overlay is
warped into projector space with inv(H) before projection.
"""

import sys
import time
from pathlib import Path

import cv2
import numpy as np

# Ensure 'python/' is on the path whether this file is imported or run directly
_python_root = Path(__file__).parent.parent
if str(_python_root) not in sys.path:
    sys.path.insert(0, str(_python_root))

from calibration.projector import ProjectorOutput, PROJ_W, PROJ_H

# ChArUco board geometry — inner corners across/down.
BOARD_COLS = 7
BOARD_ROWS = 7
# Square size that fits the board to the projector canvas
PROJ_SQUARE_PX = min(PROJ_W // (BOARD_COLS + 1), PROJ_H // (BOARD_ROWS + 1))

SETTLE_MS = 150
MAX_TRIES = 5

# ---------------------------------------------------------------------------
# ChArUco board (chessboard + ArUco markers — unambiguous orientation)
# ---------------------------------------------------------------------------

_ARUCO_DICT = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
_CHARUCO_BOARD = cv2.aruco.CharucoBoard(
    (BOARD_COLS + 1, BOARD_ROWS + 1),   # squares across × down (outer squares)
    PROJ_SQUARE_PX,                      # square side length (projector pixels)
    PROJ_SQUARE_PX * 0.75,               # marker side length
    _ARUCO_DICT,
)
_CHARUCO_DETECTOR = cv2.aruco.CharucoDetector(_CHARUCO_BOARD)


def make_projected_charuco() -> np.ndarray:
    """Generate a ChArUco board image sized for the projector, centred."""
    sq = PROJ_SQUARE_PX
    bw = (BOARD_COLS + 1) * sq
    bh = (BOARD_ROWS + 1) * sq
    ox = (PROJ_W - bw) // 2
    oy = (PROJ_H - bh) // 2
    board_img = _CHARUCO_BOARD.generateImage((bw, bh))
    canvas = np.zeros((PROJ_H, PROJ_W), dtype=np.uint8)
    canvas[oy:oy + bh, ox:ox + bw] = board_img
    return cv2.cvtColor(canvas, cv2.COLOR_GRAY2BGR)


def _proj_charuco_corners() -> dict[int, np.ndarray]:
    """
    Return {corner_id: (x, y)} for all inner ChArUco corners in projector pixels.
    Corner IDs match the ArUco convention (row-major, 0-indexed).
    """
    sq = PROJ_SQUARE_PX
    ox = (PROJ_W - (BOARD_COLS + 1) * sq) // 2
    oy = (PROJ_H - (BOARD_ROWS + 1) * sq) // 2
    corners = {}
    cid = 0
    for r in range(BOARD_ROWS):
        for c in range(BOARD_COLS):
            corners[cid] = np.array([(ox + (c + 1) * sq), (oy + (r + 1) * sq)],
                                    dtype=np.float32)
            cid += 1
    return corners


def fresh_frame(cap: cv2.VideoCapture) -> np.ndarray:
    """Read a fresh frame, flushing a couple of buffered stale ones first."""
    for _ in range(2):
        cap.read()
    ret, frame = cap.read()
    if not ret:
        raise RuntimeError("Camera read failed during geometry calibration.")
    return frame


# ---------------------------------------------------------------------------
# Geometry — homography from projected ChArUco board
# ---------------------------------------------------------------------------

def calibrate_geometry(
    cap: cv2.VideoCapture,
    proj: ProjectorOutput,
) -> np.ndarray:
    """
    Project a ChArUco board, detect it with ArUco markers, compute homography.
    Corner IDs make orientation unambiguous.

    Returns H (3×3 float64) mapping projector pixels -> camera pixels.
    Leaves the projector black on return.
    """
    pattern = make_projected_charuco()
    proj_map = _proj_charuco_corners()   # id → projector (x, y)

    proj.show(pattern, flush=True)
    time.sleep(SETTLE_MS / 1000)

    charuco_corners = charuco_ids = None
    for _ in range(MAX_TRIES):
        frame = fresh_frame(cap)
        grey = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        charuco_corners, charuco_ids, _, _ = _CHARUCO_DETECTOR.detectBoard(grey)
        if charuco_ids is not None and len(charuco_ids) >= 4:
            break
        time.sleep(0.05)
    else:
        proj.black()
        raise RuntimeError(
            f"Geometry calibration failed — detected "
            f"{len(charuco_ids) if charuco_ids is not None else 0} ChArUco corners "
            f"(need ≥ 4). Check lighting and that the projected board is fully visible."
        )

    proj.black()

    # Build matched point lists using corner IDs — orientation is unambiguous
    proj_matched = []
    cam_matched = []
    for i, cid in enumerate(charuco_ids.flatten()):
        if cid in proj_map:
            proj_matched.append(proj_map[cid])
            cam_matched.append(charuco_corners[i][0])

    if len(proj_matched) < 4:
        raise RuntimeError(f"Only {len(proj_matched)} matched corners — need ≥ 4.")

    proj_pts_arr = np.array(proj_matched, dtype=np.float32).reshape(-1, 1, 2)
    cam_pts_arr = np.array(cam_matched, dtype=np.float32).reshape(-1, 1, 2)

    H, mask = cv2.findHomography(proj_pts_arr, cam_pts_arr,
                                 cv2.RANSAC, ransacReprojThreshold=3.0)
    if H is None:
        raise RuntimeError("findHomography failed — not enough valid correspondences.")

    inliers = int(mask.sum()) if mask is not None else 0
    print(f"  Geometry: {inliers}/{len(proj_matched)} inliers  "
          f"({len(charuco_ids)} ChArUco corners detected)")
    return H


# ---------------------------------------------------------------------------
# Self-test (no projector needed — uses synthetic data)
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    # Verify the homography round-trip with synthetic correspondences.
    proj_pts = np.array(list(_proj_charuco_corners().values()), dtype=np.float32)

    # Synthetic camera points: apply a known homography
    H_true = np.array([[0.9, 0.05, 50],
                       [0.02, 0.88, 30],
                       [0.0001, 0.0001, 1.0]], dtype=np.float64)

    cam_pts_h = (H_true @ np.column_stack([proj_pts, np.ones(len(proj_pts))]).T).T
    cam_pts = (cam_pts_h[:, :2] / cam_pts_h[:, 2:3]).astype(np.float32)

    H_est, _ = cv2.findHomography(
        proj_pts.astype(np.float32).reshape(-1, 1, 2),
        cam_pts.reshape(-1, 1, 2),
        cv2.RANSAC,
    )

    err = np.abs(H_est / H_est[2, 2] - H_true / H_true[2, 2]).max()
    print(f"Homography recovery error: {err:.2e}")
    assert err < 1e-4, f"Homography error too high: {err}"
    print("PASS")
