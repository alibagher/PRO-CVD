#include "procvd/pattern.hpp"

#include <algorithm>

#include <opencv2/imgproc.hpp>

namespace procvd {
namespace {

// OpenCV 8-bit HSV: hue is 0-179. Red wraps around 0, so it is the low band
// [0, kRedHueLow] OR the high band [kRedHueHigh, 179]. Orange begins ~13; pink
// is red-hued but low saturation, excluded by the saturation floor.
constexpr int kRedHueLow = 12;
constexpr int kRedHueHigh = 167;

}  // namespace

cv::Mat red_mask(const cv::Mat& bgr, const Config& cfg) {
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    cv::Mat ch[3];
    cv::split(hsv, ch);  // ch[0]=H, ch[1]=S, ch[2]=V

    const cv::Mat in_red_band = (ch[0] <= kRedHueLow) | (ch[0] >= kRedHueHigh);
    const cv::Mat sat_ok = ch[1] >= cvRound(cfg.sat_min * 255.0);
    const cv::Mat val_ok = ch[2] >= cvRound(cfg.val_min * 255.0);

    cv::Mat mask;
    cv::bitwise_and(in_red_band, sat_ok, mask);
    cv::bitwise_and(mask, val_ok, mask);
    return mask;  // CV_8U, 0 or 255
}

cv::Mat red_stripe_overlay(const cv::Mat& scene_bgr, const Config& cfg) {
    const cv::Mat red = red_mask(scene_bgr, cfg);
    cv::Mat overlay = cv::Mat::zeros(scene_bgr.size(), CV_8UC3);

    const int spacing = std::max(2, cfg.stripe_spacing);
    for (int y = 0; y < scene_bgr.rows; ++y) {
        const uchar* rrow = red.ptr<uchar>(y);
        cv::Vec3b* orow = overlay.ptr<cv::Vec3b>(y);
        for (int x = 0; x < scene_bgr.cols; ++x) {
            // ((y - x) mod spacing), normalised to [0, spacing): light half of
            // every diagonal stripe period. (y - x) is the right-to-left "\" axis.
            const int phase = ((y - x) % spacing + spacing) % spacing;
            if (rrow[x] && phase < spacing / 2) {
                orow[x] = cv::Vec3b(255, 255, 255);
            }
        }
    }
    return overlay;
}

}  // namespace procvd
