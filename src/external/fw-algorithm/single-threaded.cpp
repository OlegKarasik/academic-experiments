#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

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

template<typename T>
class rstore {
    private:
        size_t m_length;

        T* m_data;

    public:
        rstore(T* data, size_t length)
            : m_data(data)
            , m_length(length) { }

        T* operator*() {
            return this->m_data;
        }

        rstore& operator++() {
            this->m_data += this->m_length;
            return *this;
        }
        rstore  operator++(int) {
            rstore prev = *this;

            ++(*this);

            return prev;
        }
        rstore operator+(int distance) {
            return rstore(this->m_data + (this->m_length * distance), this->m_length);
        }

        bool operator==(const rstore &other) const noexcept {
            if (this != &other) {
                return this->m_data == other.m_data && this->m_length == other.m_length;
            };
            return true;
        };

        bool operator!=(const rstore &other) const noexcept {
            return !(*this == other);
        };
};

void set(rstore<long>& rs, long i, long j, long v) {
    (*(rs + i))[j] = v;
}
long get(rstore<long>& rs, long i, long j) {
    return (*(rs + i))[j];
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

    rstore<long> rs_begin(matrix, matrix_sz);
    rstore<long> rs_end = rs_begin + matrix_sz;

    set(rs_begin, 1, 2, 9);
    set(rs_begin, 1, 3, 2);
    set(rs_begin, 1, 6, 5);
    set(rs_begin, 3, 4, 3);
    set(rs_begin, 3, 6, 6);
    set(rs_begin, 4, 6, 4);
    set(rs_begin, 4, 2, 1);

    _impl(matrix, matrix_sz);

    long c = 0;
    std::for_each(rs_begin, rs_end, [&c, matrix_sz](long* v) -> void {
        for (size_t i = 0; i < matrix_sz; ++i) {
            if (v[i] != no_edge_value()) {
                std::cout << "m[" << c << "," << i << "] = " << v[i] << endl;
            }
        }
        ++c;
    });

    return 0;
}
