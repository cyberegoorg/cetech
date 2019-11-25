#include <stdio.h>


#include <celib/memory/allocator.h>
#include <cetech/editor/editor.h>
#include <celib/log.h>
#include <cetech/log_view/log_view.h>
#include <celib/containers/array.h>
#include <cetech/debugui/icons_font_awesome.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"
#include <cetech/editor/dock.h>

#include <cetech/debugui/debugui.h>

#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <celib/os/thread.h>

#define WINDOW_NAME "Log view"
#define LOG_FORMAT "[%d|%s] -> %s"
#define LOG_ARGS worker_id, where, msg

struct log_item {
    enum ce_log_level_e0 level;
    int offset;
};

#define _G log_view_global
static struct _G {
    log_item *log_items;
    char *line_buffer;
    ImGuiTextFilter filter;

    int level_counters[5];
    uint8_t level_mask;

    bool visible;
    ce_alloc_t0 *allocator;

    ce_spinlock_t0 lock;
} _G;

static int _levels[] = {LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DBG};

static ImVec4 _level_to_color[][4] = {
        [LOG_INFO]    = {{1.0f, 1.0f, 1.0f, 1.0f},
                         {0.5f, 0.5f, 0.5f, 1.0f}},
        [LOG_WARNING] = {{1.0f, 1.0f, 0.0f, 1.0f},
                         {0.5f, 0.5f, 0.0f, 1.0f}},
        [LOG_ERROR]   = {{1.0f, 0.0f, 0.0f, 1.0f},
                         {0.5f, 0.0f, 0.0f, 1.0f}},
        [LOG_DBG]     = {{0.0f, 1.0f, 0.0f, 1.0f},
                         {0.0f, 0.5f, 0.0f, 1.0f}},
};

static const char *_level_to_label[4] = {
        [LOG_INFO]    = "I (%d)",
        [LOG_WARNING] = "W (%d)",
        [LOG_ERROR]   = "E (%d)",
        [LOG_DBG]     = "D (%d)",
};


static void log_handler(enum ce_log_level_e0 level,
                        ce_time_t time,
                        char worker_id,
                        const char *where,
                        const char *msg,
                        void *data) {
    if (!_G.allocator) {
        return;
    }

    ++_G.level_counters[level];

    int offset = ce_array_size(_G.line_buffer);
    log_item item = {
            .level = level,
            .offset = offset
    };

    char buffer[1024];
    int len = snprintf(buffer, CE_ARRAY_LEN(buffer), LOG_FORMAT, LOG_ARGS);

    ce_os_thread_a0->spin_lock(&_G.lock);
    ce_array_push(_G.log_items, item, _G.allocator);
    ce_array_push_n(_G.line_buffer, buffer, len + 1, _G.allocator);
    ce_os_thread_a0->spin_unlock(&_G.lock);
}


static void ui_filter() {
    _G.filter.Draw(ICON_FA_SEARCH);
}

static void ui_level_mask() {
    char buffer[64];
    for (int i = 0; i < CE_ARRAY_LEN(_levels); ++i) {
        int level = _levels[i];
        snprintf(buffer, CE_ARRAY_LEN(buffer),
                 _level_to_label[level], _G.level_counters[level]);

        bool active = (_G.level_mask & (1 << level)) > 0;

        ImGui::PushStyleColor(ImGuiCol_CheckMark, _level_to_color[level][0]);
        ImGui::PushStyleColor(ImGuiCol_Text, _level_to_color[level][0]);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, _level_to_color[level][1]);


        if (i > 0) {
            ct_debugui_a0->SameLine(0, -1);
        }

        if (ct_debugui_a0->RadioButton(buffer, active)) {
            if (!active) {
                _G.level_mask |= (1 << level);
            } else {
                _G.level_mask &= ~(1 << level);
            }
        }
        ImGui::PopStyleColor(3);
    }
}

static void ui_log_items() {
    ImGui::BeginChild("log_view_scrolling_region",
                      ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()),
                      false, ImGuiWindowFlags_HorizontalScrollbar);

    const int size = ce_array_size(_G.log_items);
    for (int i = size - 1; i >= 0; --i) {
        log_item *item = &_G.log_items[i];
        const char *line = &_G.line_buffer[item->offset];

        if (!_G.filter.PassFilter(line)) {
            continue;
        }

        if (!(_G.level_mask & (1 << item->level))) {
            continue;
        }

        int len = strlen(line);

        ImGui::PushStyleColor(ImGuiCol_Text, _level_to_color[item->level][0]);
        ct_debugui_a0->TextUnformatted(line, line + len);
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

static void on_debugui(uint64_t content,
                       uint64_t context,
                       uint64_t selected_object) {
    ui_filter();
    ui_level_mask();
    ui_log_items();
}

static const char *dock_title(uint64_t content,
                              uint64_t selected_object) {
    return WINDOW_NAME;
}

static const char *name() {
    return "log_view";
}

static struct ct_dock_i0 dock_api = {
        .type = LOG_VIEW,
        .display_title = dock_title,
        .draw_ui = on_debugui,
        .name = name,
};

extern "C" {

void CE_MODULE_LOAD(log_view)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ce_log_a0);

    _G = {
            .visible = true,
            .level_mask = (uint8_t) ~0,
            .allocator = ce_memory_a0->system,
    };


    ce_log_a0->register_handler(log_handler, NULL);

    ce_api_a0->add_impl(CT_DOCK_I0_STR, &dock_api, sizeof(dock_api));

    ct_dock_a0->create_dock(LOG_VIEW, true);
}

void CE_MODULE_UNLOAD(log_view)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    ce_array_free(_G.log_items, _G.allocator);
    ce_array_free(_G.line_buffer, _G.allocator);
    _G = {};
}
}