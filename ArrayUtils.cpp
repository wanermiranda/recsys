#include "ArrayUtils.h"

void debug_print_array(size_t M, size_t N,
                       float *const *array)
{
    for (int row = 0; row < M; row++) {
        debug_print_array(N, array[row]);
    }

}



void debug_print_array(size_t N, float const *array) {
#ifdef DEBUG
    std::cout << '[';
    for (int col = 0; col < N; col++) {
        std::cout << array[col] << ((col+1) < N? "," : "");
    }
    std::cout << ']' <<std::endl;
#endif
}

