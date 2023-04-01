#pragma once

#if !defined(__clang__) && !defined(__GNUC__)
#	error Was written with clang and gcc in mind
#elif __cplusplus < 202002L
#	error Was written in -std=c++20
#endif

#include <concepts>
#include <chrono>
#include <functional>
#include <iostream>

namespace hoot::time {

using Clock = std::chrono::steady_clock;

// Must only be called with designated initializer so that the result is evaluated first
// otherwise we wont be timing anything.
// Process 3) in https://en.cppreference.com/w/cpp/language/aggregate_initialization is the basis
// of this structure. There is no undefined behaviour to my knowledge.
template<typename Duration, typename Invoke_Result>
struct Time_It_Result final {
    Invoke_Result result;
    Duration duration;
};

// Void specialization
template<typename Duration>
struct Time_It_Result<Duration, void> final {
    Duration duration;
};

// Making a friend operator<< causes a redefinition error... why?
template<typename D, typename IR>
auto operator<< (std::ostream& o, const Time_It_Result<D, IR>& x) -> std::ostream& {
    if constexpr (std::same_as<IR, void>)
        return o << "Time_It_Result(duration=" << x.duration << ')';
    else
        return o << "Time_It_Result(result=" << x.result << ", duration=" << x.duration << ')';
}

template<typename Duration, typename Fn, typename... Args>
    requires std::invocable<Fn, Args...>
auto time_it(Fn&& fn, Args&&... args) -> Time_It_Result<Duration, std::invoke_result_t<Fn, Args...>> {
    if constexpr (const auto start = Clock::now();
        std::same_as<std::invoke_result_t<Fn, Args...>, void>)
    {
        std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);
        return {
            .duration = duration_cast<Duration>(Clock::now() - start)
        };
    }
    else
        return {
            .result = std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...),
            .duration = duration_cast<Duration>(Clock::now() - start),
        };
}

}// hoot::time

