//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CETECH_MAP2_H
#define CETECH_MAP2_H

#include "container_types.inl"
#include "array.inl"

namespace cetech {

    /// The map function stores its data in a "list-in-an-array" where
    /// indices are used instead of pointers.
    ///
    /// When items are removed, the array-list is repacked to always keep
    /// it tightly ordered.

    namespace map {
        /// Returns true if the specified key exists in the map.
        template<typename T>
        bool has(const Map<T> &h,
                 uint64_t key);

        /// Returns the value stored for the specified key, or deffault if the key
        /// does not exist in the map.
        template<typename T>
        const T &get(const Map<T> &h,
                     uint64_t key,
                     const T &deffault);

        /// Sets the value for the key.
        template<typename T>
        void set(Map<T> &h,
                 uint64_t key,
                 const T &value);

        /// Removes the key from the map if it exists.
        template<typename T>
        void remove(Map<T> &h,
                    uint64_t key);

        /// Resizes the map lookup table to the specified size.
        /// (The table will grow automatically when 70 % full.)
        template<typename T>
        void reserve(Map<T> &h,
                     uint32_t size);

        /// Remove all elements from the map.
        template<typename T>
        void clear(Map<T> &h);

        /// Returns a pointer to the first entry in the map table, can be used to
        /// efficiently iterate over the elements (in random order).
        template<typename T>
        const typename Map<T>::Entry *begin(const Map<T> &h);

        template<typename T>
        const typename Map<T>::Entry *end(const Map<T> &h);
    }

    namespace multi_map {
        /// Finds the first entry with the specified key.
        template<typename T>
        const typename Map<T>::Entry *find_first(const Map<T> &h,
                                                 uint64_t key);

        /// Finds the next entry with the same key as e.
        template<typename T>
        const typename Map<T>::Entry *find_next(const Map<T> &h,
                                                const typename Map<T>::Entry *e);

        /// Returns the number of entries with the key.
        template<typename T>
        uint32_t count(const Map<T> &h,
                       uint64_t key);

        /// Returns all the entries with the specified key.
        /// Use a TempAllocator for the array to avoid allocating memory.
        template<typename T>
        void get(const Map<T> &h,
                 uint64_t key,
                 Array<T> &items);

        /// Inserts the value as an aditional value for the key.
        template<typename T>
        void insert(Map<T> &h,
                    uint64_t key,
                    const T &value);

        /// Removes the specified entry.
        template<typename T>
        void remove(Map<T> &h,
                    const typename Map<T>::Entry *e);

        /// Removes all entries with the specified key.
        template<typename T>
        void remove_all(Map<T> &h,
                        uint64_t key);
    }

    namespace map_internal {
        const uint32_t END_OF_LIST = 0xffffffffu;

        struct FindResult {
            uint32_t map_i;
            uint32_t data_prev;
            uint32_t data_i;
        };

        template<typename T>
        uint32_t add_entry(Map<T> &h,
                           uint64_t key) {
            typename Map<T>::Entry e;
            e.key = key;
            e.next = END_OF_LIST;
            uint32_t ei = array::size(h._data);
            array::push_back(h._data, e);
            return ei;
        }

        template<typename T>
        FindResult find(const Map<T> &h,
                        uint64_t key) {

            FindResult fr;
            fr.map_i = END_OF_LIST;
            fr.data_prev = END_OF_LIST;
            fr.data_i = END_OF_LIST;

            if (array::size(h._hash) == 0)
                return fr;

            fr.map_i = (uint32_t) (key % array::size(h._hash));
            fr.data_i = h._hash[fr.map_i];
            while (fr.data_i != END_OF_LIST) {
                if (h._data[fr.data_i].key == key)
                    return fr;
                fr.data_prev = fr.data_i;
                fr.data_i = h._data[fr.data_i].next;
            }
            return fr;
        }

