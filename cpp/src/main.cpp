#include <cstdio>
#include <exception>

#include "procvd/app.hpp"
#include "procvd/camera.hpp"
#include "procvd/config.hpp"
#include "procvd/projector.hpp"

int main() {
    using namespace procvd;

    Config cfg;  // edit defaults here; CLI/file config can come later

    try {
        OpenCvCamera cam(cfg);
        OpenCvProjector proj(cfg);
        run(cam, proj, cfg);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "fatal: %s\n", e.what());
        return 1;
    }
    return 0;
}
