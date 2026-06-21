// Port of python/calibration/geometry.py self-test: recover a known homography
// from synthetic projector/camera correspondences. No hardware needed.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include <opencv2/core.hpp>

#include "procvd/geometry.hpp"
#include "check.hpp"

int main() {
    using namespace procvd;

    const cv::Matx33d H_true(0.9, 0.05, 50.0,
                             0.02, 0.88, 30.0,
                             0.0001, 0.0001, 1.0);

    // Synthetic ChArUco-like grid of projector corners, mapped through H_true.
    std::vector<cv::Point2f> proj_pts;
    std::vector<cv::Point2f> cam_pts;
    for (int r = 1; r <= 7; ++r) {
        for (int c = 1; c <= 7; ++c) {
            const cv::Point2f p(static_cast<float>(c * 100), static_cast<float>(r * 100));
            const cv::Vec3d ph = H_true * cv::Vec3d(p.x, p.y, 1.0);
            proj_pts.push_back(p);
            cam_pts.emplace_back(static_cast<float>(ph[0] / ph[2]),
                                 static_cast<float>(ph[1] / ph[2]));
        }
    }

    const auto H_est = homography_from_correspondences(proj_pts, cam_pts);
    CHECK(H_est.has_value());

    // Normalise both by element (2,2) before comparing.
    const cv::Matx33d a = *H_est * (1.0 / (*H_est)(2, 2));
    const cv::Matx33d b = H_true * (1.0 / H_true(2, 2));
    double err = 0.0;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            err = std::max(err, std::abs(a(i, j) - b(i, j)));
        }
    }
    CHECK(err < 1e-3);

    std::printf("geometry_test: homography err=%.2e  PASS\n", err);
    return 0;
}
