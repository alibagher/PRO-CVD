#include "procvd/camera.hpp"

#include <stdexcept>
#include <string>

namespace procvd {

OpenCvCamera::OpenCvCamera(const Config& cfg) : cap_(cfg.cam_index) {
    if (!cap_.isOpened()) {
        throw std::runtime_error("cannot open camera index " +
                                 std::to_string(cfg.cam_index));
    }
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, cfg.cam_w);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, cfg.cam_h);
}

cv::Mat OpenCvCamera::fresh_frame() {
    cv::Mat frame;
    for (int i = 0; i < 3; ++i) {  // flush stale buffered frames
        cap_.read(frame);
    }
    if (frame.empty()) {
        throw std::runtime_error("camera read failed");
    }
    return frame;
}

}  // namespace procvd
