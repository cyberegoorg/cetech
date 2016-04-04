/*******************************************************************************
**** Based on bitsquid foundation.
*******************************************************************************/

#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/container/array.inl.h"
#include "celib/container/types.h"

namespace cetech {

    /***************************************************************************
    **** Hash map
    ***************************************************************************/
    namespace hash {

        /***********************************************************************
        **** Exist key in hash?
        ***********************************************************************/
        template < typename T > bool has(const Hash < T >& h,
                                         const uint64_t key);

        /***********************************************************************
        **** Get item associate with key or return deffault value
        ***********************************************************************/
        template < typename T > const T& get(const Hash < T >& h,
                                             const uint64_t key,
                                             const T& deffault);


        /***********************************************************************
        **** Map key to value
        ***********************************************************************/
        template < typename T > void set(Hash < T >& h,
                                         const uint64_t key,
                                         const T& value);

        /***********************************************************************
        **** Remove item form hash map
        ***********************************************************************/
        template < typename T > void remove(Hash < T >& h,
                                            const uint64_t key);


        /***********************************************************************
        **** Resize hash lookup table to the specified size.
        **** Table will grow automatically when 70% full.
        ***********************************************************************/
        template < typename T > void reserve(Hash < T >& h,
                                             const uint32_t size);

        /***********************************************************************
        **** Remove all items
        ***********************************************************************/
        template < typename T > void clear(Hash < T >& h);


        /***********************************************************************
        **** Return pointer to first item
        ***********************************************************************/
        template < typename T > const typename Hash < T > ::Entry * begin(const Hash < T > &h);

        /***********************************************************************
        **** Return pointer to last item
        ***********************************************************************/
        template < typename T > const typename Hash < T > ::Entry * end(const Hash < T > &h);
    }


    /***************************************************************************
    **** Hash multi map
    ***************************************************************************/
    namespace multi_hash {

        /***********************************************************************
        **** Find first entry with specified key.
        ***********************************************************************/
        template < typename T > const typename Hash < T > ::Entry * find_first(const Hash < T > &h, const uint64_t key);


        /***********************************************************************
        **** Find next entry with same key as e.
        ***********************************************************************/
        template < typename T > const typename Hash < T > ::Entry * find_next(const Hash < T > &h,
                                                                              const typename Hash < T > ::Entry * e);


        /***********************************************************************
        **** Returns the number of entries with the key.
        ***********************************************************************/
        template < typename T > uint32_t count(const Hash < T >& h,
                                               const uint64_t key);

        /***********************************************************************
        **** Returns all the entries with the specified key.
        ***********************************************************************/
        template < typename T > void get(const Hash < T >& h,
                                         const uint64_t key,
                                         Array < T >& items);

        /***********************************************************************
        **** Inserts the value as an aditional value for the key.
        ***********************************************************************/
        template < typename T > void insert(Hash < T >& h,
                                            const uint64_t key,
                                            const T& value);

        /***********************************************************************
        **** Removes the specified entry.
        ***********************************************************************/
        template < typename T > void remove(Hash < T >& h,
                                            const typename Hash < T > ::Entry* e);

        /***********************************************************************
        **** Removes all entrise with the specified key.
        ***********************************************************************/
        template < typename T > void remove_all(Hash < T >& h,
                                                const uint64_t key);
    }

    /***************************************************************************
    **** Hash internals
    ***************************************************************************/
    namespace hash_internal {
        static const uint32_t END_OF_LIST = 0xffffffffu; //!< End of list marker.

        struct FindResult {
            uint32_t hash_i;    //!< Hash index.
            uint32_t data_prev; //!< Previous data index.
            uint32_t data_i;    //!< Data index.
        };

        template < typename T > uint32_t add_entry(Hash < T >& h,
                                                   const uint64_t key) {
            typename Hash < T > ::Entry e;
            e.key = key;
            e.next = END_OF_LIST;

            const uint32_t ei = array::size(h._data);
            array::push_back(h._data, e);

            return ei;
        }

