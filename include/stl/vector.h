#pragma once

#include "allocator.h"
#include "log_interface.h"
#include "log_macros.h"

namespace bluestl
{

    template <typename T, typename Allocator = allocator>
    class vector
    {
    public:
        using value_type = T;
        using allocator_type = Allocator;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        explicit vector(const Allocator &allocator)
            : m_allocator(allocator), m_data(nullptr), m_size(0), m_capacity(0) {}

        ~vector()
        {
            clear();
            if (m_data)
                m_allocator.deallocate(m_data, m_capacity * sizeof(T));
        }

        void push_back(const T &value)
        {
            if (m_size >= m_capacity)
                reserve((m_capacity == 0) ? 4 : m_capacity * 2);
            new (&m_data[m_size]) T(value);
            ++m_size;
        }

        void pop_back()
        {
            CONTAINER_ASSERT(m_size > 0);
            m_data[--m_size].~T();
        }

        void clear()
        {
            for (size_t i = 0; i < m_size; ++i)
                m_data[i].~T();
            m_size = 0;
        }

        void reserve(size_t new_capacity)
        {
            if (new_capacity <= m_capacity)
                return;
            T *new_data = static_cast<T *>(m_allocator.allocate(new_capacity * sizeof(T)));
            for (size_t i = 0; i < m_size; ++i)
                new (&new_data[i]) T(std::move(m_data[i]));
            for (size_t i = 0; i < m_size; ++i)
                m_data[i].~T();
            if (m_data)
                m_allocator.deallocate(m_data, m_capacity * sizeof(T));
            m_data = new_data;
            m_capacity = new_capacity;
        }

        size_t size() const { return m_size; }
        size_t capacity() const { return m_capacity; }

        T &operator[](size_t i) { return m_data[i]; }
        const T &operator[](size_t i) const { return m_data[i]; }

        iterator begin() noexcept { return m_data; }
        const_iterator begin() const noexcept { return m_data; }
        iterator end() noexcept { return m_data + m_size; }
        const_iterator end() const noexcept { return m_data + m_size; }
        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
        reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        // 先頭・末尾要素への参照
        T& front() { return m_data[0]; }
        const T& front() const { return m_data[0]; }
        T& back() { return m_data[m_size - 1]; }
        const T& back() const { return m_data[m_size - 1]; }

        // 空かどうか
        bool empty() const noexcept { return m_size == 0; }

        // 範囲チェック付きアクセス（例外は使わずアサート）
        T& at(size_t i) {
            CONTAINER_ASSERT(i < m_size);
            return m_data[i];
        }
        const T& at(size_t i) const {
            CONTAINER_ASSERT(i < m_size);
            return m_data[i];
        }

        // サイズ変更
        void resize(size_t new_size, const T& value = T()) {
            if (new_size < m_size) {
                for (size_t i = new_size; i < m_size; ++i) {
                    m_data[i].~T();
                }
            } else if (new_size > m_size) {
                reserve(new_size);
                for (size_t i = m_size; i < new_size; ++i) {
                    new (&m_data[i]) T(value);
                }
            }
            m_size = new_size;
        }

        // swap
        void swap(vector& other) noexcept {
            using std::swap;
            swap(m_allocator, other.m_allocator);
            swap(m_data, other.m_data);
            swap(m_size, other.m_size);
            swap(m_capacity, other.m_capacity);
        }

        // assign
        void assign(size_t count, const T& value) {
            clear();
            reserve(count);
            for (size_t i = 0; i < count; ++i) {
                new (&m_data[i]) T(value);
            }
            m_size = count;
        }

        // insert: 単一要素
        iterator insert(iterator pos, const T& value) {
            size_t idx = pos - m_data;
            CONTAINER_ASSERT(idx <= m_size);
            if (m_size >= m_capacity)
                reserve((m_capacity == 0) ? 4 : m_capacity * 2);
            for (size_t i = m_size; i > idx; --i) {
                new (&m_data[i]) T(std::move(m_data[i - 1]));
                m_data[i - 1].~T();
            }
            new (&m_data[idx]) T(value);
            ++m_size;
            return m_data + idx;
        }

        // erase: 単一要素
        iterator erase(iterator pos) {
            size_t idx = pos - m_data;
            CONTAINER_ASSERT(idx < m_size);
            m_data[idx].~T();
            for (size_t i = idx; i < m_size - 1; ++i) {
                new (&m_data[i]) T(std::move(m_data[i + 1]));
                m_data[i + 1].~T();
            }
            --m_size;
            return m_data + idx;
        }

    private:
        Allocator m_allocator;
        T *m_data;
        size_t m_size;
        size_t m_capacity;
    };
}