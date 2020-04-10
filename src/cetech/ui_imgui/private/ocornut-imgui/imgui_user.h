#include <stdint.h>
#include <inttypes.h>

namespace ImGui
{
    struct Font
    {
        enum Enum
        {
            Regular,
            Mono,

            Count
        };
    };

    void PushFont(Font::Enum _font);

    // BK - simple string class for convenience.
    class ImString
    {
    public:
        ImString();
        ImString(const ImString& rhs);
        ImString(const char* rhs);
        ~ImString();

        ImString& operator=(const ImString& rhs);
        ImString& operator=(const char* rhs);

        void Clear();
        bool IsEmpty() const;

        const char* CStr() const
        {
            return NULL == Ptr ? "" : Ptr;
        }

    private:
        char* Ptr;
    };

} // namespace ImGui

#include "cetech/ui_imgui/private/ocornut-imgui/widgets/color_picker.h"
#include "cetech/ui_imgui/private/ocornut-imgui/widgets/color_wheel.h"
#include "cetech/ui_imgui/private/ocornut-imgui/widgets/dock.h"
#include "cetech/ui_imgui/private/ocornut-imgui/widgets/file_list.h"
#include "cetech/ui_imgui/private/ocornut-imgui/widgets/gizmo.h"
#include "cetech/ui_imgui/private/ocornut-imgui/widgets/memory_editor.h"
#include "cetech/ui_imgui/private/ocornut-imgui/widgets/range_slider.h"
