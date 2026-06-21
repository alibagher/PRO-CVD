# PRO-CVD

Projector-camera system to assist those with colour vision deficiency (CVD). A hand-held device is pointed at a scene, calibrates in ~1.5 seconds, then projects compensated colour in real-time.

## Hardware
Raspberry Pi · Pico Projector · Camera · Physical Button

## Stack
| | |
|---|---|
| Python | Current working prototype (runtime + calibration) |
| OpenCV | Camera capture, ChArUco detection, warp, CVD pattern, radiometric solve |
| C++ | Planned real-time port for the Raspberry Pi |
| Eigen | Matrix math for the radiometric solver (C++ port) |

## How it works
Run `python python/run.py`, then:

1. **Press `g`** (point at a plain light surface) — projects a **ChArUco board** and
   detects it to compute the projector↔camera homography for the current pose.
   ChArUco marker IDs make orientation unambiguous, so it works at any angle.
2. **Press `SPACE`** (point at the actual scene) — flashes 5 colour primaries
   (black, R, G, B, white) to estimate the per-pixel radiometric model `C = V_cp·P + F`.
3. **Compensation** — red regions get diagonal CvP stripes, the result is radiometrically
   compensated (`P = V_cp⁻¹·(D − F)`), warped to projector space, and projected.

Re-press `g` if the device or surface pose changes; re-press `SPACE` for each new scene.

See [Architecture/ARCHITECTURE.md](Architecture/ARCHITECTURE.md) for full diagrams and step outline.


### Structure
PRO-CVD/
  python/
    run.py                              ← entry point
    calibration/
      projector.py                      ← projector output
      scene_calibration.py              ← ChArUco geometry + radiometric calib (self-contained)
      cvd_pattern.py                    ← red detection + stripes
      radiometric_compensation.py       ← compensate_frame, estimate_scene_model
    tests/
      test_camera.py                    ← tune redness
      test_projector.py                 ← verify projector display
  cpp/                  ← empty, ready for C++ runtime later
  .venv/                ← isolated Python environment (gitignored)
  requirements.txt
