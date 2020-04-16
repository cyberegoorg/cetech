#ifndef CETECH_UI_H
#define CETECH_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define CT_UI_A0_STR "ct_ui_a0"

#define CT_UI_A0_API \
    CE_ID64_0("ct_ui_a0", 0x9f3a0dd77cc0deecULL)

#define CT_UI_TASK \
    CE_ID64_0("ui_task", 0x2705ab68fabee4abULL)


enum ct_ui_hovered_flag_e0 {
    CT_UI_HOVERED_FLAGS_DEFAULT = 0,
    CT_UI_HOVERED_FLAGS_ChildWindows = 1 << 0,
    CT_UI_HOVERED_FLAGS_RootWindow = 1 << 1,
    CT_UI_HOVERED_FLAGS_AnyWindow = 1 << 2,
    CT_UI_HOVERED_FLAGS_AllowWhenBlockedByPopup = 1 << 3,
    CT_UI_HOVERED_FLAGS_AllowWhenBlockedByActiveItem = 1 < 5,
    CT_UI_HOVERED_FLAGS_AllowWhenOverlapped = 1 << 6,
    CT_UI_HOVERED_FLAGS_RectOnly = CT_UI_HOVERED_FLAGS_AllowWhenBlockedByPopup
                                   | CT_UI_HOVERED_FLAGS_AllowWhenBlockedByActiveItem
                                   | CT_UI_HOVERED_FLAGS_AllowWhenOverlapped,
    CT_UI_HOVERED_FLAGS_RootAndChildWindows = CT_UI_HOVERED_FLAGS_RootWindow
                                              | CT_UI_HOVERED_FLAGS_ChildWindows
};

enum ct_ui_drag_drop_0 {
    CT_UI_DROP_FLAGS_SourceNoPreviewTooltip = 1 << 0,
    CT_UI_DROP_FLAGS_SourceNoDisableHover = 1 << 1,
    CT_UI_DROP_FLAGS_SourceNoHoldToOpenOthers = 1 << 2,
    CT_UI_DROP_FLAGS_SourceAllowNullID = 1 << 3,
    CT_UI_DROP_FLAGS_SourceExtern = 1 << 4,
    CT_UI_DROP_FLAGS_AcceptBeforeDelivery = 1 << 10,
    CT_UI_DROP_FLAGS_AcceptNoDrawDefaultRect = 1 << 11,
    CT_UI_DROP_FLAGS_AcceptPeekOnly = CT_UI_DROP_FLAGS_AcceptBeforeDelivery |
                                      CT_UI_DROP_FLAGS_AcceptNoDrawDefaultRect
};

enum ct_ui_cond_e0 {
    CT_UI_COND_Always = 1 << 0,
    CT_UI_COND_Once = 1 << 1,
    CT_UI_COND_FirstUseEver = 1 << 2,
    CT_UI_COND_Appearing = 1 << 3
};

enum ct_ui_window_flags_e0 {
    CT_UI_WINDOW_FLAGS_NoTitleBar = 1 << 0,
    CT_UI_WINDOW_FLAGS_NoResize = 1 << 1,
    CT_UI_WINDOW_FLAGS_NoMove = 1 << 2,
    CT_UI_WINDOW_FLAGS_NoScrollbar = 1 << 3,
    CT_UI_WINDOW_FLAGS_NoScrollWithMouse = 1 << 4,
    CT_UI_WINDOW_FLAGS_NoCollapse = 1 << 5,
    CT_UI_WINDOW_FLAGS_AlwaysAutoResize = 1 << 6,
    //CT_UI_WINDOW_FLAGS_ShowBorders          = 1 << 7,
    CT_UI_WINDOW_FLAGS_NoSavedSettings = 1 << 8,
    CT_UI_WINDOW_FLAGS_NoInputs = 1 << 9,
    CT_UI_WINDOW_FLAGS_MenuBar = 1 << 10,
    CT_UI_WINDOW_FLAGS_HorizontalScrollbar = 1 << 11,
    CT_UI_WINDOW_FLAGS_NoFocusOnAppearing = 1 << 12,
    CT_UI_WINDOW_FLAGS_NoBringToFrontOnFocus = 1 << 13,
    CT_UI_WINDOW_FLAGS_AlwaysVerticalScrollbar = 1 << 14,
    CT_UI_WINDOW_FLAGS_AlwaysHorizontalScrollbar = 1 << 15,
    CT_UI_WINDOW_FLAGS_AlwaysUseWindowPadding = 1 << 16,
    CT_UI_WINDOW_FLAGS_ResizeFromAnySide = 1 << 17,
    CT_UI_WINDOW_FLAGS_NoNavInputs = 1 << 18,
    CT_UI_WINDOW_FLAGS_NoNavFocus = 1 << 19,
    CT_UI_WINDOW_FLAGS_NoNav = CT_UI_WINDOW_FLAGS_NoNavInputs | CT_UI_WINDOW_FLAGS_NoNavFocus,
};

