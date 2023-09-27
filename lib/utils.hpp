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

    auto show() -> void {
        fmt::print("[{}] spent: {} seconds.\n", m_label, m_tm.getTimeSec());
    }

    auto res() -> T {
        return m_res;
    }
};

template <typename F>
auto timeit(std::string label, F f) -> Timed<decltype(f())> {
    cv::TickMeter tick_meter{};
    tick_meter.start();
    auto res = f();
    tick_meter.stop();

    return Timed(label, tick_meter, res);
}
