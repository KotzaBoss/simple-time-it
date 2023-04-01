#include "timeit.hpp"

#include <vector>
#include <thread>
#include <algorithm>
#include <ranges>
namespace rs = std::ranges;

struct Foo {
    int i;

    Foo() = default;
    
    explicit
    Foo(const int _i)
        : i{_i}
    {}
    
    Foo(Foo&&) = default;

    auto operator= (Foo&&) -> Foo& = default;
};

auto func_vec(int i) -> std::vector<Foo> {
    auto v = std::vector<Foo>{5};
    rs::generate(v, [i] { return Foo{ (i + std::rand()) / 100'000 }; });
    return v;
}

auto operator<< (std::ostream& o, const std::vector<Foo>& v) -> std::ostream& {
    o << '[' << v.size() << ": ";
    rs::for_each(v, [&o] (const auto& f) {
        o << "Foo{" << f.i << "} ";
    });
    return o << ']';
}

auto func_just_sleep() -> void {
    std::this_thread::sleep_for(std::chrono::milliseconds{1500});
}

struct Accumulator {
    int i;
    auto operator() (int j) -> int {
        std::this_thread::sleep_for(std::chrono::seconds{j});
        return i += j;
    }
};

// Use this while removing the cerr<< to see a cleaner assembly
template<typename T>
extern auto black_box(T&&) -> void;

auto main() -> int {
    using namespace hoot::time;

    {
        std::cerr << "func_just_sleep() -> void\n";
        const auto ret = time_it<std::chrono::milliseconds>(&func_just_sleep);
        std::cerr << '\t' << ret << "\n\n";
    };

    {
        std::cerr << "[] {} -> double\n";
        const auto ret = time_it<std::chrono::seconds>([] {
            std::this_thread::sleep_for(std::chrono::milliseconds{1250});
            return 1.0;
        });
        std::cerr << '\t' << ret << "\n\n";
    };

    {
        std::cerr << "func_vec(2) -> vector<Foo>\n";
        auto ret = time_it<std::chrono::microseconds>(&func_vec, 2);
        std::cerr << '\t' << ret << "\n\n";
    };

    {
        auto acc = Accumulator{1};
        std::cerr << "Accumulator(1) + 2 -> int\n";
        auto ret = time_it<std::chrono::seconds>(acc, 2);
        std::cerr << '\t' << ret << "\n\n";
    };

    {
        std::cerr << "std::plus 10, 1\n";
        auto ret = time_it<std::chrono::nanoseconds>(std::plus{}, 10, 1);
        std::cerr << '\t' << ret << "\n\n";
    };

    {
        std::cerr << "void(void)\n";
        auto ret = time_it<std::chrono::nanoseconds>([] { std::cerr << "\t~~~\n";});
        std::cerr << '\t' << ret << "\n\n";
    };
}