enum ct_ui_item_flags_e0 {
    CT_UI_ITEM_FLAGS_AllowKeyboardFocus = 1 << 0,

    CT_UI_ITEM_FLAGS_ButtonRepeat = 1 << 1,

    CT_UI_ITEM_FLAGS_Disabled = 1 << 2,

    CT_UI_ITEM_FLAGS_NoNav = 1 << 3,

    CT_UI_ITEM_FLAGS_NoNavDefaultFocus = 1 << 4,

    CT_UI_ITEM_FLAGS_SelectableDontClosePopup = 1 << 5,

    CT_UI_ITEM_FLAGS_Default_ = CT_UI_ITEM_FLAGS_AllowKeyboardFocus
};


enum ct_ui_style_var_e0 {
    CT_UI_STYLE_VAR_Alpha,               // float
    CT_UI_STYLE_VAR_WindowPadding,       // vec2
    CT_UI_STYLE_VAR_WindowRounding,      // float
    CT_UI_STYLE_VAR_WindowBorderSize,    // float
    CT_UI_STYLE_VAR_WindowMinSize,       // vec2
    CT_UI_STYLE_VAR_WindowTitleAlign,    // vec2
    CT_UI_STYLE_VAR_ChildRounding,       // float
    CT_UI_STYLE_VAR_ChildBorderSize,     // float
    CT_UI_STYLE_VAR_PopupRounding,       // float
    CT_UI_STYLE_VAR_PopupBorderSize,     // float
    CT_UI_STYLE_VAR_FramePadding,        // vec2
    CT_UI_STYLE_VAR_FrameRounding,       // float
    CT_UI_STYLE_VAR_FrameBorderSize,     // float
    CT_UI_STYLE_VAR_ItemSpacing,         // vec2
    CT_UI_STYLE_VAR_ItemInnerSpacing,    // vec2
    CT_UI_STYLE_VAR_IndentSpacing,       // float
    CT_UI_STYLE_VAR_ScrollbarSize,       // float
    CT_UI_STYLE_VAR_ScrollbarRounding,   // float
    CT_UI_STYLE_VAR_GrabMinSize,         // float
    CT_UI_STYLE_VAR_GrabRounding,        // float
    CT_UI_STYLE_VAR_ButtonTextAlign,     // vec2
    CT_UI_STYLE_VAR_ViewId,              // uint8_t
    CT_UI_STYLE_VAR_COUNT
};

enum ct_ui_tree_node_flag {
    CT_TREE_NODE_FLAGS_Selected = 1 << 0,
    CT_TREE_NODE_FLAGS_Framed = 1 << 1,
    CT_TREE_NODE_FLAGS_AllowOverlapMode = 1 << 2,
    CT_TREE_NODE_FLAGS_NoTreePushOnOpen = 1 << 3,
    CT_TREE_NODE_FLAGS_NoAutoOpenOnLog = 1 << 4,
    CT_TREE_NODE_FLAGS_DefaultOpen = 1 << 5,
    CT_TREE_NODE_FLAGS_OpenOnDoubleClick = 1 << 6,
    CT_TREE_NODE_FLAGS_OpenOnArrow = 1 << 7,
    CT_TREE_NODE_FLAGS_Leaf = 1 << 8,
    CT_TREE_NODE_FLAGS_Bullet = 1 << 9,
    CT_TREE_NODE_FLAGS_CollapsingHeader =
    CT_TREE_NODE_FLAGS_Framed | CT_TREE_NODE_FLAGS_NoAutoOpenOnLog
};

typedef struct ct_ui_button_t0 {
    const char *text;
    ce_vec2_t size;
} ct_ui_button_t0;


