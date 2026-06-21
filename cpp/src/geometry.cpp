#include "procvd/geometry.hpp"

#include <algorithm>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

// ChArUco changed module and API in OpenCV 4.7. Support both so the project
// builds across versions:
//   4.7+  -> cv::aruco::CharucoDetector, CharucoBoard ctor, generateImage (objdetect)
//   <=4.6 -> CharucoBoard::create, detectMarkers + interpolateCornersCharuco (aruco)
#if (CV_VERSION_MAJOR > 4) || (CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7)
#  define PROCVD_ARUCO_NEW 1
#  include <opencv2/objdetect.hpp>
#else
#  define PROCVD_ARUCO_NEW 0
#  include <opencv2/aruco.hpp>
#  include <opencv2/aruco/charuco.hpp>
#endif

#include "procvd/camera.hpp"
#include "procvd/projector.hpp"

namespace procvd {
namespace {

// Projector-space positions of the inner ChArUco corners, keyed by corner id
// (row-major, matching the ArUco convention). Pure — no hardware.
std::vector<cv::Point2f> proj_corner_table(const Config& cfg, int sq, int ox, int oy) {
    std::vector<cv::Point2f> table;
    table.reserve(static_cast<size_t>(cfg.board_cols) * cfg.board_rows);
    for (int r = 0; r < cfg.board_rows; ++r) {
        for (int c = 0; c < cfg.board_cols; ++c) {
            table.emplace_back(static_cast<float>(ox + (c + 1) * sq),
                               static_cast<float>(oy + (r + 1) * sq));
        }
    }
    return table;
}

}  // namespace

std::optional<Homography>
homography_from_correspondences(const std::vector<cv::Point2f>& proj_pts,
                                const std::vector<cv::Point2f>& cam_pts) {
    if (proj_pts.size() < 4 || proj_pts.size() != cam_pts.size()) {
        return std::nullopt;
    }
    const cv::Mat H = cv::findHomography(proj_pts, cam_pts, cv::RANSAC, 3.0);
    if (H.empty()) {
        return std::nullopt;
    }
    Homography out;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            out(i, j) = H.at<double>(i, j);
        }
    }
    return out;
}

cv::Mat warp_to_projector(const cv::Mat& cam_img, const Homography& H, const Config& cfg) {
    const cv::Matx33d inv = H.inv();
    cv::Mat warped;
    cv::warpPerspective(cam_img, warped, cv::Mat(inv), cv::Size(cfg.proj_w, cfg.proj_h));
    return warped;
}

std::optional<Homography>
calibrate_geometry(ICamera& cam, IProjector& proj, const Config& cfg) {
    const int sq = std::min(cfg.proj_w / (cfg.board_cols + 1),
                            cfg.proj_h / (cfg.board_rows + 1));
    const int bw = (cfg.board_cols + 1) * sq;
    const int bh = (cfg.board_rows + 1) * sq;
    const int ox = (cfg.proj_w - bw) / 2;
    const int oy = (cfg.proj_h - bh) / 2;

    const std::vector<cv::Point2f> proj_corners = proj_corner_table(cfg, sq, ox, oy);

    // Generate the board image with the version-appropriate ChArUco API.
    cv::Mat board_img;
#if PROCVD_ARUCO_NEW
    const cv::aruco::Dictionary dict =
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    const cv::aruco::CharucoBoard board(
        cv::Size(cfg.board_cols + 1, cfg.board_rows + 1),
        static_cast<float>(sq), static_cast<float>(sq) * 0.75f, dict);
    const cv::aruco::CharucoDetector detector(board);
    board.generateImage(cv::Size(bw, bh), board_img, /*marginSize=*/0, /*borderBits=*/1);
#else
    const cv::Ptr<cv::aruco::Dictionary> dict =
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    const cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(
        cfg.board_cols + 1, cfg.board_rows + 1,
        static_cast<float>(sq), static_cast<float>(sq) * 0.75f, dict);
    board->draw(cv::Size(bw, bh), board_img, /*marginSize=*/0, /*borderBits=*/1);
#endif

    cv::Mat canvas = cv::Mat::zeros(cfg.proj_h, cfg.proj_w, CV_8UC1);
    board_img.copyTo(canvas(cv::Rect(ox, oy, bw, bh)));
    cv::Mat canvas_bgr;
    cv::cvtColor(canvas, canvas_bgr, cv::COLOR_GRAY2BGR);
    proj.show(canvas_bgr);

    for (int attempt = 0; attempt < 5; ++attempt) {
        cv::Mat frame = cam.fresh_frame();
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Point2f> charuco_corners;
        std::vector<int> charuco_ids;
#if PROCVD_ARUCO_NEW
        detector.detectBoard(gray, charuco_corners, charuco_ids);
#else
        std::vector<int> marker_ids;
        std::vector<std::vector<cv::Point2f>> marker_corners;
        cv::aruco::detectMarkers(gray, dict, marker_corners, marker_ids);
        if (!marker_ids.empty()) {
            cv::aruco::interpolateCornersCharuco(marker_corners, marker_ids, gray, board,
                                                 charuco_corners, charuco_ids);
        }
#endif
        if (charuco_ids.size() < static_cast<size_t>(cfg.min_charuco_corners)) {
            continue;
        }

        // Match by corner id — orientation is unambiguous.
        std::vector<cv::Point2f> matched_proj;
        std::vector<cv::Point2f> matched_cam;
        for (size_t i = 0; i < charuco_ids.size(); ++i) {
            const int id = charuco_ids[i];
            if (id >= 0 && id < static_cast<int>(proj_corners.size())) {
                matched_proj.push_back(proj_corners[static_cast<size_t>(id)]);
                matched_cam.push_back(charuco_corners[i]);
            }
        }
        proj.black();
        return homography_from_correspondences(matched_proj, matched_cam);
    }

    proj.black();
    return std::nullopt;
}

}  // namespace procvd
