#include <celib/buffer.inl>
#include <cetech/modules/yaml/yaml.h>

namespace ImGui
{
	///
	IMGUI_API void InitDockContext();

	///
	IMGUI_API void ShutdownDockContext();

	///
	IMGUI_API void RootDock(const ImVec2& pos, const ImVec2& size);

	///
	IMGUI_API bool BeginDock(const char* label, bool* opened = NULL, ImGuiWindowFlags extra_flags = 0);

	///
	IMGUI_API void EndDock();

	///
	IMGUI_API void SetDockActive();

	///
	IMGUI_API void saveToYaml(celib::Buffer& buffer);

	IMGUI_API void loadFromYaml(yaml_node_t root);

} // namespace ImGui
