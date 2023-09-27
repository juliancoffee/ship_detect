#include <iostream>
#include <fmt/core.h>
#include "utils.hpp"

int fib(size_t n) {
    if (n < 3) {
        return 1;
    }

    return fib(n-1) + fib(n-2);
}

auto main() -> int {
    auto timed = timeit("fib", []() {
            return fib(30);
    });
    fmt::print("Hello. Fib(30) is {}.\n", timed.res());
    timed.show();
}
