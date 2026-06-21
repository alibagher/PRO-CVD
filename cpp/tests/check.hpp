#pragma once

#include <cstdio>

// Minimal always-on check (asserts compile out under NDEBUG; this does not).
// Usage: CHECK(cond); inside a function returning int. Returns 1 on failure.
#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            std::fprintf(stderr, "FAIL: %s  (%s:%d)\n", #cond, __FILE__, __LINE__); \
            return 1;                                                     \
        }                                                                 \
    } while (0)
