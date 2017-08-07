#include <cetech/modules/debugui/private/bgfx_imgui/imgui.h>

#define  _to_imvec2(a) (*((ImVec2*) (a)))
#define  _to_imvec4(a) (*((ImVec4*) (a)))

namespace imgui_wrap {
    void TextV(const char *fmt,
               va_list args) {
        ImGui::TextV(fmt, args);
    }

    void Text(const char *fmt,
              ...) {
        va_list args;
        va_start(args, fmt);
        TextV(fmt, args);
        va_end(args);
    }

    void TextColoredV(const _vec4 col,
                      const char *fmt,
                      va_list args) {
        ImGui::TextColoredV(_to_imvec4(col), fmt, args);
    }

    void TextColored(const _vec4 col,
                     const char *fmt,
                     ...) {
        va_list args;
        va_start(args, fmt);
        TextColoredV(col, fmt, args);
        va_end(args);
    }

    void TextDisabledV(const char *fmt,
                       va_list args) {
        ImGui::TextDisabledV(fmt, args);
    }

    void TextDisabled(const char *fmt,
                      ...) {
        va_list args;
        va_start(args, fmt);
        TextDisabledV(fmt, args);
        va_end(args);
    }


    void TextWrappedV(const char *fmt,
                      va_list args) {
        ImGui::TextWrappedV(fmt, args);
    }

    void TextWrapped(const char *fmt,
                     ...) {
        va_list args;
        va_start(args, fmt);
        TextWrappedV(fmt, args);
        va_end(args);
    }

    void TextUnformatted(const char *text,
                         const char *text_end) {
        ImGui::TextUnformatted(text, text_end);
    }

    void LabelTextV(const char *label,
                    const char *fmt,
                    va_list args) {
        ImGui::LabelTextV(label, fmt, args);
    }

    void LabelText(const char *label,
                   const char *fmt,
                   ...) {
        va_list args;
        va_start(args, fmt);
        LabelTextV(label, fmt, args);
        va_end(args);
    }

    void Bullet() {
        ImGui::Bullet();
    }

    void BulletTextV(const char *fmt,
                     va_list args) {
        ImGui::BulletTextV(fmt, args);
    }

    void BulletText(const char *fmt,
                    ...) {
        va_list args;
        va_start(args, fmt);
        BulletTextV(fmt, args);
        va_end(args);
    }


    bool Button(const char *label,
                const _vec2 size) {
        return ImGui::Button(label, _to_imvec2(size));
    }

    bool SmallButton(const char *label) {
        return ImGui::SmallButton(label);
    }

    bool InvisibleButton(const char *str_id,
                         const _vec2 size) {
        return ImGui::InvisibleButton(str_id, _to_imvec2(size));
    }

    void Image(ImTextureID user_texture_id,
               const _vec2 size,
               const _vec2 uv0,
               const _vec2 uv1,
               const _vec4 tint_col,
               const _vec4 border_col) {
        return ImGui::Image(user_texture_id,
                            _to_imvec2(size),
                            _to_imvec2(uv0),
                            _to_imvec2(uv1),
                            _to_imvec4(tint_col),
                            _to_imvec4(border_col));
    }

    bool ImageButton(ImTextureID user_texture_id,
                     const _vec2 size,
                     const _vec2 uv0,
                     const _vec2 uv1,
                     int frame_padding,
                     const _vec4 bg_col,
                     const _vec4 tint_col) {
        return ImGui::ImageButton(user_texture_id,
                                  _to_imvec2(size),
                                  _to_imvec2(uv0),
                                  _to_imvec2(uv1),
                                  frame_padding,
                                  _to_imvec4(bg_col),
                                  _to_imvec4(tint_col));
    }

    bool Checkbox(const char *label,
                  bool *v) {
        return ImGui::Checkbox(label, v);
    }

    bool CheckboxFlags(const char *label,
                       unsigned int *flags,
                       unsigned int flags_value) {
        return ImGui::CheckboxFlags(
                label,
                flags,
                flags_value);
    }

