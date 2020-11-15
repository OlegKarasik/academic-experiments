#pragma once

#include <algorithm>
#include <functional>
#include <memory>

namespace math {
namespace utilz {
    template <typename T, typename A = std::allocator<T>>
    class square_matrix {
    public:
        class block_matrix {
        public:
            using allocator = typename std::allocator_traits<A>::template rebind_alloc<square_matrix>;

        private:
            square_matrix<square_matrix, allocator> m_matrix;

            size_t m_block_size;

        public:
            block_matrix()
                : m_matrix(square_matrix<square_matrix, allocator>())
                , m_block_size(0ULL) {};

            block_matrix(const square_matrix<square_matrix, allocator>& matrix,
                size_t block_size) noexcept
                : m_matrix(matrix)
                , m_block_size(block_size) {};

            block_matrix(const block_matrix& other) noexcept
                : m_matrix(other.m_matrix)
                , m_block_size(other.m_block_size) {};

            block_matrix(block_matrix&& other) noexcept
                : m_matrix(std::move(other.m_matrix))
                , m_block_size(other.m_block_size) {};

            ~block_matrix() = default;

            square_matrix* get_pointer() const
            {
                return this->m_matrix.get_pointer();
            };

            inline size_t size() const noexcept
            {
                return this->m_matrix.size();
            };

            inline size_t block_size() const noexcept
            {
                return this->m_block_size;
            };

            inline square_matrix* operator()(size_t row_idx)
            {
                return this->m_matrix(row_idx);
            };

            inline const square_matrix* operator()(size_t row_idx) const
            {
                return this->m_matrix(row_idx);
            };

            inline square_matrix& operator()(size_t row_idx,
                size_t col_idx)
            {
                return this->m_matrix(row_idx, col_idx);
            };

            inline const square_matrix& operator()(size_t row_idx,
                size_t col_idx) const
            {
                return this->m_matrix(row_idx, col_idx);
            };

            bool operator==(const block_matrix& other) const noexcept
            {
                if (this != &other) {
                    return this->m_block_size == other.m_block_size && this->m_matrix == other.m_matrix;
                };
                return true;
            };

            bool operator!=(const block_matrix& other) const noexcept
            {
                return !(*this == other);
            };

            block_matrix& operator=(const block_matrix& other) noexcept
            {
                if (this != &other) {
                    this->m_matrix = other.m_matrix;
                    this->m_block_size = other.m_block_size;
                };
                return *this;
            };

            block_matrix& operator=(block_matrix&& other) noexcept
            {
                if (this != &other) {
                    this->m_matrix = std::move(other.m_matrix);
                    this->m_block_size = other.m_block_size;
                };
                return *this;
            };
        };

    private:
        A m_allocator;

        size_t m_size;

        T* mp_data;

        void _alloc_resources()
        {
            size_t count = this->m_size * this->m_size;

            this->mp_data = std::allocator_traits<A>::allocate(this->m_allocator, count);
        };
        void _free_resources()
        {
            if (this->m_size > 0ULL) {
                size_t count = this->m_size * this->m_size;

                for (size_t i = 0ULL; i < count; ++i)
                    std::allocator_traits<A>::destroy(this->m_allocator, &this->mp_data[i]);

                std::allocator_traits<A>::deallocate(this->m_allocator, this->mp_data, count);
            };
        };

        void _move_transfer_resources(square_matrix& other) noexcept
        {
            this->mp_data = other.mp_data;

            other.m_size = 0ULL;
        };
        void _move_resources(square_matrix& other) noexcept
        {
            size_t count = this->m_size * this->m_size;
            for (size_t i = 0ULL; i < count; ++i)
                this->mp_data[i] = std::move(other.mp_data[i]);
        };

        void _copy_resources(const square_matrix& other) noexcept
        {
            size_t count = this->m_size * this->m_size;
            for (size_t i = 0ULL; i < count; ++i)
                std::allocator_traits<A>::construct(this->m_allocator, &this->mp_data[i], other.mp_data[i]);
        };

