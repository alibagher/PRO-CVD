#include "procvd/app.hpp"

#include <cstdio>
#include <optional>
#include <string>

#include <opencv2/highgui.hpp>

#include "procvd/camera.hpp"
#include "procvd/geometry.hpp"
#include "procvd/pattern.hpp"
#include "procvd/projector.hpp"

namespace procvd {
namespace {

// Grab a clean (projector-black) frame, build the overlay, warp it, project it.
void project_pattern(ICamera& cam, IProjector& proj, const Homography& H,
                     const Config& cfg) {
    proj.black();
    const cv::Mat scene = cam.fresh_frame();
    const cv::Mat overlay = red_stripe_overlay(scene, cfg);
    const cv::Mat warped = warp_to_projector(overlay, H, cfg);
    proj.show(warped);
}

}  // namespace

void run(ICamera& cam, IProjector& proj, const Config& cfg) {
    const std::string preview = "PRO-CVD [preview]";
    std::printf("PRO-CVD ready.  c=combined  g=geometry  SPACE=project  q=quit\n");

    std::optional<Homography> H;

    for (;;) {
        const cv::Mat frame = cam.fresh_frame();
        cv::imshow(preview, frame);
        const int key = cv::waitKey(1) & 0xFF;

        if (key == 'q') {
            break;
        }
        if (key == 'g' || key == 'c') {
            proj.black();
            H = calibrate_geometry(cam, proj, cfg);
            if (!H) {
                std::printf("  geometry failed — aim at the surface and retry\n");
                continue;
            }
            std::printf("  geometry ok\n");
            if (key == 'c') {
                project_pattern(cam, proj, *H, cfg);
            }
        } else if (key == ' ') {
            if (!H) {
                std::printf("  no geometry yet — press c or g first\n");
                continue;
            }
            project_pattern(cam, proj, *H, cfg);
        }
    }

    proj.black();
    cv::destroyWindow(preview);
}

}  // namespace procvd