    bool RadioButton(const char *label,
                     bool active) {
        return ImGui::RadioButton(
                label,
                active);
    }

    bool RadioButton2(const char *label,
                      int *v,
                      int v_button) {
        return ImGui::RadioButton(
                label,
                v,
                v_button);
    }

    bool Combo(const char *label,
               int *current_item,
               const char *const *items,
               int items_count,
               int height_in_items) {
        return ImGui::Combo(
                label,
                current_item,
                items,
                items_count,
                height_in_items);
    }

    bool Combo2(const char *label,
                int *current_item,
                const char *items_separated_by_zeros,
                int height_in_items) {
        return ImGui::Combo(
                label,
                current_item,
                items_separated_by_zeros,
                height_in_items);
    }

    bool Combo3(const char *label,
                int *current_item,
                bool (*items_getter)(void *data,
                                     int idx,
                                     const char **out_text),
                void *data,
                int items_count,
                int height_in_items) {
        return ImGui::Combo(
                label,
                current_item,
                items_getter,
                data,
                items_count,
                height_in_items);
    }

    bool ColorButton(const _vec4 col,
                     bool small_height,
                     bool outline_border) {
        return ImGui::ColorButton(
                _to_imvec4(col),
                           small_height,
                           outline_border);
    }

    bool ColorEdit3(const char *label,
                    float col[3]) {
        return ImGui::ColorEdit3(
                label,
                col);
    }

    bool ColorEdit4(const char *label,
                    float col[4],
                    bool show_alpha) {
        return ImGui::ColorEdit4(
                label,
                col,
                show_alpha);
    }

    void ColorEditMode(ImGuiColorEditMode mode) {
        ImGui::ColorEditMode(mode);
    }

    void PlotLines(const char *label,
                   const float *values,
                   int values_count,
                   int values_offset,
                   const char *overlay_text,
                   float scale_min,
                   float scale_max,
                   _vec2 graph_size,
                   int stride) {
        ImGui::PlotLines(
                label,
                values,
                values_count,
                values_offset,
                overlay_text,
                scale_min,
                scale_max,
                (ImVec2 &) graph_size,
                stride);
    }

    void PlotLines2(const char *label,
                    float (*values_getter)(void *data,
                                           int idx),
                    void *data,
                    int values_count,
                    int values_offset,
                    const char *overlay_text,
                    float scale_min,
                    float scale_max,
                    _vec2 graph_size) {
        return ImGui::PlotLines(
                label,
                values_getter, data,
                values_count,
                values_offset,
                overlay_text,
                scale_min,
                scale_max,
                (ImVec2 &) graph_size);
    }

    void PlotHistogram(const char *label,
                       const float *values,
                       int values_count,
                       int values_offset,
                       const char *overlay_text,
                       float scale_min,
                       float scale_max,
                       _vec2 graph_size,
                       int stride) {
        return ImGui::PlotHistogram(
                label,
                values,
                values_count,
                values_offset,
                overlay_text,
                scale_min,
                scale_max,
                (ImVec2 &) graph_size,
                stride);
    }

    void PlotHistogram2(const char *label,
                        float (*values_getter)(void *data,
                                               int idx),
                        void *data,
                        int values_count,
                        int values_offset,
                        const char *overlay_text,
                        float scale_min,
                        float scale_max,
                        _vec2 graph_size) {
        return ImGui::PlotHistogram(
                label,
                values_getter,
                data,
                values_count,
                values_offset,
                overlay_text,
                scale_min,
                scale_max,
                (ImVec2 &) graph_size);
    }

    void ProgressBar(float fraction,
                     const _vec2 size_arg,
                     const char *overlay) {
        return ImGui::ProgressBar(
                fraction,
                (ImVec2 &) size_arg,
                overlay);
    }

