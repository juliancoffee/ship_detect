#pragma once

#include <opencv2/core/utility.hpp>
#include <fmt/core.h>

template <typename T>
class Timed {
    std::string m_label;
    cv::TickMeter m_tm;
    T m_res;

    public:
    Timed(std::string label, cv::TickMeter tm, T res)
        : m_label(label), m_tm(tm), m_res(res) {}

    template <typename F>
    static auto timeit(std::string label, F f) -> Timed<T> {
        cv::TickMeter tick_meter{};
        tick_meter.start();
        auto res = f();
        tick_meter.stop();

        return Timed(label, tick_meter, res);
    }

    auto trace() -> T {
        fmt::print("[{}] spent: {} seconds.\n", m_label, m_tm.getTimeSec());

        return m_res;
    }
};

// make it compile with void lambdas
template <>
class Timed<void> {
    std::string m_label;
    cv::TickMeter m_tm;

public:
    Timed(std::string label, cv::TickMeter tm)
        : m_label(label), m_tm(tm) {}

    template <typename F>
    static auto timeit(std::string label, F f) -> Timed<void> {
        cv::TickMeter tick_meter{};
        tick_meter.start();
        f();
        tick_meter.stop();

        return Timed(label, tick_meter);
    }

    auto trace() -> void {
        fmt::print("[{}] spent: {} seconds.\n", m_label, m_tm.getTimeSec());
    }
};

// expose helper to avoid typing type each type
template <typename F>
auto timeit(std::string label, F f) -> Timed<decltype(f())> {
    return Timed<decltype(f())>::timeit(label, f);
}
