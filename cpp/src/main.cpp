#include <cstdio>
#include <exception>
#include <memory>

#include "procvd/app.hpp"
#include "procvd/camera.hpp"
#include "procvd/config.hpp"
#include "procvd/projector.hpp"

int main() {
    using namespace procvd;

    Config cfg;  // edit defaults here; CLI/file config can come later

    // ---- CAMERA: pick one ----
    // USB webcam is the default. On the Raspberry Pi with the camera module,
    // uncomment the next line:
    // cfg.camera = CameraKind::Csi;

    try {
        auto cam = make_camera(cfg);   // USB or Pi CSI, per cfg.camera
        OpenCvProjector proj(cfg);
        run(*cam, proj, cfg);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "fatal: %s\n", e.what());
        return 1;
    }
    return 0;
}