    public:
        square_matrix() noexcept
            : square_matrix(A()) {};

        square_matrix(const square_matrix& other)
            : m_allocator(std::allocator_traits<A>::select_on_container_copy_construction(other.m_allocator))
            , m_size(other.m_size)
            , mp_data(nullptr)
        {
            this->_alloc_resources();
            this->_copy_resources(other);
        };

        square_matrix(square_matrix&& other) noexcept(std::is_nothrow_move_constructible<A>::value)
            : m_allocator(std::move(other.get_allocator()))
            , m_size(other.m_size)
            , mp_data(nullptr)
        {
            this->_move_transfer_resources(other);
        };

        square_matrix(A allocator) noexcept
            : m_allocator(allocator)
            , m_size(0ULL)
            , mp_data(nullptr) {};

        square_matrix(size_t size)
            : square_matrix(size, A()) {};

        square_matrix(size_t size,
            A allocator)
            : m_allocator(allocator)
            , m_size(size)
            , mp_data(nullptr)
        {
#ifdef _DEBUG
            if (size == 0ULL)
                throw std::logic_error("size is zero");
#endif

            this->_alloc_resources();

            size_t count = this->m_size * this->m_size;
            for (size_t i = 0ULL; i < count; ++i)
                std::allocator_traits<A>::construct(this->m_allocator, &this->mp_data[i]);
        };

        square_matrix(size_t size,
            T default_value)
            : square_matrix(size, default_value, A()) {};

        square_matrix(size_t size,
            T default_value,
            A allocator)
            : m_allocator(allocator)
            , m_size(size)
        {
#ifdef _DEBUG
            if (size == 0ULL)
                throw std::logic_error("size is zero");
#endif

            this->_alloc_resources();

            size_t count = this->m_size * this->m_size;
            for (size_t i = 0ULL; i < count; ++i)
                std::allocator_traits<A>::construct(this->m_allocator, &this->mp_data[i], default_value);
        };

        ~square_matrix()
        {
            this->_free_resources();
        };

        inline A get_allocator() const
        {
            return this->m_allocator;
        };

        inline T* get_pointer() const
        {
            return this->mp_data;
        };

        block_matrix split(size_t block_size)
        {
#ifdef _DEBUG
            if (this->m_size % block_size != 0)
                std::logic_error("m_size is not dividable by block_size");
#endif

            size_t block_count = m_size / block_size;
            square_matrix<square_matrix, block_matrix::allocator> matrix(block_count, block_matrix::allocator(this->m_allocator));

            for (size_t s_row = 0ULL; s_row < block_count; ++s_row) {
                for (size_t s_col = 0ULL; s_col < block_count; ++s_col) {
                    square_matrix block(block_size);
                    for (size_t i = 0ULL; i < block_size; ++i) {
                        size_t offset = ((i + (s_row * block_size)) * this->m_size) + (s_col * block_size);
                        for (size_t j = 0ULL; j < block_size; ++j)
                            block(i, j) = this->mp_data[offset + j];
                    };
                    matrix(s_row, s_col) = std::move(block);
                };
            };
            return std::move(block_matrix(matrix, block_size));
        };

        void resize(size_t size,
            T default_value)
        {
#ifdef _DEBUG
            if (size == 0ULL)
                throw std::logic_error("size is zero");
#endif

            size_t round_size = min(size, this->m_size);

            square_matrix matrix(size, default_value, this->m_allocator);
            for (size_t i = 0ULL; i < round_size; ++i) {
                for (size_t j = 0ULL; j < round_size; ++j)
                    matrix(i, j) = std::move((*this)(i, j));
            };

            this->_free_resources();

            this->m_size = matrix.m_size;

            this->_move_transfer_resources(matrix);
        };

