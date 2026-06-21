// Port of python/calibration/cvd_pattern.py self-test. No hardware needed.

#include <cstdio>
#include <vector>

#include <opencv2/core.hpp>

#include "procvd/config.hpp"
#include "procvd/pattern.hpp"
#include "check.hpp"

int main() {
    using namespace procvd;
    const Config cfg;  // defaults

    const cv::Mat red(64, 64, CV_8UC3, cv::Scalar(0, 0, 200));        // solid red (BGR)
    const cv::Mat grey(64, 64, CV_8UC3, cv::Scalar(128, 128, 128));   // neutral grey
    const double total = 64.0 * 64.0;

    // --- red_mask: red detected, grey rejected ---
    const double red_cov = cv::countNonZero(red_mask(red, cfg)) / total;
    const double grey_cov = cv::countNonZero(red_mask(grey, cfg)) / total;
    CHECK(red_cov > 0.9);
    CHECK(grey_cov == 0.0);

    // --- overlay: ~half the red frame lit, black/white only ---
    const cv::Mat overlay = red_stripe_overlay(red, cfg);
    std::vector<cv::Mat> chans;
    cv::split(overlay, chans);
    const double lit = cv::countNonZero(chans[0]) / total;
    CHECK(lit > 0.3 && lit < 0.7);

    double mn = 0.0;
    double mx = 0.0;
    cv::minMaxLoc(overlay.reshape(1), &mn, &mx);
    CHECK(mn == 0.0);
    CHECK(mx == 0.0 || mx == 255.0);

    // --- grey frame produces an all-black overlay ---
    CHECK(cv::countNonZero(red_stripe_overlay(grey, cfg).reshape(1)) == 0);

    std::printf("pattern_test: red_cov=%.2f grey_cov=%.2f lit=%.2f  PASS\n",
                red_cov, grey_cov, lit);
    return 0;
}
