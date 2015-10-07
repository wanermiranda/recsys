#include <cstddef>
#include <ostream>
#include <iostream>

template<std::size_t N, std::size_t M>
void func(int (&arr)[N][M])
{
    std::cout << "int[" << N << "][" << M << "]\n";
    for (std::size_t n = 0; n != N; ++n)
        for (std::size_t m = 0; m != M; ++m)
            std::cout << arr[n][m] << ' ';
    std::cout << '\n' << std::endl;
}

int main()
{
    int i1[2][3] = { { 4, 5, 6 }, { 7, 8, 9 } };
    int i2[4][2] = { { 1, 3 }, { 5, 7 }, { 9, 11 }, { 13, 15 } };
    func(i1);
    func(i2);
}