        void assign_values(const typename square_matrix::block_matrix& other)
        {
            if (other.block_size() > 0ULL) {
#ifdef _DEBUG
                if (this->m_size < other.size() * other.block_size())
                    throw std::logic_error("m_size is smaller than other.size() * other.block_size()");
#endif

                size_t size = other.size();
                size_t block_size = other.block_size();

                for (size_t i = 0ULL; i < size; ++i) {
                    size_t row = i * block_size;
                    for (size_t j = 0ULL; j < size; ++j) {
                        size_t col = j * block_size;

                        auto& block = other(i, j);
                        for (size_t _i = 0ULL; _i < block_size; ++_i) {
                            for (size_t _j = 0ULL; _j < block_size; ++_j)
                                (*this)(row + _i, col + _j) = block(_i, _j);
                        };
                    };
                };
            };
        };

        void assign_values(const square_matrix& other)
        {
#ifdef _DEBUG
            if (this->m_size != other.m_size)
                throw std::logic_error("other.m_size is not equal to this->m_size");
#endif

            size_t count = this->m_size * this->m_size;
            for (size_t i = 0ULL; i < count; ++i)
                this->mp_data[i] = other.mp_data[i];
        };

        inline bool empty() const noexcept
        {
            return this->m_count == 0ULL;
        };

        inline size_t size() const noexcept
        {
            return this->m_size;
        };

        inline T* operator()(size_t row_idx)
        {
#ifdef _DEBUG
            if (row_idx >= this->m_size)
                throw std::out_of_range("index was out of range");
#endif

            return &this->mp_data[row_idx * this->m_size];
        };

        inline const T* operator()(size_t row_idx) const
        {
#ifdef _DEBUG
            if (row_idx >= this->m_size)
                throw std::out_of_range("index was out of range");
#endif

            return &this->mp_data[row_idx * this->m_size];
        };

        inline T& operator()(size_t row_idx,
            size_t col_idx)
        {
#ifdef _DEBUG
            if (row_idx >= this->m_size)
                throw std::out_of_range("index was out of range");
            if (col_idx >= this->m_size)
                throw std::out_of_range("index was out of range");
#endif

            return this->mp_data[row_idx * this->m_size + col_idx];
        };

        inline const T& operator()(size_t row_idx,
            size_t col_idx) const
        {
#ifdef _DEBUG
            if (row_idx >= this->m_size)
                throw std::out_of_range("index was out of range");
            if (col_idx >= this->m_size)
                throw std::out_of_range("index was out of range");
#endif

            return this->mp_data[row_idx * this->m_size + col_idx];
        };

        bool operator==(const square_matrix& other) const noexcept
        {
            if (this != &other) {
                size_t count = this->m_size * this->m_size;
                return this->m_size == other.m_size && std::equal(this->mp_data, this->mp_data + count, other.mp_data, other.mp_data + count);
            };
            return true;
        };

        bool operator!=(const square_matrix& other) const noexcept
        {
            return !(*this == other);
        };

        square_matrix& operator=(const square_matrix& other) noexcept
        {
            if (this != &other) {
                this->_free_resources();

                if (std::allocator_traits<A>::propagate_on_container_copy_assignment::value) {
                    this->m_allocator = other.m_allocator;
                };

                this->m_size = other.m_size;

                this->_alloc_resources();
                this->_copy_resources(other);
            };
            return *this;
        };

        square_matrix& operator=(square_matrix&& other) noexcept(
            std::allocator_traits<A>::propagate_on_container_move_assignment::value&&
                std::is_nothrow_move_assignable<A>::value)
        {
            if (this != &other) {
                if (std::allocator_traits<A>::propagate_on_container_move_assignment::value) {
                    this->_free_resources();

                    this->m_allocator = std::move(other.m_allocator);
                    this->m_size = other.m_size;

                    this->_move_transfer_resources(other);
                } else {
                    if (this->get_allocator() == other.get_allocator()) {
                        this->_free_resources();

                        this->m_size = other.m_size;

                        this->_move_transfer_resources(other);
                    } else {
                        this->_move_resources(other);
                    };
                };
            };

            return *this;
        };
    };
}; // namespace utilz
}; // namespace math
