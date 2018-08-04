#ifndef CE_MODULE_INL
#define CE_MODULE_INL

#if defined(CE_DYNAMIC_MODULE)
#define CE_MODULE_API static
#else
#define CE_MODULE_API extern
#endif

#define CE_MODULE(name) CE_MODULE_API struct name* name

#endif //CE_MODULE_INL
