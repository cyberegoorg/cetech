#include <stdio.h>

#include <cetech/debugui/debugui.h>
#include <cetech/editor/editor.h>
#include <celib/log.h>
#include <cetech/editor/log_view.h>
#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <celib/array.inl>
#include <celib/ebus.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>

#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"
#include <cetech/editor/dock.h>

#define WINDOW_NAME "Log view"
#define LOG_FORMAT "[%d|%s] -> %s"
#define LOG_ARGS worker_id, where, msg

struct log_item {
    enum ce_log_level level;
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
    ce_alloc *allocator;
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


static void log_handler(enum ce_log_level level,
                        time_t time,
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

    ce_array_push(_G.log_items, item, _G.allocator);
    ce_array_push_n(_G.line_buffer, buffer, len + 1, _G.allocator);
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

static void on_debugui(ct_dock_i0 *dock) {
    ui_filter();
    ui_level_mask();
    ui_log_items();
}

static const char *dock_title(struct ct_dock_i0 *dock) {
    return WINDOW_NAME;
}

static const char *name(struct ct_dock_i0 *dock) {
    return "log_view";
}

static struct ct_dock_i0 ct_dock_i0 = {
        .id = 0,
        .visible = true,
        .display_title = dock_title,
        .draw_ui = on_debugui,
        .name = name,
};

static void _init(struct ce_api_a0 *api) {
    _G = {
            .visible = true,
            .level_mask = (uint8_t) ~0,
            .allocator = ce_memory_a0->system,
    };


    ce_log_a0->register_handler(log_handler, NULL);

    ce_api_a0->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);
}

static void _shutdown() {
    ce_array_free(_G.log_items, _G.allocator);
    ce_array_free(_G.line_buffer, _G.allocator);
    _G = {};
}

CE_MODULE_DEF(
        log_view,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ce_log_a0);
            CE_INIT_API(api, ce_ebus_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)