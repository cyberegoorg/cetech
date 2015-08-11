#include "collection_types.h"
#include "array.h"

namespace foundation
{
	namespace queue 
	{
		/// Returns the number of items in the queue.
		template <typename T> uint32_t size(const Queue<T> &q);
		/// Returns the ammount of free space in the queue/ring buffer.
		/// This is the number of items we can push before the queue needs to grow.
		template<typename T> uint32_t space(const Queue<T> &q);
		/// Makes sure the queue has room for at least the specified number of items.
		template<typename T> void reserve(Queue<T> &q, uint32_t size);

		/// Pushes the item to the end of the queue.
		template<typename T> void push_back(Queue<T> &q, const T &item);
		/// Pops the last item from the queue. The queue cannot be empty.
		template<typename T> void pop_back(Queue<T> &q);
		/// Pushes the item to the front of the queue.
		template<typename T> void push_front(Queue<T> &q, const T &item);
		/// Pops the first item from the queue. The queue cannot be empty.
		template<typename T> void pop_front(Queue<T> &q);

		/// Consumes n items from the front of the queue.
		template <typename T> void consume(Queue<T> &q, uint32_t n);
		/// Pushes n items to the back of the queue.
		template <typename T> void push(Queue<T> &q, const T *items, uint32_t n);

		/// Returns the begin and end of the continuous chunk of elements at
		/// the start of the queue. (Note that this chunk does not necessarily
		/// contain all the elements in the queue (if the queue wraps around
		/// the array).
		///
		/// This is useful for when you want to process many queue elements at
		/// once.
		template<typename T> T* begin_front(Queue<T> &q);
		template<typename T> const T* begin_front(const Queue<T> &q);
		template<typename T> T* end_front(Queue<T> &q);
		template<typename T> const T* end_front(const Queue<T> &q);
	}

	namespace queue_internal
	{
		// Can only be used to increase the capacity.
		template<typename T> void increase_capacity(Queue<T> &q, uint32_t new_capacity)
		{
			uint32_t end = array::size(q._data);
			array::resize(q._data, new_capacity);
			if (q._offset + q._size > end) {
				uint32_t end_items = end - q._offset;
				memmove(array::begin(q._data) + new_capacity - end_items, array::begin(q._data) + q._offset, end_items * sizeof(T));
				q._offset += new_capacity - end;
			}
		}

		template<typename T> void grow(Queue<T> &q, uint32_t min_capacity = 0)
		{
			uint32_t new_capacity = array::size(q._data)*2 + 8;
			if (new_capacity < min_capacity)
				new_capacity = min_capacity;
			increase_capacity(q, new_capacity);
		}
	}

	namespace queue 
	{
		template<typename T> inline uint32_t size(const Queue<T> &q)
		{
			return q._size;
		}

		template<typename T> inline uint32_t space(const Queue<T> &q)
		{
			return array::size(q._data) - q._size;
		}

		template<typename T> void reserve(Queue<T> &q, uint32_t size)
		{
			if (size > q._size)
				queue_internal::increase_capacity(q, size);
		}

		template<typename T> inline void push_back(Queue<T> &q, const T &item)
		{
			if (!space(q))
				queue_internal::grow(q);
			q[q._size++] = item;
		}

		template<typename T> inline void pop_back(Queue<T> &q)
		{
			--q._size;
		}
		
		template<typename T> inline void push_front(Queue<T> &q, const T &item)
		{
			if (!space(q))
				queue_internal::grow(q);
			q._offset = (q._offset - 1 + array::size(q._data)) % array::size(q._data);
			++q._size;
			q[0] = item;
		}
		
		template<typename T> inline void pop_front(Queue<T> &q)
		{
			q._offset = (q._offset + 1) % array::size(q._data);
			--q._size;
		}

		template <typename T> inline void consume(Queue<T> &q, uint32_t n)
		{
			q._offset = (q._offset + n) % array::size(q._data);
			q._size -= n;
		}

		template <typename T> void push(Queue<T> &q, const T *items, uint32_t n)
		{
			if (space(q) < n)
				queue_internal::grow(q, size(q) + n);
			const uint32_t size = array::size(q._data);
			const uint32_t insert = (q._offset + q._size) % size;
			uint32_t to_insert = n;
			if (insert + to_insert > size)
				to_insert = size - insert;
			memcpy(array::begin(q._data) + insert, items, to_insert * sizeof(T));
			q._size += to_insert;
			items += to_insert;
			n -= to_insert;
			memcpy(array::begin(q._data), items, n * sizeof(T));
			q._size += n;
		}

		template<typename T> inline T* begin_front(Queue<T> &q)
		{
			return array::begin(q._data) + q._offset;
		}
		template<typename T> inline const T* begin_front(const Queue<T> &q)
		{
			return array::begin(q._data) + q._offset;
		}
		template<typename T> T* end_front(Queue<T> &q)
		{
			uint32_t end = q._offset + q._size;
			return end > array::size(q._data) ? array::end(q._data) : array::begin(q._data) + end;
		}
		template<typename T> const T* end_front(const Queue<T> &q)
		{
			uint32_t end = q._offset + q._size;
			return end > array::size(q._data) ? array::end(q._data) : array::begin(q._data) + end;
		}
	}

	template <typename T> inline Queue<T>::Queue(Allocator &allocator) : _data(allocator), _size(0), _offset(0) {}

	template <typename T> inline T & Queue<T>::operator[](uint32_t i)
	{
		return _data[(i + _offset) % array::size(_data)];
	}

	template <typename T> inline const T & Queue<T>::operator[](uint32_t i) const
	{
		return _data[(i + _offset) % array::size(_data)];
	}
}