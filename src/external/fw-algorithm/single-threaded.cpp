#include <iostream>
#include <vector>
#include <string>

#include "../../utilz/matrix.h"

using namespace std;
using namespace math::utilz;

void _impl(square_matrix<long> &matrix) {
    auto *matrix_data = matrix.get_pointer();

    const size_t matrix_size = matrix.size();
    for (size_t k = 0ULL; k < matrix_size; ++k) {
        long *k_row = &matrix_data[k * matrix_size];
        for (size_t i = 0ULL; i < matrix_size; ++i) {
            long *i_row = &matrix_data[i * matrix_size];

            long ik = i_row[k];

            long *__k_row = k_row;
            for (size_t j = 0ULL; j < matrix_size; ++j, ++i_row, ++__k_row) {
                long distance = ik + *__k_row;
                if (*i_row > distance)
                    *i_row = distance;
            };
        };
    };
};

int main()
{
    vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};
    square_matrix<long> a;

    for (const string& word : msg)
    {
        cout << word << " ";
    }
    cout << endl;
}
