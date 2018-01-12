namespace ImGui
{
    ImString::ImString()
        : Ptr(NULL)
    {
    }

    ImString::ImString(const ImString& rhs)
        : Ptr(NULL)
    {
        if (NULL != rhs.Ptr
        &&  0 != strcmp(rhs.Ptr, ""))
        {
            Ptr = ImStrdup(rhs.Ptr);
        }
    }

    ImString::ImString(const char* rhs)
        : Ptr(NULL)
    {
        if (NULL != rhs
        &&  0 != strcmp(rhs, ""))
        {
            Ptr = ImStrdup(rhs);
        }
    }

    ImString::~ImString()
    {
        Clear();
    }

    ImString& ImString::operator=(const ImString& rhs)
    {
        if (this != &rhs)
        {
            *this = rhs.Ptr;
        }

        return *this;
    }

    ImString& ImString::operator=(const char* rhs)
    {
        if (Ptr != rhs)
        {
            Clear();

            if (NULL != rhs
            &&  0 != strcmp(rhs, ""))
            {
                Ptr = ImStrdup(rhs);
            }
        }

        return *this;
    }

    void ImString::Clear()
    {
        if (NULL != Ptr)
        {
            MemFree(Ptr);
            Ptr = NULL;
        }
    }

    bool ImString::IsEmpty() const
    {
        return NULL == Ptr;
    }

    void HSplitter(const char* str_id, ImVec2* size)
    {
        ImVec2 screen_pos = GetCursorScreenPos();
        InvisibleButton(str_id, ImVec2(-1, 3));
        ImVec2 end_pos = screen_pos + GetItemRectSize();
        ImGuiWindow* win = GetCurrentWindow();
        ImVec4* colors = GetStyle().Colors;
        ImU32 color = GetColorU32(IsItemActive() || IsItemHovered() ? colors[ImGuiCol_ButtonActive] : colors[ImGuiCol_Button]);
        win->DrawList->AddRectFilled(screen_pos, end_pos, color);
        if (ImGui::IsItemActive())
        {
            size->y = ImMax(1.0f, ImGui::GetIO().MouseDelta.y + size->y);
        }
    }


    void VSplitter(const char* str_id, ImVec2* size)
    {
        ImVec2 screen_pos = GetCursorScreenPos();
        InvisibleButton(str_id, ImVec2(3, -1));
        ImVec2 end_pos = screen_pos + GetItemRectSize();
        ImGuiWindow* win = GetCurrentWindow();
        ImVec4* colors = GetStyle().Colors;
        ImU32 color = GetColorU32(IsItemActive() || IsItemHovered() ? colors[ImGuiCol_ButtonActive] : colors[ImGuiCol_Button]);
        win->DrawList->AddRectFilled(screen_pos, end_pos, color);
        if (ImGui::IsItemActive())
        {
            size->x = ImMax(1.0f, ImGui::GetIO().MouseDelta.x + size->x);
        }
    }
} // namespace

#include "widgets/color_picker.inl"
#include "widgets/color_wheel.inl"
#include "widgets/dock.inl"
#include "widgets/file_list.inl"
#include "widgets/gizmo.inl"
#include "widgets/memory_editor.inl"
#include "widgets/range_slider.inl"

