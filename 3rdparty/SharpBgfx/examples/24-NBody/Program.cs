using System;
using Common;
using SharpBgfx;

static class Program {
    static void Main () {
        // create a UI thread and kick off a separate render thread
        var sample = new Sample("NBody", 1280, 720);
        sample.Run(RenderThread);
    }

    static unsafe void RenderThread (Sample sample) {
        // initialize the renderer
        Bgfx.Init();
        Bgfx.Reset(sample.WindowWidth, sample.WindowHeight, ResetFlags.Vsync);

        // enable debug text
        Bgfx.SetDebugFeatures(DebugFeatures.DisplayText);

        // set view 0 clear state
        Bgfx.SetViewClear(0, ClearTargets.Color | ClearTargets.Depth, 0x303030ff);

        // check capabilities
        var caps = Bgfx.GetCaps();
        var computeSupported = caps.SupportedFeatures.HasFlag(DeviceFeatures.Compute);
        var indirectSupported = caps.SupportedFeatures.HasFlag(DeviceFeatures.DrawIndirect);

        if (computeSupported)
            RunCompute(sample, indirectSupported);
        else
            RunUnsupported(sample);

        // clean up
        Bgfx.Shutdown();
    }

    static unsafe void RunCompute (Sample sample, bool indirectSupported) {
        // build vertex layouts
        var quadLayout = new VertexLayout();
        quadLayout.Begin()
            .Add(VertexAttributeUsage.Position, 2, VertexAttributeType.Float)
            .End();

        var computeLayout = new VertexLayout();
        computeLayout.Begin()
            .Add(VertexAttributeUsage.TexCoord0, 4, VertexAttributeType.Float)
            .End();

        // static quad data
        var vb = new VertexBuffer(MemoryBlock.FromArray(QuadVertices), quadLayout);
        var ib = new IndexBuffer(MemoryBlock.FromArray(QuadIndices));

        // create compute buffers
        var currPositionBuffer0 = new DynamicVertexBuffer(1 << 15, computeLayout, BufferFlags.ComputeReadWrite);
        var currPositionBuffer1 = new DynamicVertexBuffer(1 << 15, computeLayout, BufferFlags.ComputeReadWrite);
        var prevPositionBuffer0 = new DynamicVertexBuffer(1 << 15, computeLayout, BufferFlags.ComputeReadWrite);
        var prevPositionBuffer1 = new DynamicVertexBuffer(1 << 15, computeLayout, BufferFlags.ComputeReadWrite);

        // load shaders
        var particleProgram = ResourceLoader.LoadProgram("vs_particle", "fs_particle");
        var initInstancesProgram = ResourceLoader.LoadProgram("cs_init_instances");
        var updateInstancesProgram = ResourceLoader.LoadProgram("cs_update_instances");

        // indirect rendering support
        var indirectProgram = SharpBgfx.Program.Invalid;
        var indirectBuffer = IndirectBuffer.Invalid;
        bool useIndirect = false;

        if (indirectSupported) {
            indirectProgram = ResourceLoader.LoadProgram("cs_indirect");
            indirectBuffer = new IndirectBuffer(2);
            useIndirect = true;
        }

        // setup params uniforms
        var paramData = new ParamsData {
            TimeStep = 0.0157f,
            DispatchSize = 32,
            Gravity = 0.109f,
            Damping = 0.25f,
            ParticleIntensity = 0.64f,
            ParticleSize = 0.279f,
            BaseSeed = 57,
            ParticlePower = 3.5f,
            InitialSpeed = 3.2f,
            InitialShape = 1,
            MaxAccel = 100.0f
        };

        // have the compute shader run initialization
        var u_params = new Uniform("u_params", UniformType.Vector4, 3);
        Bgfx.SetUniform(u_params, &paramData, 3);
        Bgfx.SetComputeBuffer(0, prevPositionBuffer0, ComputeBufferAccess.Write);
        Bgfx.SetComputeBuffer(1, currPositionBuffer0, ComputeBufferAccess.Write);
        Bgfx.Dispatch(0, initInstancesProgram, MaxParticleCount / ThreadGroupUpdateSize);

        // start the frame clock
        var clock = new Clock();
        clock.Start();

        // main loop
        while (sample.ProcessEvents(ResetFlags.Vsync)) {
            // tick the clock
            var elapsed = clock.Frame();
            var time = clock.TotalTime();

            // write some debug text
            Bgfx.DebugTextClear();
            Bgfx.DebugTextWrite(0, 1, DebugColor.White, DebugColor.Blue, "SharpBgfx/Samples/24-NBody");
            Bgfx.DebugTextWrite(0, 2, DebugColor.White, DebugColor.Cyan, "Description: N-body simulation with compute shaders using buffers.");
            Bgfx.DebugTextWrite(0, 3, DebugColor.White, DebugColor.Cyan, "Frame: {0:F3} ms", elapsed * 1000);

            // fill the indirect buffer if we're using it
            if (useIndirect) {
                Bgfx.SetUniform(u_params, &paramData, 3);
                Bgfx.SetComputeBuffer(0, indirectBuffer, ComputeBufferAccess.Write);
                Bgfx.Dispatch(0, indirectProgram);
            }

            // update particle positions
            Bgfx.SetComputeBuffer(0, prevPositionBuffer0, ComputeBufferAccess.Read);
            Bgfx.SetComputeBuffer(1, currPositionBuffer0, ComputeBufferAccess.Read);
            Bgfx.SetComputeBuffer(2, prevPositionBuffer1, ComputeBufferAccess.Write);
            Bgfx.SetComputeBuffer(3, currPositionBuffer1, ComputeBufferAccess.Write);
            Bgfx.SetUniform(u_params, &paramData, 3);
            if (useIndirect)
                Bgfx.Dispatch(0, updateInstancesProgram, indirectBuffer, 1);
            else
                Bgfx.Dispatch(0, updateInstancesProgram, paramData.DispatchSize);

            // ping-pong the buffers for next frame
            Swap(ref currPositionBuffer0, ref currPositionBuffer1);
            Swap(ref prevPositionBuffer0, ref prevPositionBuffer1);

            // view transforms for particle rendering
            var viewMatrix = Matrix4x4.CreateLookAt(new Vector3(0.0f, 0.0f, -45.0f), -Vector3.UnitZ, Vector3.UnitY);
            var projMatrix = Matrix4x4.CreatePerspectiveFieldOfView((float)Math.PI / 4, (float)sample.WindowWidth / sample.WindowHeight, 0.1f, 10000.0f);
            Bgfx.SetViewTransform(0, &viewMatrix.M11, &projMatrix.M11);
            Bgfx.SetViewRect(0, 0, 0, sample.WindowWidth, sample.WindowHeight);

            // draw the particles
            Bgfx.SetVertexBuffer(vb);
            Bgfx.SetIndexBuffer(ib);
            Bgfx.SetInstanceDataBuffer(currPositionBuffer0, 0, paramData.DispatchSize * ThreadGroupUpdateSize);
            Bgfx.SetRenderState(RenderState.ColorWrite | RenderState.BlendAdd | RenderState.DepthTestAlways);
            if (useIndirect)
                Bgfx.Submit(0, particleProgram, indirectBuffer);
            else
                Bgfx.Submit(0, particleProgram);

            // done with frame
            Bgfx.Frame();
        }

        // cleanup
        if (indirectSupported) {
            indirectProgram.Dispose();
            indirectBuffer.Dispose();
        }

        u_params.Dispose();
        currPositionBuffer0.Dispose();
        currPositionBuffer1.Dispose();
        prevPositionBuffer0.Dispose();
        prevPositionBuffer1.Dispose();
        updateInstancesProgram.Dispose();
        initInstancesProgram.Dispose();
        particleProgram.Dispose();
        ib.Dispose();
        vb.Dispose();
    }

