// Minimal test harness: TEST(name) registers a case, CHECK/CHECK_EQ
// record failures without aborting, main() in test_main.cpp runs all.

#pragma once

#include <cstdio>
#include <sstream>
#include <vector>

namespace test {

using TestFn = void (*)();
struct Case {
    const char* name;
    TestFn fn;
};

inline std::vector<Case>& registry() {
    static std::vector<Case> r;
    return r;
}
inline int& failures() {
    static int f = 0;
    return f;
}
inline const char*& current() {
    static const char* c = "";
    return c;
}

struct Register {
    Register(const char* name, TestFn fn) { registry().push_back({name, fn}); }
};

template <class A, class B>
void check_eq(const A& a, const B& b, const char* ea, const char* eb,
              const char* file, int line) {
    if (!(a == b)) {
        ++failures();
        std::ostringstream va, vb;
        va << a;
        vb << b;
        std::printf("FAIL %s (%s:%d): %s == %s  [%s vs %s]\n", current(), file,
                    line, ea, eb, va.str().c_str(), vb.str().c_str());
    }
}

}  // namespace test

#define TEST(name)                                              \
    static void test_##name();                                  \
    static ::test::Register reg_##name(#name, test_##name);     \
    static void test_##name()

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            ++::test::failures();                                            \
            std::printf("FAIL %s (%s:%d): %s\n", ::test::current(), __FILE__, \
                        __LINE__, #cond);                                    \
        }                                                                    \
    } while (0)

#define CHECK_EQ(a, b) ::test::check_eq((a), (b), #a, #b, __FILE__, __LINE__)