        template<typename T>
        FindResult find(const Map<T> &h,
                        const typename Map<T>::Entry *e) {
            FindResult fr;
            fr.map_i = END_OF_LIST;
            fr.data_prev = END_OF_LIST;
            fr.data_i = END_OF_LIST;

            if (array::size(h._hash) == 0)
                return fr;

            fr.map_i = e->key % array::size(h._hash);
            fr.data_i = h._hash[fr.map_i];
            while (fr.data_i != END_OF_LIST) {
                if (&h._data[fr.data_i] == e)
                    return fr;
                fr.data_prev = fr.data_i;
                fr.data_i = h._data[fr.data_i].next;
            }
            return fr;
        }

        template<typename T>
        uint32_t find_or_fail(const Map<T> &h,
                              uint64_t key) {
            return find(h, key).data_i;
        }

        template<typename T>
        uint32_t find_or_make(Map<T> &h,
                              uint64_t key) {
            const FindResult fr = find(h, key);
            if (fr.data_i != END_OF_LIST)
                return fr.data_i;

            uint32_t i = add_entry(h, key);
            if (fr.data_prev == END_OF_LIST)
                h._hash[fr.map_i] = i;
            else
                h._data[fr.data_prev].next = i;
            return i;
        }

        template<typename T>
        uint32_t make(Map<T> &h,
                      uint64_t key) {
            const FindResult fr = find(h, key);
            const uint32_t i = add_entry(h, key);

            if (fr.data_prev == END_OF_LIST)
                h._hash[fr.map_i] = i;
            else
                h._data[fr.data_prev].next = i;

            h._data[i].next = fr.data_i;
            return i;
        }

        template<typename T>
        void erase(Map<T> &h,
                   const FindResult &fr) {
            if (fr.data_prev == END_OF_LIST)
                h._hash[fr.map_i] = h._data[fr.data_i].next;
            else
                h._data[fr.data_prev].next = h._data[fr.data_i].next;

            array::pop_back(h._data);

            if (fr.data_i == array::size(h._data)) {
                return;
            }

            h._data[fr.data_i] = h._data[array::size(h._data)];
            FindResult last = find(h, h._data[fr.data_i].key);

            if (last.data_prev != END_OF_LIST)
                h._data[last.data_prev].next = fr.data_i;
            else
                h._hash[last.map_i] = fr.data_i;
        }


        template<typename T>
        void find_and_erase(Map<T> &h,
                            uint64_t key) {
            const FindResult fr = find(h, key);
            if (fr.data_i != END_OF_LIST)
                erase(h, fr);
        }

        template<typename T>
        void rehash(Map<T> &h,
                    uint32_t new_size) {
            Map<T> nh(h._hash._allocator);
            array::resize(nh._hash, new_size);
            array::reserve(nh._data, array::size(h._data));
            for (uint32_t i = 0; i < new_size; ++i)
                nh._hash[i] = END_OF_LIST;
            for (uint32_t i = 0; i < array::size(h._data); ++i) {
                const typename Map<T>::Entry &e = h._data[i];
                multi_map::insert(nh, e.key, e.value);
            }

            Map<T> empty(h._hash._allocator);
            h.~Map<T>();
            memcpy(&h, &nh, sizeof(Map<T>));
            memcpy(&nh, &empty, sizeof(Map<T>));
        }

        template<typename T>
        bool full(const Map<T> &h) {
            const float max_load_factor = 0.7f;
            return array::size(h._data) >=
                   array::size(h._hash) * max_load_factor;
        }

        template<typename T>
        void grow(Map<T> &h) {
            const uint32_t new_size = array::size(h._data) * 2 + 10;
            rehash(h, new_size);
        }
    }

    namespace map {
        template<typename T>
        bool has(const Map<T> &h,
                 uint64_t key) {
            return map_internal::find_or_fail(h, key) !=
                   map_internal::END_OF_LIST;
        }

