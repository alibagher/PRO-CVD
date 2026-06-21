#pragma once

#include <optional>
#include <vector>

#include <opencv2/core.hpp>

#include "procvd/config.hpp"

namespace procvd {

struct ICamera;
struct IProjector;

// Homography mapping projector pixels -> camera pixels for the current surface.
using Homography = cv::Matx33d;

// Project a ChArUco board, detect it, and compute the homography.
// Returns nullopt when the board can't be detected — a normal outcome the user
// retries, not an error. Hardware failures (camera read) throw.
[[nodiscard]] std::optional<Homography>
calibrate_geometry(ICamera& cam, IProjector& proj, const Config& cfg);

// Pure helper: least-squares homography from matched projector/camera points.
// nullopt if there are too few correspondences. Independently testable.
[[nodiscard]] std::optional<Homography>
homography_from_correspondences(const std::vector<cv::Point2f>& proj_pts,
                                const std::vector<cv::Point2f>& cam_pts);

// Warp a camera-space image into projector space using H (projector->camera).
// Applies inv(H) so camera pixels land back on their projector positions.
[[nodiscard]] cv::Mat
warp_to_projector(const cv::Mat& cam_img, const Homography& H, const Config& cfg);

}  // namespace procvd
