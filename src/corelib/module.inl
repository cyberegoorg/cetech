#ifndef CETECH_MODULE_INL
#define CETECH_MODULE_INL

#if defined(CT_DYNAMIC_MODULE)
#define CT_MODULE_API static
#else
#define CT_MODULE_API extern
#endif

#define CT_MODULE(name) CT_MODULE_API struct name* name

#endif //CETECH_MODULE_INL
