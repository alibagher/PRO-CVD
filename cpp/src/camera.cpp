#include "procvd/camera.hpp"

#include <memory>
#include <stdexcept>
#include <string>

namespace procvd {
namespace {

// Read a fresh frame, flushing buffered stale frames first. Throws on failure.
cv::Mat read_fresh(cv::VideoCapture& cap, const char* what) {
    cv::Mat frame;
    for (int i = 0; i < 3; ++i) {
        cap.read(frame);
    }
    if (frame.empty()) {
        throw std::runtime_error(std::string(what) + ": camera read failed");
    }
    return frame;
}

}  // namespace

// --- USB webcam -------------------------------------------------------------

OpenCvCamera::OpenCvCamera(const Config& cfg) : cap_(cfg.cam_index) {
    if (!cap_.isOpened()) {
        throw std::runtime_error("cannot open USB camera index " +
                                 std::to_string(cfg.cam_index));
    }
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, cfg.cam_w);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, cfg.cam_h);
}

cv::Mat OpenCvCamera::fresh_frame() { return read_fresh(cap_, "USB camera"); }

// --- Raspberry Pi camera module (CSI) via libcamera/GStreamer ---------------

CsiCamera::CsiCamera(const Config& cfg) {
    // libcamera -> GStreamer -> OpenCV. The appsink delivers BGR frames that map
    // straight onto a cv::Mat, so the rest of the pipeline is unchanged.
    const std::string pipeline =
        "libcamerasrc ! video/x-raw,width=" + std::to_string(cfg.cam_w) +
        ",height=" + std::to_string(cfg.cam_h) + ",framerate=30/1 "
        "! videoconvert ! video/x-raw,format=BGR "
        "! appsink drop=true max-buffers=2";

    if (!cap_.open(pipeline, cv::CAP_GSTREAMER)) {
        throw std::runtime_error(
            "cannot open Pi CSI camera via GStreamer. Check that: 'rpicam-hello' "
            "shows video, 'gstreamer1.0-libcamera' is installed, and OpenCV was "
            "built with GStreamer support.");
    }
}

cv::Mat CsiCamera::fresh_frame() { return read_fresh(cap_, "CSI camera"); }

// --- Factory ----------------------------------------------------------------

std::unique_ptr<ICamera> make_camera(const Config& cfg) {
    if (cfg.camera == CameraKind::Csi) {
        return std::make_unique<CsiCamera>(cfg);
    }
    return std::make_unique<OpenCvCamera>(cfg);
}

}  // namespace procvd
