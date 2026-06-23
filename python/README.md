## How it works
Run `python python/run.py`, then:

1. **Press `g`** (point at a plain light surface) — projects a **ChArUco board** and
   detects it to compute the projector-camera homography for the current pose.
   ChArUco marker IDs make orientation unambiguous, so it works at any angle.

Re-press `g` if the device or surface pose changes; re-press `SPACE` for each new scene.