        template<typename T>
        const T &get(const Map<T> &h,
                     uint64_t key,
                     const T &deffault) {
            const uint32_t i = map_internal::find_or_fail(h, key);
            return i == map_internal::END_OF_LIST ? deffault
                                                  : h._data[i].value;
        }


        template<typename T>
        void set(Map<T> &h,
                 uint64_t key,
                 const T &value) {
            if (array::size(h._hash) == 0)
                map_internal::grow(h);

            const uint32_t i = map_internal::find_or_make(h, key);
            h._data[i].value = value;
            if (map_internal::full(h))
                map_internal::grow(h);
        }

        template<typename T>
        void remove(Map<T> &h,
                    uint64_t key) {
            map_internal::find_and_erase(h, key);
        }

        template<typename T>
        void reserve(Map<T> &h,
                     uint32_t size) {
            map_internal::rehash(h, size);
        }

        template<typename T>
        void clear(Map<T> &h) {
            array::clear(h._data);
            array::clear(h._hash);
        }

        template<typename T>
        const typename Map<T>::Entry *begin(const Map<T> &h) {
            return array::begin(h._data);
        }

        template<typename T>
        const typename Map<T>::Entry *end(const Map<T> &h) {
            return array::end(h._data);
        }
    }

    namespace multi_map {
        template<typename T>
        const typename Map<T>::Entry *find_first(const Map<T> &h,
                                                 uint64_t key) {
            const uint32_t i = map_internal::find_or_fail(h, key);
            return i == map_internal::END_OF_LIST ? 0 : &h._data[i];
        }

        template<typename T>
        const typename Map<T>::Entry *find_next(const Map<T> &h,
                                                const typename Map<T>::Entry *e) {
            uint32_t i = e->next;
            while (i != map_internal::END_OF_LIST) {
                if (h._data[i].key == e->key)
                    return &h._data[i];
                i = h._data[i].next;
            }
            return 0;
        }

        template<typename T>
        uint32_t count(const Map<T> &h,
                       uint64_t key) {
            uint32_t i = 0;
            const typename Map<T>::Entry *e = find_first(h, key);
            while (e) {
                ++i;
                e = find_next(h, e);
            }
            return i;
        }

        template<typename T>
        void get(const Map<T> &h,
                 uint64_t key,
                 Array<T> &items) {
            const typename Map<T>::Entry *e = find_first(h, key);
            while (e) {
                array::push_back(items, e->value);
                e = find_next(h, e);
            }
        }

        template<typename T>
        void insert(Map<T> &h,
                    uint64_t key,
                    const T &value) {
            if (array::size(h._hash) == 0)
                map_internal::grow(h);

            const uint32_t i = map_internal::make(h, key);
            h._data[i].value = value;
            if (map_internal::full(h))
                map_internal::grow(h);
        }

        template<typename T>
        void remove(Map<T> &h,
                    const typename Map<T>::Entry *e) {
            const map_internal::FindResult fr = map_internal::find(h, e);
            if (fr.data_i != map_internal::END_OF_LIST)
                map_internal::erase(h, fr);
        }

        template<typename T>
        void remove_all(Map<T> &h,
                        uint64_t key) {
            while (map::has(h, key))
                map::remove(h, key);
        }
    }

    template<typename T>
    Map<T>::Map() {}

//    template<typename T>
//    Map<T>::Map(const Map<T> &other) {
//        _data = other._data;
//        _hash = other._hash;
//    }
//
//    template<typename T>
//    Map<T> &Map<T>::operator=(const Map<T> &other) {
//        _data = other._data;
//        _hash = other._hash;
//
//        return *this;
//    }


    template<typename T>
    Map<T>::Map(allocator *a) :
            _hash(a), _data(a) {}

    template<typename T>
    void Map<T>::init(allocator *a) {
        _hash.init(a);
        _data.init(a);
    }

    template<typename T>
    void Map<T>::destroy() {
        _hash.destroy();
        _data.destroy();
    }

}


#endif //CETECH_MAP2_H