        template < typename T > FindResult find(const Hash < T >& h,
                                                const uint64_t key) {
            FindResult fr;
            fr.data_i = END_OF_LIST;
            fr.data_prev = END_OF_LIST;
            fr.hash_i = END_OF_LIST;

            if (array::size(h._hash) == 0) {
                return fr;
            }

            fr.hash_i = key % array::size(h._hash);
            fr.data_i = h._hash[fr.hash_i];
            while (fr.data_i != END_OF_LIST) {
                if (h._data[fr.data_i].key == key) {
                    return fr;
                }

                fr.data_prev = fr.data_i;
                fr.data_i = h._data[fr.data_i].next;
            }

            return fr;
        }

        template < typename T > FindResult find(const Hash < T >& h,
                                                const typename Hash < T > ::Entry* e) {
            FindResult fr = {END_OF_LIST, END_OF_LIST, END_OF_LIST};

            if (array::size(h._hash) == 0) {
                return fr;
            }

            fr.hash_i = e->key % array::size(h._hash);
            fr.data_i = h._hash[fr.hash_i];
            while (fr.data_i != END_OF_LIST) {
                if (&h._data[fr.data_i] == e) {
                    return fr;
                }

                fr.data_prev = fr.data_i;
                fr.data_i = h._data[fr.data_i].next;
            }

            return fr;
        }

        template < typename T > uint32_t find_or_fail(const Hash < T >& h,
                                                      const uint64_t key) {
            return find(h, key).data_i;
        }

        template < typename T > uint32_t find_or_make(Hash < T >& h,
                                                      const uint64_t key) {
            const FindResult fr = find(h, key);
            if (fr.data_i != END_OF_LIST) {
                return fr.data_i;
            }

            const uint32_t i = add_entry(h, key);
            if (fr.data_prev == END_OF_LIST) {
                h._hash[fr.hash_i] = i;
            } else {
                h._data[fr.data_prev].next = i;
            }

            return i;
        }

        template < typename T > void erase(Hash < T >& h,
                                           const FindResult& fr) {
            if (fr.data_prev == END_OF_LIST) {
                h._hash[fr.hash_i] = h._data[fr.data_i].next;
            } else {
                h._data[fr.data_prev].next = h._data[fr.data_i].next;
            }

            if (fr.data_i == array::size(h._data) - 1) {
                array::pop_back(h._data);
                return;
            }

            h._data[fr.data_i] = h._data[array::size(h._data) - 1];
            FindResult last = find(h, h._data[fr.data_i].key);

            if (last.data_prev != END_OF_LIST) {
                h._data[last.data_prev].next = fr.data_i;
            } else {
                h._hash[last.hash_i] = fr.data_i;
            }
        }

        template < typename T > uint32_t make(Hash < T >& h,
                                              const uint64_t key) {
            const FindResult fr = find(h, key);
            const uint32_t i = add_entry(h, key);

            if (fr.data_prev == END_OF_LIST) {
                h._hash[fr.hash_i] = i;
            } else {
                h._data[fr.data_prev].next = i;
            }

            h._data[i].next = fr.data_i;
            return i;
        }

        template < typename T > void find_and_erase(Hash < T >& h,
                                                    const uint64_t key) {
            const FindResult fr = find(h, key);
            if (fr.data_i != END_OF_LIST) {
                erase(h, fr);
            }
        }

        template < typename T > void rehash(Hash < T >& h,
                                            const uint32_t new_size) {
            Hash < T > nh(*h._hash._allocator);
            array::resize(nh._hash, new_size);
            array::reserve(nh._data, array::size(h._data));
            for (uint32_t i = 0; i < new_size; ++i) {
                nh._hash[i] = END_OF_LIST;
            }

            for (uint32_t i = 0; i < array::size(h._data); ++i) {
                const typename Hash < T > ::Entry& e = h._data[i];
                multi_hash::insert(nh, e.key, e.value);
            }

            Hash < T > empty(*h._hash._allocator);
            h.~Hash < T > ();
            memory::memcpy(&h, &nh, sizeof(Hash < T >));
            memory::memcpy(&nh, &empty, sizeof(Hash < T >));
        }

        template < typename T > bool full(const Hash < T >& h) {
            const float max_load_factor = 0.7f;

            return array::size(h._data) >= array::size(h._hash) * max_load_factor;
        }

