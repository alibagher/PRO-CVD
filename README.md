# PRO-CVD

Projector-camera system to assist those with colour vision deficiency (CVD). A hand-held device is pointed at the scene, calibrates using Charuco board, and projects patterns depending on the amount of red present.

## Hardware
Raspberry Pi - Projector - Camera

## Stack
| | |
|---|---|
| OpenCV | Camera capture, ChArUco detection, warp, CVD pattern, radiometric solve |
| C++ | |

Use CMAKE because cross-platform, finds OpenCV and g++, easy to replicate. No need to build manually. big enough to use it.

### Structure
PRO-CVD/
  python/
    run.py                               entry point
    calibration/
      projector.py                       projector output
      scene_calibration.py               ChArUco geometry + radiometric calib (self-contained)
      cvd_pattern.py                     red detection + stripes
      radiometric_compensation.py        compensate_frame, estimate_scene_model
    tests/
      test_camera.py                     tune redness
      test_projector.py                  verify projector display
  cpp/                   empty, ready for C++ runtime later
  .venv/                 isolated Python environment (gitignored)
  requirements.txt
