"""
CVD pattern overlay for red regions — Color-via-Pattern (CvP).

Based on: Herbst & Brinkman, "Color-via-Pattern: Distinguishing Colors of
Confusion Without Affecting Perceived Brightness."

The overlay is a black projector frame, lit white only on the diagonal-stripe
pixels of red regions, so the projector adds bright diagonal lines on top of the
real red surface — making red distinguishable to CVD users by texture. Neutral
colours get no pattern.

Pipeline position (radiometric compensation was removed — simplified):
    camera frame  ──►  red_stripe_overlay()  ──►  warp to projector  ──►  projector
"""

import numpy as np
import cv2

# Strict red hue band in OpenCV HSV (hue 0-179). Red wraps around 0, so it is the
# low band (0–12) OR the high band (167–179). Orange starts ~13; pink is red hue
# but low saturation, so it's excluded by the saturation floor.
_RED_HUE_LOW  = 12
_RED_HUE_HIGH = 167


def red_mask(
    frame_bgr: np.ndarray,
    sat_min: float = 0.35,   # saturation floor — excludes pink/pastel
    val_min: float = 0.15,   # brightness floor — excludes very dark pixels
) -> np.ndarray:
    """
    Boolean mask of strictly-red, sufficiently-saturated pixels.

    frame_bgr : (H, W, 3) uint8 BGR
    returns   : (H, W) bool — True where the pixel should be patterned
    """
    h, s, v = cv2.split(cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2HSV))
    in_red_band = (h <= _RED_HUE_LOW) | (h >= _RED_HUE_HIGH)
    return in_red_band & (s >= sat_min * 255) & (v >= val_min * 255)


def red_stripe_overlay(
    frame_bgr: np.ndarray,
    spacing: int = 8,
    sat_min: float = 0.20,
) -> np.ndarray:
    """
    Build a projector overlay: black everywhere except white diagonal stripes
    (right-to-left, `\\`) over the red regions of `frame_bgr`.

    The result is in camera space (same size as the input). The runtime warps it
    into projector space with inv(H) before projecting, so the lit stripes land
    on the real red surface. Projectors only add light, hence black background.

    frame_bgr : (H, W, 3) uint8 BGR — clean camera-captured scene
    spacing   : pixels per stripe cycle (e.g. 8 → 4px lit + 4px dark)
    sat_min   : saturation floor for red detection
    returns   : (H, W, 3) uint8 BGR — black background, white stripes on red
    """
    rows, cols = np.indices(frame_bgr.shape[:2])
    stripes = (rows - cols) % spacing < spacing // 2     # every other diagonal
    lit = red_mask(frame_bgr, sat_min) & stripes

    overlay = np.zeros_like(frame_bgr)
    overlay[lit] = 255
    return overlay


# ---------------------------------------------------------------------------
# Self-test
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    H, W = 64, 64

    red_frame = np.zeros((H, W, 3), dtype=np.uint8)
    red_frame[..., 2] = 200                                 # solid red (BGR R=ch 2)
    grey_frame = np.full((H, W, 3), 128, dtype=np.uint8)    # neutral grey

    # --- red_mask: red detected, grey rejected ---
    assert red_mask(red_frame).mean() > 0.9, "Red frame should be mostly masked"
    assert red_mask(grey_frame).mean() == 0, "Grey frame should not be masked"
    print(f"red_mask coverage — red: {red_mask(red_frame).mean():.2f}  grey: {red_mask(grey_frame).mean():.2f}")

    # --- overlay: ~half the red frame lit, black/white only ---
    overlay = red_stripe_overlay(red_frame, spacing=8)
    lit_fraction = (overlay[..., 0] > 0).mean()
    assert 0.3 < lit_fraction < 0.7, f"Expected ~half lit, got {lit_fraction:.2f}"
    assert set(np.unique(overlay)).issubset({0, 255}), "Overlay should be black or white only"
    print(f"Red frame stripe coverage: {lit_fraction:.2f} (expected ~0.5)")

    # --- grey frame produces an all-black overlay ---
    assert red_stripe_overlay(grey_frame).max() == 0, "Grey frame should produce no stripes"
    print("Grey frame overlay is all black")

    print("PASS")
