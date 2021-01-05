#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <typename T> class square_shape {
  using size_type = size_t;
  using pointer = T *;
  using reference = T &;
  using value_type = T;

private:
  const size_type m_s;

  pointer m_p;

public:
  class iterator {
  private:
    pointer m_p;

  public:
    iterator(pointer data) : m_p(data) {}

    iterator &operator++() {
      ++this->m_p;
      return *this;
    }
    iterator &operator++(int) {
      iterator p = this->m_p;

      ++(*this);

      return *this;
    }
    iterator &operator--() {
      --this->m_p;
      return *this;
    }
    iterator &operator--(int) {
      iterator p = this->m_p;

      --(*this);

      return *this;
    }

    reference operator*() { return *this->m_p; }
    reference operator->() { return *this->m_p; }

    iterator &operator=(pointer v) {
      this->m_p = v;
      return *this;
    }
    iterator &operator=(const iterator &o) {
      this->m_p = o.m_p;
      return *this;
    }

    reference operator[](size_type idx) { return this->m_p[idx]; }
    const reference operator[](size_type idx) const { return this->m_p[idx]; }

    bool operator==(const iterator &o) const noexcept {
      return this == &o || this->m_p == o.m_p;
    };
    bool operator!=(const iterator &o) const noexcept { return !(*this == o); };
  };

  square_shape(pointer data, size_type size)
      : m_p(data), m_s(size) {}

  size_type s() { return this->m_s; }

  iterator begin() { return iterator(this->m_p); }
  iterator end() { return iterator(this->m_p + (this->m_s * this->m_s)); }

  pointer operator[](size_type idx) { return &this->m_p[idx * this->m_s]; }

  bool operator==(const square_shape &o) const noexcept {
    return this == &o || (this->m_p == o.m_p && this->m_s == o.m_s && this->m_h == o.m_h);
  };
  bool operator!=(const square_shape &o) const noexcept { return !(*this == o); };
};

constexpr long no_edge_value() {
  return ((std::numeric_limits<long>::max)() / 2L) - 1L;
};

void _impl(square_shape<long> &shape) {
  for (size_t k = 0; k < shape.s(); ++k) {
    long *kr = shape[k];

    for (size_t i = 0; i < shape.s(); ++i) {
      long *ir = shape[i];
      long *r = kr;

      long v = ir[k];
      for (size_t j = 0; j < shape.s(); ++j, ++ir, ++r) {
        long distance = v + *r;
        if (*ir > distance)
          *ir = distance;
      };
    };
  };
};

int main(int argc, char *argv[]) {
  // if (argc < 2) {
  //     std::cerr << "Usage: " << argv[0] << ""
  //         << "Options:\n"
  //         << "\t-h,--help\t\tShow this help message\n"
  //         << "\t-d,--destination DESTINATION\tSpecify the destination path"
  //         << std::endl;
  // }
  size_t matrix_sz = 7;

  long *matrix = (long *)malloc(matrix_sz * matrix_sz * sizeof(long));
  if (matrix == nullptr) {
    std::cerr << "erro: can't allocate memory to hold input matrix";
    return 1;
  }

  // Create rectangular shape
  square_shape<long> shape(matrix, matrix_sz);

  // Fill it with default values
  std::fill(shape.begin(), shape.end(), no_edge_value());

  shape[1][2] = 9;
  shape[1][3] = 2;
  shape[1][6] = 5;
  shape[3][4] = 3;
  shape[3][6] = 6;
  shape[4][6] = 4;
  shape[4][2] = 1;

  _impl(shape);

  for (size_t i = 0; i < shape.s(); ++i)
    for (size_t j = 0; j < shape.s(); ++j)
      if (shape[i][j] != no_edge_value())
        std::cout << "m[" << i << "," << j << "] = " << shape[i][j] << endl;

  return 0;
}
