#pragma once

#include <opencv2/core.hpp>

#include "procvd/config.hpp"

// Color-via-Pattern (Herbst & Brinkman): mark red regions with diagonal stripes
// so CVD users distinguish red by texture. Pure functions — no hardware, no
// shared state. Mirror of python/calibration/cvd_pattern.py.

namespace procvd {

// Mask of strictly-red, sufficiently-saturated pixels.
// Returns CV_8U where 255 = patternable red, 0 = leave alone.
[[nodiscard]] cv::Mat red_mask(const cv::Mat& bgr, const Config& cfg);

// Projector overlay: black everywhere except white right-to-left diagonal
// stripes ("\") over the red regions of `scene_bgr`. Result is in camera space
// (same size as input); warp it to projector space before projecting.
[[nodiscard]] cv::Mat red_stripe_overlay(const cv::Mat& scene_bgr, const Config& cfg);

}  // namespace procvd
