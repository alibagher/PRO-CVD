#include "procvd/projector.hpp"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace procvd {

OpenCvProjector::OpenCvProjector(const Config& cfg)
    : cfg_(cfg),
      win_("PRO-CVD"),
      blank_(cv::Mat::zeros(cfg.proj_h, cfg.proj_w, CV_8UC3)) {
    cv::namedWindow(win_, cv::WINDOW_NORMAL);
    cv::moveWindow(win_, cfg.proj_display_x, cfg.proj_display_y);
    // True fullscreen — otherwise the desktop/taskbar is projected at the edges.
    cv::setWindowProperty(win_, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    cv::imshow(win_, blank_);
    cv::waitKey(200);  // let the window go fullscreen before any content
}

OpenCvProjector::~OpenCvProjector() {
    cv::destroyWindow(win_);
    cv::waitKey(1);
}

void OpenCvProjector::show(const cv::Mat& bgr) {
    cv::Mat frame = bgr;
    if (frame.cols != cfg_.proj_w || frame.rows != cfg_.proj_h) {
        cv::resize(bgr, frame, cv::Size(cfg_.proj_w, cfg_.proj_h));
    }
    cv::imshow(win_, frame);
    cv::waitKey(1);
}

void OpenCvProjector::black() {
    cv::imshow(win_, blank_);
    cv::waitKey(1);
}

}  // namespace procvd
