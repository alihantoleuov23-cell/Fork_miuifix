#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <sys/stat.h>
#include <cstdio>

extern "C" void android_main(struct android_app* state) {
    std::string base = state->activity->externalDataPath;
    std::string pdir = base + "/port";
    mkdir(pdir.c_str(), 0777);
    std::string bin = pdir + "/fo2.aarch64";
    struct stat st{};
    if (stat(bin.c_str(), &st) != 0 || st.st_size < 1024 * 1024) {
        AAsset* a = AAssetManager_open(state->activity->assetManager, "fo2/fo2.aarch64", AASSET_MODE_STREAMING);
        if (a) {
            FILE* f = fopen(bin.c_str(), "wb");
            if (f) {
                char buf[16384];
                int n;
                while ((n = AAsset_read(a, buf, sizeof(buf))) > 0) fwrite(buf, 1, n, f);
                fclose(f);
            }
            AAsset_close(a);
        }
    }
    bool started = false;
    while (!state->destroyRequested) {
        int events;
        struct android_poll_source* source;
        while (ALooper_pollOnce(started ? 16 : -1, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(state, source);
            if (state->window && !started) {
                started = true;
                fo2_log_init((pdir + "/runtime.log").c_str());
                guest_start(bin.c_str(), (base + "/gamedata").c_str(), pdir.c_str());
                video_set_surface(state->window);
            }
            if (state->destroyRequested) return;
        }
    }
}
