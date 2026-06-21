"""
Projector output test.

Cycles through test patterns so you can verify:
  1. The window appears on the correct display (the projector, not your laptop screen)
  2. Each colour is correct and fills the full screen
  3. The resolution looks right (no letterboxing / stretching)

Controls:
  SPACE   next pattern
  q       quit
"""

import sys
import cv2
import numpy as np
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))
from calibration.projector import ProjectorOutput, PROJ_W, PROJ_H


PATTERNS = [
    ("Black  (projector minimum)",  (  0,   0,   0)),
    ("White  (projector maximum)",  (255, 255, 255)),
    ("Red",                         (  0,   0, 255)),
    ("Green",                       (  0, 255,   0)),
    ("Blue",                        (255,   0,   0)),
    ("Grey 50%",                    (128, 128, 128)),
]


def make_grid(bgr: tuple) -> np.ndarray:
    """Solid colour with a white centre crosshair — confirms alignment and fill."""
    img = np.full((PROJ_H, PROJ_W, 3), bgr, dtype=np.uint8)
    cx, cy = PROJ_W // 2, PROJ_H // 2
    cv2.line(img, (cx - 40, cy), (cx + 40, cy), (255, 255, 255), 2)
    cv2.line(img, (cx, cy - 40), (cx, cy + 40), (255, 255, 255), 2)
    cv2.circle(img, (cx, cy), 30, (255, 255, 255), 1)
    # Corner markers so you can see all four edges
    for x, y in [(20, 20), (PROJ_W-20, 20), (20, PROJ_H-20), (PROJ_W-20, PROJ_H-20)]:
        cv2.drawMarker(img, (x, y), (255, 255, 255), cv2.MARKER_CROSS, 20, 2)
    return img


def main() -> None:
    proj  = ProjectorOutput()
    index = 0

    print("Projector window opened.")
    print("If it appeared on your laptop screen instead of the projector,")
    print(f"update DISPLAY_X in python/calibration/projector.py")
    print("\nPress SPACE to cycle patterns, q to quit.\n")

    while True:
        name, bgr = PATTERNS[index]
        frame = make_grid(bgr)
        proj.show(frame)
        print(f"  Pattern {index + 1}/{len(PATTERNS)}: {name}")

        key = cv2.waitKey(0) & 0xFF
        if key == ord(' '):
            index = (index + 1) % len(PATTERNS)
        elif key == ord('q'):
            break

    proj.black()
    proj.close()
    print("Done.")


if __name__ == "__main__":
    main()