typedef struct ct_ui_modal_popup_t0 {
    uint64_t id;
    const char *text;
} ct_ui_modal_popup_t0;

typedef struct ct_ui_popup_t0 {
    uint64_t id;
} ct_ui_popup_t0;

typedef struct ct_ui_input_text_t0 {
    uint64_t id;
} ct_ui_input_text_t0;

typedef struct ct_ui_combo_t0 {
    uint64_t id;
    const char *label;
    const char *const *items;
    uint32_t items_count;
} ct_ui_combo_t0;

typedef struct ct_ui_combo2_t0 {
    uint64_t id;
} ct_ui_combo2_t0;

typedef struct ct_ui_child_t0 {
    uint64_t id;
    ce_vec2_t size;
    bool border;
} ct_ui_child_t0;

typedef struct ct_ui_selectable_t0 {
    uint64_t id;
    const char *text;
    ce_vec2_t size;
    bool selected;
    bool dont_close_popups;
    bool allow_double_click;
} ct_ui_selectable_t0;

typedef struct ct_ui_checkbox_t0 {
    const char *text;
} ct_ui_checkbox_t0;

typedef struct ct_ui_menu_t0 {
    const char *text;
    bool disabled;
} ct_ui_menu_t0;


typedef struct ct_ui_menu_item_t0 {
    const char *text;
    const char *shortcut;
    bool selected;
    bool disabled;
} ct_ui_menu_item_t0;

typedef struct ct_ui_drop_source_t0 {
    const char *type;
    const void *data;
    size_t size;
    enum ct_ui_cond_e0 cond;
} ct_ui_drop_payload_t0;

typedef struct ct_ui_image_t0 {
    uint32_t user_texture_id;
    ce_vec2_t size;
    ce_vec4_t tint_col;
    ce_vec4_t border_col;
} ct_ui_image_t0;

typedef struct ct_ui_dock_t0 {
    const char *label;
    enum ct_ui_window_flags_e0 flags;
} ct_ui_dock_t0;

typedef struct ct_ui_drag_float_t0 {
    uint64_t id;
    float v_speed;
    float v_min;
    float v_max;
    const char *display_format;
    float power;
} ct_ui_drag_float_t0;

typedef struct ct_ui_drag_int_t0 {
    uint64_t id;
    float v_speed;
    int32_t v_min;
    int32_t v_max;
    const char *display_format;
} ct_ui_drag_int_t0;

typedef struct ct_ui_tree_node_ex_t0 {
    uint64_t id;
    const char *text;
    enum ct_ui_tree_node_flag flags;
} ct_ui_tree_node_ex_t0;

typedef struct ct_ui_collapsing_header_t0 {
    uint64_t id;
    const char *text;
    enum ct_ui_tree_node_flag flags;
} ct_ui_collapsing_header_t0;

typedef struct ct_ui_color_edit_t0 {
    uint64_t id;
} ct_ui_color_edit_t0;

typedef struct ct_ui_radio_buttton_t0 {
    uint64_t id;
    const char *text;
    int32_t value;
} ct_ui_radio_buttton_t0;

typedef struct ct_ui_plot_lines_t0 {
    uint64_t id;
    const float *values;
    int32_t values_count;
    int32_t values_offset;
    const char *overlay_text;
    float scale_min;
    float scale_max;
    ce_vec2_t graph_size;
    int32_t stride;
} ct_ui_plot_lines_t0;

struct ce_vio_t0;

//! UI API V0
struct ct_ui_a0 {
    void (*render)();

    uint64_t (*generate_id)();

    bool (*button)(const ct_ui_button_t0 *button);

    void (*text)(const char *text);

    void (*text_unformated)(const char *text,
                            const char *text_end);

    bool (*input_text)(const ct_ui_input_text_t0 *input,
                       char *buf,
                       size_t buf_size);

    bool (*combo)(const ct_ui_combo_t0 *combo,
                  int32_t *current_item);

    bool (*combo2)(const ct_ui_combo2_t0 *combo,
                   int32_t *current_item,
                   const char *items_separated_by_zeros);

    bool (*selectable)(const ct_ui_selectable_t0 *selectable);

    bool (*checkbox)(const ct_ui_checkbox_t0 *checkbox,
                     bool *v);

