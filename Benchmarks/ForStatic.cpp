#include "./BenchCommon.hpp"

static constexpr int N1 = 4, N2 = 4;

// ------------------------------- 2d ----------------------------------------

static void for2(benchmark::State& state)
{
    enda::array<double, 2> a(N1, N2);
    const long             l0 = a.indexmap().lengths()[0];
    const long             l1 = a.indexmap().lengths()[1];

    while (state.KeepRunning())
    {
        for (long i = 0; i < l0; ++i)
            for (long j = 0; j < l1; ++j)
            {
                benchmark::DoNotOptimize(a(i, j) = 10);
            }
    }
}
BENCHMARK(for2);

static void foreach2(benchmark::State& state)
{
    enda::array<double, 2> a(N1, N2);

    while (state.KeepRunning())
    {
        enda::for_each(a.shape(), [&a](auto x0, auto x1) { benchmark::DoNotOptimize(a(x0, x1) = 10); });
    }
}
BENCHMARK(foreach2);

static void foreach_static2(benchmark::State& state)
{
    enda::array<double, 2> a(N1, N2);

    while (state.KeepRunning())
    {
        enda::for_each_static<encode(std::array {N1, N2}), 0>(a.shape(), [&a](auto x0, auto x1) { benchmark::DoNotOptimize(a(x0, x1) = 10); });
    }
}
BENCHMARK(foreach_static2);
