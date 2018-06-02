#include <stdio.h>

#include <cetech/engine/debugui/debugui.h>
#include <cetech/playground/playground.h>
#include <cetech/kernel/log/log.h>
#include <cetech/playground/log_view.h>
#include <cetech/engine/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/kernel/containers/array.h>
#include <cetech/kernel/ebus/ebus.h>

#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"
#include "cetech/kernel/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_ebus_a0);

#define WINDOW_NAME "Log view"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("log_view")
#define LOG_FORMAT "%s -> %s"

struct log_item {
    enum ct_log_level level;
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
    ct_alloc *allocator;
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

static ct_log_view_a0 log_view_api = {
};

static void log_handler(enum ct_log_level level,
                        time_t time,
                        char worker_id,
                        const char *where,
                        const char *msg,
                        void *data) {
    if (!_G.allocator) {
        return;
    }

    ++_G.level_counters[level];

    int offset = ct_array_size(_G.line_buffer);
    log_item item = {
            .level = level,
            .offset = offset
    };

    char buffer[1024];
    int len = snprintf(buffer, CT_ARRAY_LEN(buffer), LOG_FORMAT, where,
                       msg);

    ct_array_push(_G.log_items, item, _G.allocator);
    ct_array_push_n(_G.line_buffer, buffer, len + 1, _G.allocator);
}


static void ui_filter() {
    _G.filter.Draw();
}

static void ui_level_mask() {
    char buffer[64];
    for (int i = 0; i < CT_ARRAY_LEN(_levels); ++i) {
        int level = _levels[i];
        snprintf(buffer, CT_ARRAY_LEN(buffer),
                 _level_to_label[level], _G.level_counters[level]);

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
                      ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()),
                      false, ImGuiWindowFlags_HorizontalScrollbar);

    const int size = ct_array_size(_G.log_items);
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

static void on_debugui(uint32_t bus_name,
                       void *event) {

    if (ct_debugui_a0.BeginDock(WINDOW_NAME,
                                &_G.visible,
                                DebugUIWindowFlags_(0))) {
        ui_filter();
        ui_level_mask();
        ui_log_items();
    }

    ct_debugui_a0.EndDock();

}

static void on_menu_window(uint32_t bus_name,
                           void *event) {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true,
            .level_mask = (uint8_t) ~0,
            .allocator = ct_memory_a0.main_allocator(),
    };

    api->register_api("ct_log_view_a0", &log_view_api);

    ct_log_a0.register_handler(log_handler, NULL);


    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui, 0);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT, on_menu_window, 0);

}

static void _shutdown() {
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui);
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT, on_menu_window);

    ct_array_free(_G.log_items, _G.allocator);
    ct_array_free(_G.line_buffer, _G.allocator);
    _G = {};
}

CETECH_MODULE_DEF(
        log_view,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_ebus_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)