    static void RunUnsupported (Sample sample) {
        // main loop
        while (sample.ProcessEvents(ResetFlags.Vsync)) {
            Bgfx.SetViewRect(0, 0, 0, sample.WindowWidth, sample.WindowHeight);

            Bgfx.DebugTextClear();
            Bgfx.DebugTextWrite(0, 1, DebugColor.White, DebugColor.Blue, "SharpBgfx/Samples/24-NBody");
            Bgfx.DebugTextWrite(0, 2, DebugColor.White, DebugColor.Cyan, "Description: N-body simulation with compute shaders using buffers.");
            Bgfx.DebugTextWrite(0, 5, DebugColor.White, DebugColor.Red, "Compute is not supported by your GPU.");

            Bgfx.Touch(0);
            Bgfx.Frame();
        }
    }

    static void Swap<T>(ref T left, ref T right) {
        var temp = left;
        left = right;
        right = temp;
    }

    struct ParamsData {
        public float TimeStep;
        public int DispatchSize;
        public float Gravity;
        public float Damping;
        public float ParticleIntensity;
        public float ParticleSize;
        public int BaseSeed;
        public float ParticlePower;
        public float InitialSpeed;
        public int InitialShape;
        public float MaxAccel;
    };

    const int ThreadGroupUpdateSize = 512;
    const int MaxParticleCount = 32 * 1024;

    static readonly float[] QuadVertices = {
         1.0f,  1.0f,
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,
    };

    static readonly ushort[] QuadIndices = { 0, 1, 2, 2, 3, 0, };
}