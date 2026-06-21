#pragma once

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "procvd/config.hpp"

namespace procvd {

// Hardware port: the runtime depends on this interface, not on OpenCV directly,
// so a fake camera can drive the whole pipeline in tests without hardware.
struct ICamera {
    virtual ~ICamera() = default;
    // A fresh frame with stale buffered frames flushed first. Throws on read failure.
    [[nodiscard]] virtual cv::Mat fresh_frame() = 0;
};

// Adapter: a real webcam / Pi camera via OpenCV. RAII — releases on destruction.
class OpenCvCamera final : public ICamera {
public:
    explicit OpenCvCamera(const Config& cfg);
    [[nodiscard]] cv::Mat fresh_frame() override;

private:
    cv::VideoCapture cap_;
};

}  // namespace procvd
