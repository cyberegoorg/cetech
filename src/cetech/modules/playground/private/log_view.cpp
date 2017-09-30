#include <stdio.h>
#include "celib/map.inl"
#include "celib/fpumath.h"

#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/playground/playground.h>
#include <cetech/kernel/log.h>
#include <cetech/modules/playground/log_view.h>
#include <cetech/modules/debugui/private/ocornut-imgui/imgui.h>

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_log_a0);

using namespace celib;

#define WINDOW_NAME "Log view"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("log_view")
#define LOG_FORMAT "%s -> %s"

struct log_item {
    enum ct_log_level level;
    int offset;
};

#define _G log_view_global
static struct _G {
    Array<log_item> log_items;
    Array<char> line_buffer;
    ImGuiTextFilter filter;

    int level_counters[5];
    uint8_t level_mask;

    bool visible;
} _G;

static int _levels[] = {LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DBG};

static ImVec4 _level_to_color[][4] = {
        [LOG_INFO]    = {{0.0f, 0.0f, 1.0f, 1.0f},
                         {0.0f, 0.0f, 0.5f, 1.0f}},
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

static ct_log_view_a0 log_view_api = {
};

static void log_handler(enum ct_log_level level,
                        time_t time,
                        char worker_id,
                        const char *where,
                        const char *msg,
                        void *data) {
    if (!_G.log_items._allocator) {
        return;
    }

    ++_G.level_counters[level];

    int offset = array::size(_G.line_buffer);
    log_item item = {
            .level = level,
            .offset = offset
    };

    char buffer[1024];
    int len = snprintf(buffer, CETECH_ARRAY_LEN(buffer), LOG_FORMAT, where,
                       msg);

    array::push_back(_G.log_items, item);
    array::push(_G.line_buffer, buffer, len + 1);
}


static void ui_filter() {
    _G.filter.Draw();
}

static void ui_level_mask() {
    char buffer[64];
    for (int i = 0; i < CETECH_ARRAY_LEN(_levels); ++i) {
        int level = _levels[i];
        snprintf(buffer, CETECH_ARRAY_LEN(buffer),
                 _level_to_label[level],_G.level_counters[level]);

        bool active = (_G.level_mask & (1 << level)) > 0;

        ImGui::PushStyleColor(ImGuiCol_CheckMark, _level_to_color[level][0]);
        ImGui::PushStyleColor(ImGuiCol_Text, _level_to_color[level][0]);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, _level_to_color[level][1]);


        if (i > 0) {
            ct_debugui_a0.SameLine(0, -1);
        }

        if (ct_debugui_a0.RadioButton(buffer, active)) {
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
                      ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()),
                      false, ImGuiWindowFlags_HorizontalScrollbar);

    const int size = array::size(_G.log_items);
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
        ct_debugui_a0.TextUnformatted(line, line + len);
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

static void on_debugui() {
    if (ct_debugui_a0.BeginDock(WINDOW_NAME,
                                &_G.visible,
                                DebugUIWindowFlags_(0))) {
        ui_filter();
        ui_level_mask();
        ui_log_items();
    }

    ct_debugui_a0.EndDock();

}

static void on_menu_window() {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true,
            .level_mask = (uint8_t) ~0,
    };

    api->register_api("ct_log_view_a0", &log_view_api);
    _G.log_items.init(ct_memory_a0.main_allocator());
    _G.line_buffer.init(ct_memory_a0.main_allocator());

    ct_log_a0.register_handler(log_handler, NULL);
    ct_playground_a0.register_module(
            PLAYGROUND_MODULE_NAME,
            (ct_playground_module_fce) {
                    .on_ui = on_debugui,
                    .on_menu_window = on_menu_window,
            });
}

static void _shutdown() {
    ct_playground_a0.unregister_module(PLAYGROUND_MODULE_NAME);
    _G.log_items.destroy();
    _G.line_buffer.destroy();
    _G = {};
}

CETECH_MODULE_DEF(
        log_view,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_log_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)