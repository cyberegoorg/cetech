using System;
using Common;
using SharpBgfx;

static class Program {
    static void Main () {
        // create a UI thread and kick off a separate render thread
        var sample = new Sample("Hello World", 1280, 720);
        sample.Run(RenderThread);
    }

    static void RenderThread (Sample sample) {
        // initialize the renderer
        Bgfx.Init();
        Bgfx.Reset(sample.WindowWidth, sample.WindowHeight, ResetFlags.Vsync);

        // enable debug text
        Bgfx.SetDebugFeatures(DebugFeatures.DisplayText);

        // set view 0 clear state
        Bgfx.SetViewClear(0, ClearTargets.Color | ClearTargets.Depth, 0x303030ff);

        // main loop
        while (sample.ProcessEvents(ResetFlags.Vsync)) {
            // set view 0 viewport
            Bgfx.SetViewRect(0, 0, 0, sample.WindowWidth, sample.WindowHeight);

            //  make sure view 0 is cleared if no other draw calls are submitted
            Bgfx.Touch(0);

            // write some debug text
            Bgfx.DebugTextClear();
            Bgfx.DebugTextWrite(0, 1, DebugColor.White, DebugColor.Blue, "SharpBgfx/Samples/00-HelloWorld");
            Bgfx.DebugTextWrite(0, 2, DebugColor.White, DebugColor.Cyan, "Description: Initialization and debug text.");

            // advance to the next frame. Rendering thread will be kicked to
            // process submitted rendering primitives.
            Bgfx.Frame();
        }

        // clean up
        Bgfx.Shutdown();
    }
}
