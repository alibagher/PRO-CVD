#pragma once

namespace procvd {

// Which camera adapter to use.
//   Usb = a USB webcam (default; works on any Linux, including WSL for dev)
//   Csi = a Raspberry Pi camera module via libcamera/GStreamer
enum class CameraKind { Usb, Csi };

// All tunable runtime parameters in one place — no module globals.
// Override with designated initialisers, e.g.
//     Config cfg{ .cam_index = 1, .stripe_spacing = 6 };
struct Config {
    // Projector output
    int proj_w = 1920;
    int proj_h = 1080;
    int proj_display_x = 1920;   // x offset of the projector in the display layout
    int proj_display_y = 0;

    // Camera input
    CameraKind camera = CameraKind::Usb;  // set to Csi on the Raspberry Pi
    int cam_index = 0;                    // USB camera only: which /dev/videoN
    int cam_w = 1280;
    int cam_h = 720;

    // Pattern
    int stripe_spacing = 8;      // pixels per stripe cycle (e.g. 8 -> 4 lit + 4 dark)
    double sat_min = 0.20;       // saturation floor for red detection [0,1]
    double val_min = 0.15;       // brightness floor for red detection [0,1]

    // ChArUco geometry (inner corners across/down)
    int board_cols = 7;
    int board_rows = 7;
    int min_charuco_corners = 4; // minimum detected corners to accept a homography
};

}  // namespace procvd
