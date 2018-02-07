
#include <cetech/core/containers/buffer.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/yaml/yng.h>

namespace ImGui {
    ///
    IMGUI_API void InitDockContext();

    ///
    IMGUI_API void ShutdownDockContext();

    ///
    IMGUI_API void RootDock(const ImVec2 &pos,
                            const ImVec2 &size);

    ///
    IMGUI_API bool BeginDock(const char *label,
                             bool *opened = NULL,
                             ImGuiWindowFlags extra_flags = 0);

    ///
    IMGUI_API void EndDock();

    ///
    IMGUI_API void SetDockActive();

    ///
    IMGUI_API void saveToYaml(char** buffer, ct_alloc* alloc);

    IMGUI_API void loadFromYaml(const char *file,
                                ct_ydb_a0 *ydb,
                                ct_yng_a0 *yng);

} // namespace ImGui
