#pragma once

#include "common/container/queue.inl.h"
#include "common/container/array.inl.h"

namespace cetech {
    namespace handler {
        template <
        typename handler_type,
        typename free_idx_type,
        typename gen_type,
        int index_bit_count,
        int gen_bit_count,
        int min_free_indexs
        >
        struct HandlerManager {
            typedef handler_type HandlerType;
            typedef free_idx_type FreeIdxType;
            typedef gen_type GentTyp;

            enum {
                IndexBitCount = index_bit_count,
                GenBitCount = gen_bit_count,
                MinFreeIndex = min_free_indexs
            };

            static handler_type idx(const handler_type handler) {
                return handler >> index_bit_count;
            }

            static handler_type gen(const handler_type handler) {
                return handler & ((1 << gen_bit_count) - 1);
            }

            static handler_type make_handler(const handler_type idx, const handler_type gen) {
                return ((idx) << (index_bit_count)) | (gen);
            }

            static handler_type create(Queue < free_idx_type >& free_idx, Array < gen_type >& generation) {
                uint32_t idx;

                if (queue::size(free_idx) > min_free_indexs) {
                    idx = free_idx[0];
                    queue::pop_front(free_idx);
                } else {
                    array::push_back < gen_type > (generation, 0);
                    idx = array::size(generation) - 1;
                }

                return make_handler(idx, generation[idx]);
            }

            static void destroy(handler_type handler, Queue < free_idx_type >& free_idx,
                                Array < gen_type >& generation) {
                const handler_type id = idx(handler);

                ++generation[id];
                queue::push_back(free_idx, id);
            }

            static bool alive(handler_type handler, const Array < gen_type >& generation) {
                return generation[idx(handler)] == gen(handler);
            }
        };
    }
}