        template < typename T > void grow(Hash < T >& h) {
            const uint32_t new_size = array::size(h._data) * 2 + 10;

            rehash(h, new_size);
        }
    }

    /***************************************************************************
    **** Hash implementation
    ***************************************************************************/
    namespace hash {
        template < typename T > bool has(const Hash < T >& h,
                                         const uint64_t key) {
            return hash_internal::find_or_fail(h, key) != hash_internal::END_OF_LIST;
        }

        template < typename T > const T& get(const Hash < T >& h,
                                             const uint64_t key,
                                             const T& deffault) {
            const uint32_t i = hash_internal::find_or_fail(h, key);
            return i == hash_internal::END_OF_LIST ? deffault : h._data[i].value;
        }

        template < typename T > void set(Hash < T >& h,
                                         const uint64_t key,
                                         const T& value) {
            if (array::size(h._hash) == 0) {
                hash_internal::grow(h);
            }

            const uint32_t i = hash_internal::find_or_make(h, key);
            h._data[i].value = value;

            if (hash_internal::full(h)) {
                hash_internal::grow(h);
            }
        }

        template < typename T > void remove(Hash < T >& h,
                                            const uint64_t key) {
            hash_internal::find_and_erase(h, key);
        }

        template < typename T > void reserve(Hash < T >& h,
                                             const uint32_t size) {
            hash_internal::rehash(h, size);
        }

        template < typename T > void clear(Hash < T >& h) {
            array::clear(h._data);
            array::clear(h._hash);
        }

        template < typename T > const typename Hash < T > ::Entry * begin(const Hash < T > &h)
        {
            return array::begin(h._data);
        }

        template < typename T > const typename Hash < T > ::Entry * end(const Hash < T > &h)
        {
            return array::end(h._data);
        }
    }

    /***************************************************************************
    **** Hash multi map
    ***************************************************************************/
    namespace multi_hash {
        template < typename T > const typename Hash < T > ::Entry * find_first(const Hash < T > &h, const uint64_t key)
        {
            const uint32_t i = hash_internal::find_or_fail(h, key);

            return i == hash_internal::END_OF_LIST ? 0 : &h._data[i];
        }

        template < typename T > const typename Hash < T > ::Entry * find_next(const Hash < T > &h,
                                                                              const typename Hash < T > ::Entry * e)
        {
            uint32_t i = e->next;
            while (i != hash_internal::END_OF_LIST) {
                if (h._data[i].key == e->key) {
                    return &h._data[i];
                }

                i = h._data[i].next;
            }

            return 0;
        }

        template < typename T > uint32_t count(const Hash < T >& h,
                                               const uint64_t key) {
            uint32_t i = 0;
            const typename Hash < T > ::Entry* e = find_first(h, key);
            while (e) {
                ++i;
                e = find_next(h, e);
            }

            return i;
        }

        template < typename T > void get(const Hash < T >& h,
                                         const uint64_t key,
                                         Array < T >& items) {
            const typename Hash < T > ::Entry* e = find_first(h, key);
            while (e) {
                array::push_back(items, e->value);
                e = find_next(h, e);
            }
        }

        template < typename T > void insert(Hash < T >& h,
                                            const uint64_t key,
                                            const T& value) {
            if (array::size(h._hash) == 0) {
                hash_internal::grow(h);
            }

            const uint32_t i = hash_internal::make(h, key);
            h._data[i].value = value;

            if (hash_internal::full(h)) {
                hash_internal::grow(h);
            }
        }

        template < typename T > void remove(Hash < T >& h,
                                            const typename Hash < T > ::Entry* e) {
            const hash_internal::FindResult fr = hash_internal::find(h, e);

            if (fr.data_i != hash_internal::END_OF_LIST) {
                hash_internal::erase(h, fr);
            }
        }

        template < typename T > void remove_all(Hash < T >& h,
                                                const uint64_t key) {
            while (hash::has(h, key)) {
                hash::remove(h, key);
            }
        }
    }


    /***************************************************************************
    **** Hash implementation
    ***************************************************************************/
    template < typename T > Hash < T > ::Hash(Allocator & a) : _hash(a), _data(a) {}

}
