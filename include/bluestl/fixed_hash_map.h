#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include "hash.h"

namespace bluestl
{

	template <typename Key, typename T, std::size_t Capacity = 128>
	class fixed_hash_map
	{
	public:
		using key_type = Key;
		using mapped_type = T;
		using value_type = std::pair<const Key, T>;
		using size_type = std::size_t;
		using reference = value_type &;
		using const_reference = const value_type &;

		struct bucket
		{
			std::pair<Key, T> kv;
			bool used;
			constexpr bucket() noexcept : kv(), used(false) {}
		};

		constexpr fixed_hash_map() noexcept : size_(0) {}

		constexpr size_type size() const noexcept { return size_; }
		constexpr size_type capacity() const noexcept { return Capacity; }
		constexpr bool empty() const noexcept { return size_ == 0; }

		constexpr void clear() noexcept
		{
			for (size_type i = 0; i < Capacity; ++i)
				buckets_[i].used = false;
			size_ = 0;
		}

		constexpr T &operator[](const Key &key) noexcept
		{
			size_type idx = find_index(key);
			if (idx != npos)
				return buckets_[idx].kv.second;
			return insert(key, T{}).first->second;
		}

		constexpr T &at(const Key &key) noexcept
		{
			size_type idx = find_index(key);
			if (idx == npos)
				return dummy();
			return buckets_[idx].kv.second;
		}

		constexpr std::pair<value_type *, bool> insert(const Key &key, const T &value) noexcept
		{
			size_type hash = bluestl::hash(key) % Capacity;
			for (size_type i = 0; i < Capacity; ++i)
			{
				size_type idx = (hash + i) % Capacity;
				if (!buckets_[idx].used)
				{
					buckets_[idx].kv.first = key;	 // = value_type(key, value);
					buckets_[idx].kv.second = value; // = value_type(key, value);
					buckets_[idx].used = true;
					++size_;
					return {to_value_type(&buckets_[idx]), true};
				}
				if (buckets_[idx].used && buckets_[idx].kv.first == key)
				{
					return {to_value_type(&buckets_[idx]), false};
				}
			}
			return {nullptr, false};
		}

		constexpr bool erase(const Key &key) noexcept
		{
			size_type idx = find_index(key);
			if (idx == npos)
				return false;
			buckets_[idx].used = false;
			--size_;
			return true;
		}

		constexpr value_type *find(const Key &key) noexcept
		{
			size_type idx = find_index(key);
			if (idx == npos)
				return nullptr;
			return to_value_type(&buckets_[idx]);
		}

		constexpr const value_type *find(const Key &key) const noexcept
		{
			size_type idx = find_index(key);
			if (idx == npos)
				return nullptr;
			return to_value_type(&buckets_[idx]);
		}

		static constexpr size_type npos = static_cast<size_type>(-1);

	private:
		bucket buckets_[Capacity];
		size_type size_;

		constexpr size_type find_index(const Key &key) const noexcept
		{
			size_type hash = bluestl::hash(key) % Capacity;
			for (size_type i = 0; i < Capacity; ++i)
			{
				size_type idx = (hash + i) % Capacity;
				if (!buckets_[idx].used)
					return npos;
				if (buckets_[idx].kv.first == key)
					return idx;
			}
			return npos;
		}
		constexpr value_type *to_value_type(bucket *b) const noexcept
		{
			return reinterpret_cast<value_type *>(&b->kv);
		}
		constexpr const value_type *to_value_type(const bucket *b) const noexcept
		{
			return reinterpret_cast<const value_type *>(&b->kv);
		}
		static T &dummy()
		{
			static T d{};
			return d;
		}
	};

} // namespace bluestl
