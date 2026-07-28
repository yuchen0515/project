#include "test.h"

int main() {
    for (const auto& c : test::registry()) {
        test::current() = c.name;
        c.fn();
    }
    if (test::failures()) {
        std::printf("%d check(s) FAILED\n", test::failures());
        return 1;
    }
    std::printf("all %zu test case(s) passed\n", test::registry().size());
    return 0;
}
