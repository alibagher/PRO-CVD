"""
PRO-CVD — main runtime.

Aims a projector-camera at a surface, calibrates geometry with a ChArUco board,
then projects bright diagonal stripes onto the red regions of the scene so CVD
users can distinguish red by texture.

IMPORTANT — alignment rule
    Geometry (g) and the projected pattern (SPACE) must BOTH be done while the
    device is aimed at THE SAME SURFACE without moving. If you move the device
    between g and SPACE the projected stripes will be spatially misaligned.

    Easiest workflow: press  c  (combined) — geometry then project in one go.

Controls:
  c       COMBINED — aim at the surface and press c.
          Calibrates geometry, then snapshots the scene and projects the
          red-stripe pattern. Use this as your normal workflow.

  g       Geometry only — aim at the surface, press g. DO NOT MOVE before SPACE.

  SPACE   Pattern only — press after g (same position). Snapshots the scene,
          finds red regions, projects diagonal stripes onto them.

  q       quit
"""

import sys
import cv2
import numpy as np
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from calibration.projector   import ProjectorOutput, PROJ_W, PROJ_H
from calibration.geometry    import calibrate_geometry, fresh_frame
from calibration.cvd_pattern import red_stripe_overlay

# 0 is the default webcam; increment if you have multiple cameras.
CAM_INDEX = 1

# Flip the projector output if the image appears mirrored or upside-down.
#   1 = horizontal,  0 = vertical,  -1 = both,  None = no flip
PROJ_FLIP = None


# ---------------------------------------------------------------------------
# Pipeline
# ---------------------------------------------------------------------------

def build_proj_frame(scene_frame: np.ndarray, H: np.ndarray) -> np.ndarray:
    """
    Build the projector frame: red-stripe overlay (camera space) warped into
    projector space via inv(H).

    H is the homography projector->camera, so inv(H) maps camera pixels back to
    projector pixels — putting the lit stripes where they will land on the red
    surface.
    """
    overlay = red_stripe_overlay(scene_frame)
    warped  = cv2.warpPerspective(overlay, np.linalg.inv(H), (PROJ_W, PROJ_H))
    if PROJ_FLIP is not None:
        warped = cv2.flip(warped, PROJ_FLIP)
    return warped


def snapshot_and_project(cap, proj, H) -> None:
    """Grab a clean (projector-black) frame, build the pattern, project it."""
    proj.black()
    scene_frame = fresh_frame(cap)           # clean capture, no projection
    proj_frame  = build_proj_frame(scene_frame, H)
    proj.show(proj_frame, flush=True)
    print(f"  Projecting pattern. Lit pixels: {(proj_frame[..., 0] > 0).mean() * 100:.1f}%\n")


# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------

def main() -> None:
    cap = cv2.VideoCapture(CAM_INDEX)
    if not cap.isOpened():
        print(f"ERROR: cannot open camera {CAM_INDEX}.")
        sys.exit(1)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,  1280)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

    proj = ProjectorOutput()
    proj.black()

    print("\nPRO-CVD ready.")
    print("  c     -> COMBINED   *** AIM AT THE SURFACE, press c ***")
    print("  g     -> geometry only   (DO NOT MOVE before pressing SPACE)")
    print("  SPACE -> project pattern (immediately after g, same position)")
    print("  q     -> quit\n")

    H            = None    # homography projector->camera (set by g or c)
    projecting   = False

    while True:
        ret, frame = cap.read()
        if not ret:
            continue

        # --- Status overlay on the laptop preview ---
        preview = frame.copy()
        if projecting:
            status = "PROJECTING  |  c=recalibrate  SPACE=refresh  q=quit"
            colour = (0, 255, 0)
        elif H is not None:
            status = "GEOMETRY DONE — DO NOT MOVE — press SPACE now"
            colour = (0, 80, 255)
        else:
            status = "AIM AT THE SURFACE and press  c  to calibrate"
            colour = (0, 100, 255)
        cv2.putText(preview, status, (10, 28),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, colour, 2)
        cv2.imshow("PRO-CVD  [laptop preview]", preview)

        key = cv2.waitKey(1) & 0xFF

        if key == ord('q'):
            break

        # ------------------------------------------------------------------ c
        elif key == ord('c'):
            projecting = False
            proj.black()
            print("=" * 60)
            print("COMBINED — AIM AT THE SURFACE AND HOLD STILL")
            print("=" * 60)
            try:
                print("Step 1/2  geometry...")
                H = calibrate_geometry(cap, proj)
                print("Step 2/2  pattern...")
                snapshot_and_project(cap, proj, H)
                projecting = True
            except RuntimeError as e:
                print(f"  Calibration failed: {e}\n")
            continue

        # ------------------------------------------------------------------ g
        elif key == ord('g'):
            projecting = False
            proj.black()
            print("-" * 60)
            print("Geometry calibration — AIM AT THE SURFACE, DO NOT MOVE AFTER")
            print("-" * 60)
            try:
                H = calibrate_geometry(cap, proj)
                print("  Geometry done.")
                print("  *** DO NOT MOVE THE DEVICE — press SPACE immediately ***\n")
            except RuntimeError as e:
                print(f"  Geometry failed: {e}\n")
            continue

        # --------------------------------------------------------------- SPACE
        elif key == ord(' '):
            if H is None:
                print("  No geometry — aim at the surface and press c (or g then SPACE).\n")
                continue
            projecting = False
            print("Snapshot + project pattern...")
            try:
                snapshot_and_project(cap, proj, H)
                projecting = True
            except RuntimeError as e:
                print(f"  Pattern projection failed: {e}\n")
            continue

        # The projected frame is static — it holds until the next c / g / SPACE.

    cap.release()
    proj.black()
    proj.close()
    cv2.destroyAllWindows()
    print("Bye.")


if __name__ == "__main__":
    main()
