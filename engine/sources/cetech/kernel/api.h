//! \addtogroup Plugin
//! \{
#ifndef CETECH_API_H
#define CETECH_API_H

//==============================================================================
// Defines
//==============================================================================

#define IMPORT_API(name) static struct name name;
#define GET_API(api, name) name = *(struct name*) (api)->first(#name)

//==============================================================================
// Api
//==============================================================================

//! Plugin expot api struct V0
struct api_v0 {
    void (*register_api)(const char* name, void* api);
    void* (*first)(const char* name);
};

#endif //CETECH_API_H
//! \}