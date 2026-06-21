"""
Live camera test for the CVD pattern pipeline.

Shows three panels side-by-side:
  LEFT   — original camera frame
  CENTRE — redness heatmap (blue=0, red=1)
  RIGHT  — stripe overlay (black bg, white stripes on red — what gets projected)

Keyboard controls:
  q       quit
  +/-     increase / decrease stripe spacing
  t / T   increase / decrease saturation threshold (redness sensitivity)
  p       cycle pattern spacing preset  (fine=4, normal=8, coarse=16)

Run from the project root:
    python tests/test_camera.py
"""

import sys
import cv2
import numpy as np

sys.path.insert(0, str(__import__("pathlib").Path(__file__).parent.parent))
from calibration.cvd_pattern import red_stripe_overlay, red_mask


def draw_params(frame: np.ndarray, spacing: int, sat_min: float) -> None:
    params = [
        f"spacing  : {spacing}  (+/-)",
        f"sat_min  : {sat_min:.2f}  (t/T)",
    ]
    for i, text in enumerate(params):
        cv2.putText(frame, text, (8, 20 + i * 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)
        cv2.putText(frame, text, (8, 20 + i * 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 2, cv2.LINE_AA)
        cv2.putText(frame, text, (8, 20 + i * 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)


def make_heatmap(frame_bgr: np.ndarray, sat_min: float) -> np.ndarray:
    """Highlight the detected red region — exactly the pixels that get patterned."""
    mask_u8 = (red_mask(frame_bgr, sat_min) * 255).astype(np.uint8)
    return cv2.applyColorMap(mask_u8, cv2.COLORMAP_JET)


def main() -> None:
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("ERROR: could not open camera 0. Try changing the index in test_camera.py.")
        sys.exit(1)

    cap.set(cv2.CAP_PROP_FRAME_WIDTH,  640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    spacing  = 8
    sat_min  = 0.20
    spacing_presets = [4, 8, 16]
    preset_idx = 1

    print("Camera opened. Press 'q' to quit.")
    print("Point at something red to see the pattern overlay.")

    while True:
        ret, frame = cap.read()
        if not ret:
            print("ERROR: failed to read frame.")
            break

        heatmap  = make_heatmap(frame, sat_min)
        overlay  = red_stripe_overlay(frame, spacing=spacing, sat_min=sat_min)

        draw_params(overlay, spacing, sat_min)

        # Label each panel
        for panel, label in [(frame, "Original"),
                              (heatmap, "Redness"),
                              (overlay, "Overlay")]:
            cv2.putText(panel, label, (panel.shape[1] - 90, 18),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 0, 0),   2, cv2.LINE_AA)
            cv2.putText(panel, label, (panel.shape[1] - 90, 18),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255,255,255), 1, cv2.LINE_AA)

        combined = np.hstack([frame, heatmap, overlay])
        cv2.imshow("PRO-CVD  |  original | redness | overlay", combined)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('+') or key == ord('='):
            spacing = min(spacing + 2, 32)
        elif key == ord('-'):
            spacing = max(spacing - 2, 2)
        elif key == ord('t'):
            sat_min = min(round(sat_min + 0.05, 2), 0.80)
        elif key == ord('T'):
            sat_min = max(round(sat_min - 0.05, 2), 0.05)
        elif key == ord('p'):
            preset_idx = (preset_idx + 1) % len(spacing_presets)
            spacing = spacing_presets[preset_idx]

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
