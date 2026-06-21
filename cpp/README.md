# PRO-CVD — C++ runtime

Real-time projector-camera runtime for the PRO-CVD assistive device. Calibrates
geometry with a ChArUco board, then projects diagonal stripes onto the red
regions of the scene so colour-vision-deficient users can distinguish red by
texture. Port of the Python prototype in [`../python/`](../python/).

## Design

A small **pure core** behind a **thin hardware shell**:

| Layer | Files | Depends on |
|---|---|---|
| Pure logic | `pattern.*`, `geometry.*` (math) | OpenCV data types only — no hardware |
| Hardware ports | `ICamera`, `IProjector` (interfaces) | nothing |
| Adapters | `OpenCvCamera`, `OpenCvProjector` | OpenCV videoio/highgui |
| Runtime | `app.*`, `main.cpp` | the above |

The pure functions take data (and the hardware *interfaces*), so the whole
pipeline is testable without a camera or projector — see `tests/`.

## Dependencies

- A C++20 compiler (GCC 12+, Clang 15+)
- CMake ≥ 3.21
- OpenCV 4 (core, imgproc, calib3d, videoio, highgui, aruco)

Install OpenCV:

```bash
# Debian/Ubuntu/Raspberry Pi OS
sudo apt-get update && sudo apt-get install -y libopencv-dev
```

> `geometry.cpp` auto-selects the ChArUco API by OpenCV version (a `CV_VERSION`
> check): `CharucoDetector`/`generateImage` on 4.7+, `CharucoBoard::create` +
> `interpolateCornersCharuco` on ≤4.6. No manual change needed.

## Build & test

```bash
cmake --preset dev          # configure (Debug + sanitizers)
cmake --build build/dev     # build
ctest --preset dev          # run the hardware-free tests

# optimised build
cmake --preset release && cmake --build build/release
./build/release/procvd      # run the device (needs a camera + projector)
```

## Controls

`c` combined (geometry + project) · `g` geometry only · `SPACE` (re)project ·
`q` quit. Geometry and projection must happen at the same surface pose without
moving the device.
