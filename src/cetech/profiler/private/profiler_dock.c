#define CE_DYNAMIC_MODULE 1

#include <celib/module.h>
#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/containers/array.h>
#include <celib/api.h>
#include <celib/memory/memory.h>
#include <celib/log.h>
#include <celib/id.h>
#include <cetech/editor/dock.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <float.h>

#include "cetech/metrics/metrics.h"

#define _G prifiler_dock_global

#define CT_PROFILER_DOCK  \
    CE_ID64_0("profiler_dock", 0x626ec368079c6053ULL)

static struct _G {
    ce_alloc_t0 *alloc;
} _G = {};

static const char *dock_title() {
    return "Profiler";
}

static const char *name(uint64_t dock) {
    return "profiler_dock";
}

static void draw_menu(uint64_t content,
                      uint64_t context,
                      uint64_t selected_object) {
    bool is_recording = ct_metrics_a0->is_recording();

    if (ct_debugui_a0->Checkbox("Record", &is_recording)) {
        if (is_recording) ct_metrics_a0->start_record();
        else ct_metrics_a0->stop_record();
    }

    ct_debugui_a0->SameLine(0, 0);
    if (ct_debugui_a0->Button("Clear", &CE_VEC2_ZERO)) {
        ct_metrics_a0->clear_record();
    }
}

static void draw_dock(uint64_t content,
                      uint64_t context,
                      uint64_t selected_object) {

    float w = ct_debugui_a0->GetContentRegionAvail().x;

    ce_vec2_t plot_size = {w, 20};
    float dt = ct_metrics_a0->get_float(ce_id_a0->id64("dt"));
    ct_debugui_a0->Text("dt: %f", dt);

    uint32_t frames_n = ct_metrics_a0->recorded_frames_num();
    const float *float_buffer;

    if (ct_debugui_a0->TreeNodeEx("CPU", DebugUITreeNodeFlags_DefaultOpen)) {
        float_buffer = ct_metrics_a0->get_recorded_floats(ce_id_a0->id64("dt"));
        ct_debugui_a0->PlotLines("", float_buffer, frames_n,
                                 0, "dt", FLT_MAX, FLT_MAX, &plot_size, sizeof(float));

        float_buffer = ct_metrics_a0->get_recorded_floats(ce_id_a0->id64("memory.system"));
        ct_debugui_a0->PlotLines("", float_buffer, frames_n,
                                 0, "memory", FLT_MAX, FLT_MAX, &plot_size, sizeof(float));

        ct_debugui_a0->TreePop();
    }

    if (ct_debugui_a0->TreeNodeEx("Renderer", DebugUITreeNodeFlags_DefaultOpen)) {
        float_buffer = ct_metrics_a0->get_recorded_floats(ce_id_a0->id64("renderer.wait_submit"));
        ct_debugui_a0->PlotLines("", float_buffer, frames_n,
                                 0, "wait_submit", FLT_MAX, FLT_MAX, &plot_size, sizeof(float));

        float_buffer = ct_metrics_a0->get_recorded_floats(ce_id_a0->id64("renderer.wait_render"));
        ct_debugui_a0->PlotLines("", float_buffer, frames_n,
                                 0, "wait_render", FLT_MAX, FLT_MAX, &plot_size, sizeof(float));

        float_buffer = ct_metrics_a0->get_recorded_floats(ce_id_a0->id64("renderer.num_draw"));
        ct_debugui_a0->PlotLines("", float_buffer, frames_n,
                                 0, "num_draw", FLT_MAX, FLT_MAX, &plot_size, sizeof(float));

        ct_debugui_a0->TreePop();
    }
}

static struct ct_dock_i0 profile_dock = {
        .type = CT_PROFILER_DOCK,
        .display_title = dock_title,
        .name = name,
        .draw_ui = draw_dock,
        .draw_menu = draw_menu,
};

void CE_MODULE_LOAD(profiler)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ct_dock_a0);
    CE_INIT_API(api, ct_metrics_a0);
    CE_INIT_API(api, ct_debugui_a0);

    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };

    api->add_impl(CT_DOCK_I, &profile_dock, sizeof(profile_dock));

    if (!reload) {
        ct_dock_a0->create_dock(CT_PROFILER_DOCK, true);
    }
}

void CE_MODULE_UNLOAD(profiler)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    api->remove_impl(CT_DOCK_I, &profile_dock);
}