    bool DragFloat(const char *label,
                   float *v,
                   float v_speed,
                   float v_min,
                   float v_max,
                   const char *display_format,
                   float power) {
        return ImGui::DragFloat(
                label,
                v,
                v_speed,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool DragFloat2(const char *label,
                    float v[2],
                    float v_speed,
                    float v_min,
                    float v_max,
                    const char *display_format,
                    float power) {
        return ImGui::DragFloat(
                label,
                v,
                v_speed,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool DragFloat3(const char *label,
                    float v[3],
                    float v_speed,
                    float v_min,
                    float v_max,
                    const char *display_format,
                    float power) {
        return ImGui::DragFloat(
                label,
                v,
                v_speed,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool DragFloat4(const char *label,
                    float v[4],
                    float v_speed,
                    float v_min,
                    float v_max,
                    const char *display_format,
                    float power) {
        return ImGui::DragFloat(
                label,
                v,
                v_speed,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool DragFloatRange2(const char *label,
                         float *v_current_min,
                         float *v_current_max,
                         float v_speed,
                         float v_min,
                         float v_max,
                         const char *display_format,
                         const char *display_format_max,
                         float power) {
        return ImGui::DragFloatRange2(
                label,
                v_current_min,
                v_current_max,
                v_speed,
                v_min,
                v_max,
                display_format,
                display_format_max,
                power);
    }

    bool DragInt(const char *label,
                 int *v,
                 float v_speed,
                 int v_min,
                 int v_max,
                 const char *display_format) {
        return ImGui::DragInt(
                label,
                v,
                v_speed,
                v_min,
                v_max,
                display_format);
    }

    bool DragInt2(const char *label,
                  int v[2],
                  float v_speed,
                  int v_min,
                  int v_max,
                  const char *display_format) {
        return ImGui::DragInt(
                label,
                v,
                v_speed,
                v_min,
                v_max,
                display_format);
    }

    bool DragInt3(const char *label,
                  int v[3],
                  float v_speed,
                  int v_min,
                  int v_max,
                  const char *display_format) {
        return ImGui::DragInt(
                label,
                v,
                v_speed,
                v_min,
                v_max,
                display_format);
    }

    bool DragInt4(const char *label,
                  int v[4],
                  float v_speed,
                  int v_min,
                  int v_max,
                  const char *display_format) {
        return ImGui::DragInt(
                label,
                v,
                v_speed,
                v_min,
                v_max,
                display_format);
    }

    bool DragIntRange2(const char *label,
                       int *v_current_min,
                       int *v_current_max,
                       float v_speed,
                       int v_min,
                       int v_max,
                       const char *display_format,
                       const char *display_format_max) {
        return ImGui::DragIntRange2(
                label,
                v_current_min,
                v_current_max,
                v_speed,
                v_min,
                v_max,
                display_format,
                display_format_max);
    }

    bool InputText(const char *label,
                   char *buf,
                   size_t buf_size,
                   ImGuiInputTextFlags flags,
                   ImGuiTextEditCallback callback,
                   void *user_data) {

        return ImGui::InputText(
                label,
                buf,
                buf_size, flags, callback, user_data);
    }

    bool InputTextMultiline(const char *label,
                            char *buf,
                            size_t buf_size,
                            const _vec2 size,
                            ImGuiInputTextFlags flags,
                            ImGuiTextEditCallback callback,
                            void *user_data) {
        return ImGui::InputTextMultiline(
                label,
                buf,
                buf_size,
                (ImVec2 &) size,
                flags, callback, user_data);
    }

    bool InputFloat(const char *label,
                    float *v,
                    float step,
                    float step_fast,
                    int decimal_precision,
                    ImGuiInputTextFlags extra_flags) {
        return ImGui::InputFloat(
                label,
                v,
                step,
                step_fast,
                decimal_precision, extra_flags);
    }

    bool InputFloat2(const char *label,
                     float v[2],
                     int decimal_precision,
                     ImGuiInputTextFlags extra_flags) {
        return ImGui::InputFloat(
                label,
                v,
                decimal_precision, extra_flags);
    }

    bool InputFloat3(const char *label,
                     float v[3],
                     int decimal_precision,
                     ImGuiInputTextFlags extra_flags) {
        return ImGui::InputFloat(
                label,
                v,
                decimal_precision, extra_flags);
    }

    bool InputFloat4(const char *label,
                     float v[4],
                     int decimal_precision,
                     ImGuiInputTextFlags extra_flags) {
        return ImGui::InputFloat(
                label,
                v,
                decimal_precision, extra_flags);
    }

    bool InputInt(const char *label,
                  int *v,
                  int step,
                  int step_fast00,
                  ImGuiInputTextFlags extra_flags) {
        return ImGui::InputInt(
                label,
                v,
                step,
                step_fast00, extra_flags);
    }

    bool InputInt2(const char *label,
                   int v[2],
                   ImGuiInputTextFlags extra_flags) {
        return ImGui::InputInt(
                label,
                v, extra_flags);
    }

    bool InputInt3(const char *label,
                   int v[3],
                   ImGuiInputTextFlags extra_flags) {
        return ImGui::InputInt3(
                label,
                v, extra_flags);
    }

    bool InputInt4(const char *label,
                   int v[4],
                   ImGuiInputTextFlags extra_flags) {
        return ImGui::InputInt4(
                label, v, extra_flags);
    }

    bool SliderFloat(const char *label,
                     float *v,
                     float v_min,
                     float v_max,
                     const char *display_format,
                     float power) {
        return ImGui::SliderFloat(
                label,
                v,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool SliderFloat2(const char *label,
                      float v[2],
                      float v_min,
                      float v_max,
                      const char *display_format,
                      float power) {
        return ImGui::SliderFloat(
                label,
                v,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool SliderFloat3(const char *label,
                      float v[3],
                      float v_min,
                      float v_max,
                      const char *display_format,
                      float power) {
        return ImGui::SliderFloat(
                label,
                v,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool SliderFloat4(const char *label,
                      float v[4],
                      float v_min,
                      float v_max,
                      const char *display_format,
                      float power) {
        return ImGui::SliderFloat4(
                label,
                v,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool SliderAngle(const char *label,
                     float *v_rad,
                     float v_degrees_min,
                     float v_degrees_max) {
        return ImGui::SliderAngle(
                label,
                v_rad,
                v_degrees_min,
                v_degrees_max);
    }

    bool SliderInt(const char *label,
                   int *v,
                   int v_min,
                   int v_max,
                   const char *display_format) {
        return ImGui::SliderInt(
                label,
                v,
                v_min,
                v_max,
                display_format);
    }

    bool SliderInt2(const char *label,
                    int v[2],
                    int v_min,
                    int v_max,
                    const char *display_format) {
        return ImGui::SliderInt2(
                label,
                v,
                v_min,
                v_max,
                display_format);
    }

    bool SliderInt3(const char *label,
                    int v[3],
                    int v_min,
                    int v_max,
                    const char *display_format) {
        return ImGui::SliderInt3(
                label,
                v,
                v_min,
                v_max,
                display_format);
    }

    bool SliderInt4(const char *label,
                    int v[4],
                    int v_min,
                    int v_max,
                    const char *display_format) {
        return ImGui::SliderInt4(
                label,
                v,
                v_min,
                v_max,
                display_format);
    }

    bool VSliderFloat(const char *label,
                      const _vec2 size,
                      float *v,
                      float v_min,
                      float v_max,
                      const char *display_format,
                      float power) {
        return ImGui::VSliderFloat(
                label,
                (ImVec2 &) size,
                v,
                v_min,
                v_max,
                display_format,
                power);
    }

    bool VSliderInt(const char *label,
                    const _vec2 size,
                    int *v,
                    int v_min,
                    int v_max,
                    const char *display_format) {
        return ImGui::VSliderInt(
                label,
                (ImVec2 &) size,
                v,
                v_min,
                v_max,
                display_format);
    }

    bool TreeNode(const char *label) {
        return ImGui::TreeNode(label);
    }

    bool TreeNodeV(const char *str_id,
                   const char *fmt,
                   va_list args) {
        return ImGui::TreeNodeV(
                str_id,
                fmt,
                args);
    }

    bool TreeNodeV2(const void *ptr_id,
                    const char *fmt,
                    va_list args) {
        return ImGui::TreeNodeV(
                ptr_id,
                fmt, args);
    }

    bool TreeNode2(const char *str_id,
                   const char *fmt,
                   ...) {
        va_list args;
        va_start(args, fmt);
        bool ret = TreeNodeV(str_id, fmt, args);
        va_end(args);

        return ret;
    }

    bool TreeNode3(const void *ptr_id,
                   const char *fmt,
                   ...) {
        va_list args;
        va_start(args, fmt);
        bool ret = TreeNodeV2(ptr_id, fmt, args);
        va_end(args);

        return ret;
    }


    bool TreeNodeEx(const char *label,
                    ImGuiTreeNodeFlags flags) {
        return ImGui::TreeNodeEx(label, flags);
    }

    bool TreeNodeEx2(const char *str_id,
                     ImGuiTreeNodeFlags flags,
                     const char *fmt,
                     ...) {
        va_list args;
        va_start(args, fmt);
        bool ret = ImGui::TreeNodeExV(str_id, flags, fmt, args);
        va_end(args);

        return ret;
    }

    bool TreeNodeEx3(const void *ptr_id,
                     ImGuiTreeNodeFlags flags,
                     const char *fmt,
                     ...) {
        va_list args;
        va_start(args, fmt);
        bool ret = ImGui::TreeNodeExV(ptr_id, flags, fmt, args);
        va_end(args);

        return ret;
    }

    bool TreeNodeExV(const char *str_id,
                     ImGuiTreeNodeFlags flags,
                     const char *fmt,
                     va_list args) {
        return ImGui::TreeNodeExV(str_id, flags, fmt, args);
    }

    bool TreeNodeExV2(const void *ptr_id,
                      ImGuiTreeNodeFlags flags,
                      const char *fmt,
                      va_list args) {
        return ImGui::TreeNodeExV(ptr_id, flags, fmt, args);
    }

    void TreePush(const char *str_id) {
        return ImGui::TreePush(str_id);
    }

    void TreePush2(const void *ptr_id) {
        return ImGui::TreePush(ptr_id);
    }

    void TreePop() {
        return ImGui::TreePop();
    }

    void TreeAdvanceToLabelPos() {
        return ImGui::TreeAdvanceToLabelPos();
    }

    float GetTreeNodeToLabelSpacing() {
        return ImGui::GetTreeNodeToLabelSpacing();
    }

    void SetNextTreeNodeOpen(bool is_open,
                             ImGuiSetCond cond) {
        return ImGui::SetNextTreeNodeOpen(is_open, cond);
    }

    bool CollapsingHeader(const char *label,
                          ImGuiTreeNodeFlags flags) {
        return ImGui::CollapsingHeader(label, flags);
    }

    bool CollapsingHeader2(const char *label,
                           bool *p_open,
                           ImGuiTreeNodeFlags flags) {
        return ImGui::CollapsingHeader(label, p_open, flags);
    }

    bool Selectable(const char *label,
                    bool selected,
                    ImGuiSelectableFlags flags,
                    const _vec2 size) {
        return ImGui::Selectable(
                label,
                selected, flags, (ImVec2 &) size);
    }

    bool Selectable2(const char *label,
                     bool *p_selected,
                     ImGuiSelectableFlags flags,
                     const _vec2 size) {
        return ImGui::Selectable(
                label,
                p_selected, flags,
                (ImVec2 &) size);
    }

    bool ListBox(const char *label,
                 int *current_item,
                 const char *const *items,
                 int items_count,
                 int height_in_items) {
        return ImGui::ListBox(
                label,
                current_item,
                items,
                items_count,
                height_in_items);
    }

    bool ListBox2(const char *label,
                  int *current_item,
                  bool (*items_getter)(void *data,
                                       int idx,
                                       const char **out_text),
                  void *data,
                  int items_count,
                  int height_in_items) {
        return ImGui::ListBox(
                label,
                current_item,
                items_getter, data,
                items_count,
                height_in_items);
    }

    bool ListBoxHeader(const char *label,
                       const _vec2 size) {
        return ImGui::ListBoxHeader(
                label,
                (ImVec2 &) size);
    }

    bool ListBoxHeader2(const char *label,
                        int items_count,
                        int height_in_items) {
        return ImGui::ListBoxHeader(
                label,
                items_count,
                height_in_items);
    }

    void ListBoxFooter() {
        return ImGui::ListBoxFooter();
    }

    void Value(const char *prefix,
               bool b) {
        return ImGui::Value(
                prefix,
                b);
    }

    void Value2(const char *prefix,
                int v) {
        return ImGui::Value(
                prefix,
                v);
    }

    void Value3(const char *prefix,
                unsigned int v) {
        return ImGui::Value(
                prefix,
                v);
    }

    void Value4(const char *prefix,
                float v,
                const char *float_format) {
        return ImGui::Value(
                prefix,
                v,
                float_format);
    }

    void ValueColor(const char *prefix,
                    const _vec4 v) {
        return ImGui::ValueColor(
                prefix,
                (ImVec4 &) v);
    }

    void ValueColor2(const char *prefix,
                     ImU32 v) {
        return ImGui::ValueColor(prefix, v);
    }

    void SetTooltip(const char *fmt,
                    ...) {

    }

    void SetTooltipV(const char *fmt,
                     va_list args) {
        return ImGui::SetTooltipV(fmt, args);
    }

    void BeginTooltip() {
        return ImGui::BeginTooltip();
    }

    void EndTooltip() {
        return ImGui::EndTooltip();
    }

    bool BeginMainMenuBar() {
        return ImGui::BeginMainMenuBar();
    }

    void EndMainMenuBar() {
        return ImGui::EndMainMenuBar();
    }

    bool BeginMenuBar() {
        return ImGui::BeginMenuBar();
    }

    void EndMenuBar() {
        return ImGui::EndMenuBar();
    }

    bool BeginMenu(const char *label,
                   bool enabled) {
        return ImGui::BeginMenu(label, enabled);
    }

    void EndMenu() {
        return ImGui::EndMenu();
    }

    bool MenuItem(const char *label,
                  const char *shortcut,
                  bool selected,
                  bool enabled) {
        return ImGui::MenuItem(
                label,
                shortcut,
                selected,
                enabled);
    }

    bool MenuItem2(const char *label,
                   const char *shortcut,
                   bool *p_selected,
                   bool enabled) {
        return ImGui::MenuItem(
                label,
                shortcut,
                p_selected,
                enabled);
    }

    void OpenPopup(const char *str_id) {
        return ImGui::OpenPopup(str_id);
    }

    bool BeginPopup(const char *str_id) {
        return ImGui::BeginPopup(str_id);
    }

    bool BeginPopupModal(const char *name,
                         bool *p_open,
                         ImGuiWindowFlags extra_flags) {
        return ImGui::BeginPopupModal(name, p_open, extra_flags);
    }

    bool BeginPopupContextItem(const char *str_id,
                               int mouse_button) {
        return ImGui::BeginPopupContextItem(str_id, mouse_button);
    }

    bool BeginPopupContextWindow(bool also_over_items,
                                 const char *str_id,
                                 int mouse_button) {
        return ImGui::BeginPopupContextWindow(also_over_items, str_id,
                                              mouse_button);
    }

    bool BeginPopupContextVoid(const char *str_id,
                               int mouse_button) {
        return ImGui::BeginPopupContextVoid(str_id, mouse_button);
    }

    void EndPopup() {
        return ImGui::EndPopup();
    }

    void CloseCurrentPopup() {
        return ImGui::CloseCurrentPopup();
    }
}