    bool (*drag_float)(const ct_ui_drag_float_t0 *drag_float,
                       float *v);

    bool (*drag_float2)(const ct_ui_drag_float_t0 *drag_float,
                        ce_vec2_t *v);

    bool (*drag_float3)(const ct_ui_drag_float_t0 *drag_float,
                        ce_vec3_t *v);

    bool (*drag_float4)(const ct_ui_drag_float_t0 *drag_float,
                        ce_vec4_t *v);

    bool (*drag_int)(const ct_ui_drag_int_t0 *drag_int,
                     int32_t *v);

    bool (*color_edit)(const ct_ui_color_edit_t0 *color,
                       ce_vec4_t *c);

    bool (*radio_button2)(const ct_ui_radio_buttton_t0 *btn,
                          int32_t *v);


    void (*plot_lines)(const ct_ui_plot_lines_t0 *plot);

    //
    bool (*child_begin)(const ct_ui_child_t0 *child);

    void (*child_end)();

    //
    void (*push_item_width)(float item_width);

    void (*pop_item_width)();

    //
    void (*tooltip_begin)();

    void (*tooltip_end)();

    //
    void (*same_line)(float pos_x,
                      float spacing_w);

    //
    bool (*begin_main_menu_bar)();

    void (*end_main_menu_bar)();

    bool (*begin_menu_bar)();

    void (*end_menu_bar)();


    bool (*menu_begin)(const ct_ui_menu_t0 *menu);

    void (*menu_end)();


    bool (*menu_item)(const ct_ui_menu_item_t0 *item);

    bool (*menu_item_checkbox)(const ct_ui_menu_item_t0 *item,
                               bool *selected);


    //
    bool (*popup_begin)(const ct_ui_popup_t0 *popup);

    void (*popup_end)();

    void (*popup_open)(uint64_t modal_id);

    void (*popup_close_current)();


    //
    bool (*modal_popup_begin)(const ct_ui_modal_popup_t0 *modal,
                              bool *p_open);

    void (*modal_popup_end)();

    void (*modal_popup_open)(uint64_t modal_id);

    //
    ce_vec2_t (*get_window_size)();

    void (*set_next_window_size)(ce_vec2_t size);

    //
    //

    void (*columns)(uint64_t id,
                    uint32_t count,
                    bool border);

    void (*columns_end)();

    void (*next_column)();

    void (*indent)();
    void (*unindent)();


    //
    bool (*is_item_hovered)(enum ct_ui_hovered_flag_e0 flags);

    //
    bool (*is_mouse_double_clicked)(uint32_t btn);

    bool (*is_mouse_clicked)(uint32_t btn,
                             bool repeat);

    bool (*is_mouse_hovering_window)();

    bool (*is_item_clicked)(uint32_t btn);

    //
    bool (*drag_drop_source_begin)(enum ct_ui_drag_drop_0 flags);

    void (*drag_drop_source_end)();

    bool (*set_drag_drop_payload)(ct_ui_drop_payload_t0 *payload);

    const void *(*accept_drag_drop_payload)(const char *type);

    bool (*drag_drop_target_begin)();

    void (*drag_drop_target_end)();

    //
    void (*image)(const ct_ui_image_t0 *image);

    //
    ce_vec2_t (*get_content_region_avail)();

    //
    void (*dock_root)(ce_vec2_t pos,
                      ce_vec2_t size);

    bool (*dock_begin)(const ct_ui_dock_t0 *dock,
                       bool *opened);

    void (*dock_end)();

    void (*dock_save)(struct ce_vio_t0 *output);

    void (*dock_load)(const char *path);

    //
    void (*separator)();

    //
    void (*tree_pop)();

    //
    void (*push_item_flag)(enum ct_ui_item_flags_e0 flags,
                           bool enabled);

    void (*push_style_var)(enum ct_ui_style_var_e0,
                           float val);

    void (*pop_item_flag)();

    void (*pop_style_var)(uint32_t count);

    void (*pop_style_color)(uint32_t count);

    //
    bool (*tree_node_ex)(const ct_ui_tree_node_ex_t0 *node);

    bool (*collapsing_header)(const ct_ui_collapsing_header_t0 *header);

    //
    void (*push_id)(uint64_t id);

    void (*pop_id)();

};


CE_MODULE(ct_ui_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_UI_H
