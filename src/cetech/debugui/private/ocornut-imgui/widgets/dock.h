
#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <celib/containers/buffer.h>
#include <celib/ydb.h>
#include <celib/cdb.h>

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
    IMGUI_API void saveToYaml(char** buffer, ce_alloc_t0* alloc);

    IMGUI_API void loadFromYaml(const char *file,
                                struct ce_ydb_a0 *ydb,
                                struct ce_ydb_a0 *yng, struct ce_cdb_a0* cdb);

} // namespace ImGui
