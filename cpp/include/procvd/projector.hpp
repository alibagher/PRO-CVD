#pragma once

#include <string>

#include <opencv2/core.hpp>

#include "procvd/config.hpp"

namespace procvd {

// Hardware port for projector output (single point of contact with the display).
struct IProjector {
    virtual ~IProjector() = default;
    virtual void show(const cv::Mat& bgr) = 0;  // push a BGR frame (resized to fit)
    virtual void black() = 0;                    // minimum light output
};

// Adapter: a fullscreen OpenCV window on the projector display. RAII.
class OpenCvProjector final : public IProjector {
public:
    explicit OpenCvProjector(const Config& cfg);
    ~OpenCvProjector() override;

    OpenCvProjector(const OpenCvProjector&) = delete;
    OpenCvProjector& operator=(const OpenCvProjector&) = delete;

    void show(const cv::Mat& bgr) override;
    void black() override;

private:
    Config cfg_;
    std::string win_;
    cv::Mat blank_;
};

}  // namespace procvd
