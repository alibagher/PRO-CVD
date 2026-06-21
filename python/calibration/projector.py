"""
Projector output for PRO-CVD.

Manages a fullscreen OpenCV window on the projector display.
All other modules use this as the single point of contact with the projector.

Config:
  DISPLAY_X / DISPLAY_Y — position of the projector in the Windows desktop.
  Windows: Settings → System → Display → check the projector's position.
  Default (1920, 0) assumes a 1920px-wide primary monitor with projector to the right.
"""

import cv2
import numpy as np

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------

PROJ_W    = 1920
PROJ_H    = 1080
DISPLAY_X = 19200   # x pixel where the projector screen starts in the Windows desktop
DISPLAY_Y = 0

_WIN_NAME = "PRO-CVD"

# ---------------------------------------------------------------------------
# ProjectorOutput
# ---------------------------------------------------------------------------

class ProjectorOutput:
    """
    Fullscreen window on the projector display.

    Usage:
        proj = ProjectorOutput()
        proj.show(frame)          # push any BGR frame
        proj.solid((0, 0, 255))   # project solid red (BGR)
        proj.black()              # projector off / minimum
        proj.close()
    """

    def __init__(self, display_x: int = DISPLAY_X, display_y: int = DISPLAY_Y):
        self._blank = np.zeros((PROJ_H, PROJ_W, 3), dtype=np.uint8)
        cv2.namedWindow(_WIN_NAME, cv2.WINDOW_NORMAL)
        cv2.moveWindow(_WIN_NAME, display_x, display_y)
        cv2.imshow(_WIN_NAME, self._blank)
        # TRUE fullscreen is required: otherwise the Windows taskbar/desktop is
        # projected at the screen edges and corrupts the structured-light decode
        # (the taskbar is a bright static band that overrides the row patterns).
        cv2.setWindowProperty(_WIN_NAME, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
        cv2.waitKey(200)  # give the window time to go fullscreen before any content is shown

    def show(self, frame: np.ndarray, flush: bool = False) -> None:
        """
        Push a BGR uint8 frame to the projector window. 
        BGR is the OpenCV default colour order. 
        it is the reverse of RGB due to integer storage and historical reasons.

        flush=True forces an immediate cv2.waitKey(1) — only needed when called
        outside the main loop (e.g. during calibration flashes).
        In the main loop, leave flush=False — the loop's own waitKey renders it.

        frame.shape[:2] extracts (height, width) from the frame. If it doesn't match the projector resolution,
        the frame is resized to fit. This allows flexible input sizes while ensuring correct projector output.
        """
        if frame.shape[:2] != (PROJ_H, PROJ_W):
            frame = cv2.resize(frame, (PROJ_W, PROJ_H))
        cv2.imshow(_WIN_NAME, frame)
        if flush:
            cv2.waitKey(1)

    def solid(self, bgr: tuple) -> None:
        """Project a flat solid colour — used for radiometric calibration flashes."""
        img = np.full((PROJ_H, PROJ_W, 3), bgr, dtype=np.uint8)
        self.show(img, flush=True)

    def black(self) -> None:
        """Project black (minimum light output)."""
        self.show(self._blank, flush=True)

    def close(self) -> None:
        cv2.destroyWindow(_WIN_NAME)
        cv2.waitKey(1)
