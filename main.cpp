#include <iostream>
#include <fmt/core.h>
#include "utils.hpp"

auto fib(int n) -> int {
    if (n < 3) {
        return 1;
    }
    return fib(n - 1) + fib(n - 2);
}

auto main() -> int {
    timeit("hello", []() {
            fmt::print("Hello.\n");
    }).trace();

    constexpr int N = 10;
    auto fib_res = timeit("fib", []() {
        return fib(N);
    }).trace();
    fmt::print("fib({}) is: {}.\n", N, fib_res);
}
