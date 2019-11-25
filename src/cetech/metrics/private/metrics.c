#include <celib/module.h>
#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/containers/array.h>
#include <celib/api.h>
#include <celib/memory/memory.h>
#include <celib/log.h>
#include <celib/id.h>
#include <celib/containers/hash.h>

#include "cetech/metrics/metrics.h"

#define _G prifiler_global

static struct _G {
    ce_alloc_t0 *alloc;

    bool record;
    uint32_t frame_n;

    uint32_t values_n;
    ce_hash_t value_idx;
    float** values;

    float* curent_values;
} _G = {};

void add_float_metric(const char *name) {
    uint64_t id = ce_id_a0->id64(name);
    uint32_t idx = _G.values_n++;
    ce_hash_add(&_G.value_idx, id, idx, _G.alloc);
    ce_array_push(_G.values, NULL, _G.alloc);
    ce_array_push(_G.curent_values, 0, _G.alloc);
}

static void begin() {
    if (_G.record) {
    }
}

static void end() {
    if (_G.record) {
        for (int i = 0; i < _G.values_n; ++i) {
            ce_array_push(_G.values[i], _G.curent_values[i], _G.alloc);
        }

        _G.frame_n++;
    }
}

void record_float(uint64_t name,
                  float value) {

    uint64_t idx = ce_hash_lookup(&_G.value_idx, name, UINT64_MAX);
    if(idx == UINT64_MAX) {
        return;
    }

    _G.curent_values[idx] = value;
}

float get_float(uint64_t name) {
    uint64_t idx = ce_hash_lookup(&_G.value_idx, name, UINT64_MAX);
    if(idx == UINT64_MAX) {
        return 0;
    }

    return _G.curent_values[idx];
}

const float* get_recorded_floats(uint64_t name) {
    uint64_t idx = ce_hash_lookup(&_G.value_idx, name, UINT64_MAX);
    if(idx == UINT64_MAX) {
        return NULL;
    }

    return _G.values[idx];
}

bool is_record() {
    return _G.record;
}

void clear_record() {
    for (int i = 0; i < _G.values_n; ++i) {
        ce_array_clean(_G.values[i]);
    }

    _G.frame_n = 0;
}

void start_record() {
    _G.record = true;
}

void stop_record() {
    _G.record = false;
}

uint32_t recorded_frames() {
    return _G.frame_n;
}


static struct ct_metrics_a0 profiler_api = {
        .begin = begin,
        .end = end,
        .reg_float_metric = add_float_metric,

        .is_recording = is_record,
        .clear_record = clear_record,
        .start_record = start_record,
        .stop_record = stop_record,
        .recorded_frames_num = recorded_frames,

        .set_float = record_float,
        .get_float = get_float,

        .get_recorded_floats = get_recorded_floats,
};

struct ct_metrics_a0 *ct_metrics_a0 = &profiler_api;

void CE_MODULE_LOAD(metrics)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_log_a0);

    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };

    api->add_api(CT_METRICS_A0_STR, &profiler_api, sizeof(profiler_api));
}

void CE_MODULE_UNLOAD(metrics)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    clear_record();
}
