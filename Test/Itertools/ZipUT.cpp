
#include <Itertools/Zip.hpp>

#include <iostream>
#include <list>
#include <string>
#include <vector>


void test_zip()
{
    std::vector<int>  ints {1, 2, 3, 4, 5};
    std::list<double> doubles {0.1, 0.2, 0.3};
    std::string       letters {"ABCDEFG"};
    for (auto [x, y, z] : itertools::zip(ints, doubles, letters))
    {
        std::cout << x << " " << y << " " << z << std::endl;
    }
}

int main()
{
    test_zip();

    return 0;
}
