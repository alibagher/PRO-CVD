#pragma once

#include "procvd/config.hpp"

namespace procvd {

struct ICamera;
struct IProjector;

// Runtime loop: previews the camera and, on keypress, calibrates geometry and
// projects the red-stripe pattern.
//   c = combined (geometry + project)   g = geometry only
//   SPACE = (re)project pattern         q = quit
void run(ICamera& cam, IProjector& proj, const Config& cfg);

}  // namespace procvd
