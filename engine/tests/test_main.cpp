#include <cstring>

#include "test.h"

// -v prints each case before it runs, which is how you find the one
// that crashed rather than merely failed.
int main(int argc, char** argv) {
    const bool verbose = argc > 1 && std::strcmp(argv[1], "-v") == 0;

    for (const auto& c : test::registry()) {
        test::current() = c.name;
        if (verbose) {
            std::printf("running %s\n", c.name);
            std::fflush(stdout);
        }
        c.fn();
    }
    if (test::failures()) {
        std::printf("%d check(s) FAILED\n", test::failures());
        return 1;
    }
    std::printf("all %zu test case(s) passed\n", test::registry().size());
    return 0;
}
