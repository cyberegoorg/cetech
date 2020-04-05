#ifndef CETECH_ASSET_BROWSER_H
#define CETECH_ASSET_BROWSER_H

#ifdef __cplusplus
extern "C" {
#endif

#define ASSET_BROWSER_SOURCE \
    CE_ID64_0("source", 0x921f1370045bad6eULL)

#define ASSET_BROWSER  \
    CE_ID64_0("asset_browser", 0x41bbc9e4707d4c8dULL)

struct ct_asset_browser_a0 {
    uint64_t (*edited)();
};

CE_MODULE(ct_asset_browser_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_ASSET_BROWSER_H
