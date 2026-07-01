#pragma once

#include <memory>

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

// Adapter: a USB webcam via OpenCV VideoCapture(index). RAII — releases on destruction.
class OpenCvCamera final : public ICamera {
public:
    explicit OpenCvCamera(const Config& cfg);
    [[nodiscard]] cv::Mat fresh_frame() override;

private:
    cv::VideoCapture cap_;
};

// Adapter: a Raspberry Pi camera module (CSI) via libcamera's GStreamer source.
// Same interface as OpenCvCamera — only how frames are acquired differs.
class CsiCamera final : public ICamera {
public:
    explicit CsiCamera(const Config& cfg);
    [[nodiscard]] cv::Mat fresh_frame() override;

private:
    cv::VideoCapture cap_;
};

// Build the camera selected by cfg.camera. Single entry point for the runtime —
// switch USB <-> Pi camera by changing cfg.camera, nothing else.
[[nodiscard]] std::unique_ptr<ICamera> make_camera(const Config& cfg);

}  // namespace procvd
