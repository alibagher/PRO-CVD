// Full pure pipeline, headless: FakeCamera -> red_stripe_overlay -> warp_to_projector.
// Uses a non-trivial homography so the test proves the warp transforms
// coordinates rather than just copying the overlay.

#include <cstdio>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "procvd/config.hpp"
#include "procvd/geometry.hpp"
#include "procvd/pattern.hpp"
#include "fakes.hpp"
#include "check.hpp"

namespace {

// Strongly-lit pixels of a BGR frame (tolerates warp interpolation fringes).
cv::Mat lit_mask(const cv::Mat& bgr) {
    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::Mat mask;
    cv::threshold(gray, mask, 200, 255, cv::THRESH_BINARY);
    return mask;
}

}  // namespace

int main() {
    using namespace procvd;

    Config cfg;
    cfg.proj_w = 1280;  // small frames keep the test fast
    cfg.proj_h = 720;
    cfg.cam_w = 640;
    cfg.cam_h = 360;

    // H maps projector (X,Y) -> camera (0.5X, 0.5Y). warp_to_projector therefore
    // fills projector pixel (X,Y) from camera (0.5X, 0.5Y). So camera red at x<320
    // (left half of 640) ends up at projector X<640 (left half of 1280) — the 2x
    // scale is the thing under test, not a plain copy.
    const Homography H(0.5, 0.0, 0.0,
                       0.0, 0.5, 0.0,
                       0.0, 0.0, 1.0);

    // --- Case 1: red in the camera's left half ---
    cv::Mat scene(cfg.cam_h, cfg.cam_w, CV_8UC3, cv::Scalar(128, 128, 128));   // grey
    scene(cv::Rect(0, 0, cfg.cam_w / 2, cfg.cam_h)).setTo(cv::Scalar(0, 0, 200));  // red left

    FakeCamera cam(scene);
    const cv::Mat overlay = red_stripe_overlay(cam.fresh_frame(), cfg);
    const cv::Mat warped = warp_to_projector(overlay, H, cfg);
    CHECK(warped.cols == cfg.proj_w && warped.rows == cfg.proj_h);

    const cv::Mat lit = lit_mask(warped);
    const int lit_total = cv::countNonZero(lit);
    CHECK(lit_total > 0);  // the pipeline produced stripes

    // Stripes present left of X=640, absent right of it (8px margin around the seam).
    const int left = cv::countNonZero(lit(cv::Rect(0, 0, 632, cfg.proj_h)));
    const int right = cv::countNonZero(lit(cv::Rect(648, 0, cfg.proj_w - 648, cfg.proj_h)));
    CHECK(left > 0);
    CHECK(right == 0);

    // --- Case 2: all-grey camera -> projector output entirely black ---
    cv::Mat grey(cfg.cam_h, cfg.cam_w, CV_8UC3, cv::Scalar(128, 128, 128));
    FakeCamera cam2(grey);
    const cv::Mat warped2 = warp_to_projector(red_stripe_overlay(cam2.fresh_frame(), cfg), H, cfg);
    CHECK(cv::countNonZero(lit_mask(warped2)) == 0);

    std::printf("pipeline_test: lit_total=%d left=%d right=%d  PASS\n", lit_total, left, right);
    return 0;
}
