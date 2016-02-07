using System;
using Common;
using SharpBgfx;

static class Program {
    const float Step = 0.6f;
    const float Scale = 0.25f;
    const int HighThreshold = 65;
    const int LowThreshold = 57;

    static void Main () {
        // create a UI thread and kick off a separate render thread
        var sample = new Sample("Draw Stress", 1280, 720);
        sample.Run(RenderThread);
    }

    static unsafe void RenderThread (Sample sample) {
        // initialize the renderer
        Bgfx.Init();
        Bgfx.Reset(sample.WindowWidth, sample.WindowHeight, ResetFlags.None);

        // enable debug text
        Bgfx.SetDebugFeatures(DebugFeatures.DisplayText);

        // set view 0 clear state
        Bgfx.SetViewClear(0, ClearTargets.Color | ClearTargets.Depth, 0x303030ff);

        // create vertex and index buffers
        var vbh = Cube.CreateVertexBuffer();
        var ibh = Cube.CreateIndexBuffer();

        // load shaders
        var program = ResourceLoader.LoadProgram("vs_cubes", "fs_cubes");

        // start the frame clock
        var clock = new Clock();
        clock.Start();

        int cubeDim = 15;
        float lastUpdate = 0.0f;
        int frameCount = 0;

        // main loop
        while (sample.ProcessEvents(ResetFlags.None)) {
            // set view 0 viewport
            Bgfx.SetViewRect(0, 0, 0, sample.WindowWidth, sample.WindowHeight);

            // view transforms
            var viewMatrix = Matrix4x4.CreateLookAt(new Vector3(0.0f, 0.0f, -35.0f), Vector3.Zero, Vector3.UnitY);
            var projMatrix = Matrix4x4.CreatePerspectiveFieldOfView((float)Math.PI / 3, (float)sample.WindowWidth / sample.WindowHeight, 0.1f, 100.0f);
            Bgfx.SetViewTransform(0, &viewMatrix.M11, &projMatrix.M11);

            // make sure view 0 is cleared if no other draw calls are submitted
            Bgfx.Touch(0);

            // tick the clock
            var elapsed = clock.Frame();
            var time = clock.TotalTime();
            if (elapsed > 10)
                elapsed = 0;

            frameCount++;
            lastUpdate += elapsed;
            if (lastUpdate > 1.0f) {
                var avgFrameTime = frameCount / lastUpdate;
                if (avgFrameTime > HighThreshold)
                    cubeDim = Math.Min(cubeDim + 2, 40);
                else if (avgFrameTime < LowThreshold)
                    cubeDim = Math.Max(cubeDim - 1, 2);

                frameCount = 0;
                lastUpdate = 0;
            }

            var initial = new Vector3(
                -Step * cubeDim / 2.0f,
                -Step * cubeDim / 2.0f,
                -15.0f
            );

            // write some debug text
            Bgfx.DebugTextClear();
            Bgfx.DebugTextWrite(0, 1, DebugColor.White, DebugColor.Blue, "Description: CPU/driver stress test, maximizing draw calls.");
            Bgfx.DebugTextWrite(0, 2, DebugColor.White, DebugColor.Cyan, "Draw Calls: {0}", cubeDim * cubeDim * cubeDim);
            Bgfx.DebugTextWrite(0, 3, DebugColor.White, DebugColor.Cyan, "Frame:      {0:F3} ms", elapsed * 1000);

            for (int z = 0; z < cubeDim; z++) {
                for (int y = 0; y < cubeDim; y++) {
                    for (int x = 0; x < cubeDim; x++) {
                        // model matrix
                        var transform = Matrix4x4.CreateFromYawPitchRoll(time + x * 0.21f, time + y * 0.37f, time + y * 0.13f);
                        transform = Matrix4x4.CreateScale(Scale) * transform;
                        transform.M41 = initial.X + x * Step;
                        transform.M42 = initial.Y + y * Step;
                        transform.M43 = initial.Z + z * Step;
                        Bgfx.SetTransform(&transform.M11);

                        // set pipeline states
                        Bgfx.SetVertexBuffer(vbh);
                        Bgfx.SetIndexBuffer(ibh);
                        Bgfx.SetRenderState(RenderState.Default);

                        // submit primitives
                        Bgfx.Submit(0, program);
                    }
                }
            }

            // advance to the next frame. Rendering thread will be kicked to
            // process submitted rendering primitives.
            Bgfx.Frame();
        }

        // clean up
        ibh.Dispose();
        vbh.Dispose();
        program.Dispose();
        Bgfx.Shutdown();
    }
}