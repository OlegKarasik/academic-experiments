#include <iostream>
#include <vector>
#include <string>

using namespace std;

void _impl(long* matrix, size_t matrix_sz) {
    for (size_t k = 0; k < matrix_sz; ++k) {
        long *kr = &matrix[k * matrix_sz];

        for (size_t i = 0; i < matrix_sz; ++i) {
            long* ir = &matrix[i * matrix_sz];
            long* r = kr;

            long v = ir[k];
            for (size_t j = 0; j < matrix_sz; ++j, ++ir, ++r) {
                long distance = v + *r;
                if (*ir > distance)
                    *ir = distance;
            };
        };
    };
};

void set(long* matrix, size_t sz, long i, long j, long v) {
    matrix[i * sz + j] = v;
}
long get(long* matrix, size_t sz, long i, long j) {
    return matrix[i * sz + j];
}

constexpr long no_edge_value() { return ((std::numeric_limits<long>::max)() / 2L) - 1L; };

int main(int argc, char* argv[]) {
    // if (argc < 2) {
    //     std::cerr << "Usage: " << argv[0] << ""
    //         << "Options:\n"
    //         << "\t-h,--help\t\tShow this help message\n"
    //         << "\t-d,--destination DESTINATION\tSpecify the destination path"
    //         << std::endl;
    // }

    size_t matrix_sz = 7;

    long* matrix = (long*)malloc(matrix_sz * matrix_sz * sizeof(long));
    if (matrix == nullptr) {
        std::cerr << "erro: can't allocate memory to hold input matrix";
        return 1;
    }

    std::fill_n(matrix, matrix_sz * matrix_sz, no_edge_value());

    set(matrix, matrix_sz, 1, 2, 9);
    set(matrix, matrix_sz, 1, 3, 2);
    set(matrix, matrix_sz, 1, 6, 3);
    set(matrix, matrix_sz, 3, 4, 3);
    set(matrix, matrix_sz, 3, 6, 6);
    set(matrix, matrix_sz, 4, 6, 4);
    set(matrix, matrix_sz, 4, 2, 1);

    _impl(matrix, matrix_sz);

    for (size_t i = 0; i < matrix_sz; ++i) {
        for (size_t j = 0; j < matrix_sz; ++j) {
            long v = get(matrix, matrix_sz, i, j);
            if (v != no_edge_value()) {
                std::cout << "m[" << i << "," << j << "] = " << v << endl;
            }
        }
    };

    return 0;
}
