#include "TestCommon.hpp"

TEST(TraitsTest, Traits)
{
    static_assert(enda::is_instantiation_of_v<std::vector, std::vector<double>>, "INTERNAL");

    static_assert(enda::is_any_of<int, int, double>, "INTERNAL");
    static_assert(enda::is_any_of<double, int, double>, "INTERNAL");

    static_assert(enda::is_complex_v<std::complex<float>>, "INTERNAL");
    static_assert(enda::is_complex_v<std::complex<double>>, "INTERNAL");
    static_assert(not enda::is_complex_v<double>, "INTERNAL");

    static_assert(not enda::is_scalar_v<std::string>, "INTERNAL");
    static_assert(enda::is_scalar_v<int>, "INTERNAL");
    static_assert(enda::is_scalar_v<double>, "INTERNAL");
    static_assert(enda::is_scalar_v<std::complex<double>>, "INTERNAL");

    static_assert(enda::get_rank<std::vector<double>> == 1, "INTERNAL");
    static_assert(enda::get_rank<enda::vector<double>> == 1, "INTERNAL");
    static_assert(enda::get_rank<enda::matrix<double>> == 2, "INTERNAL");
    static_assert(enda::get_rank<enda::array<double, 4>> == 4, "INTERNAL");

    static_assert(not enda::is_view_v<enda::array<double, 4>>, "INTERNAL");
    static_assert(enda::is_view_v<enda::array_view<double, 4>>, "INTERNAL");

    static_assert(std::is_same_v<enda::get_value_t<enda::vector<double>>, double>, "INTERNAL");
    static_assert(std::is_same_v<enda::get_value_t<double>, double>, "INTERNAL");
}
