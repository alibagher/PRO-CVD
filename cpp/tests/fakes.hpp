#pragma once

#include <utility>

#include <opencv2/core.hpp>

#include "procvd/camera.hpp"
#include "procvd/projector.hpp"

// Hardware fakes — let the full pipeline run in tests with no camera/projector.

namespace procvd {

// ICamera that always returns a fixed frame.
class FakeCamera final : public ICamera {
public:
    explicit FakeCamera(cv::Mat frame) : frame_(std::move(frame)) {}
    [[nodiscard]] cv::Mat fresh_frame() override { return frame_.clone(); }

private:
    cv::Mat frame_;
};

// IProjector that records the last frame shown, so tests can assert on output.
class FakeProjector final : public IProjector {
public:
    void show(const cv::Mat& bgr) override { last = bgr.clone(); }
    void black() override { last = cv::Mat(); }

    cv::Mat last;
};

}  // namespace procvd
