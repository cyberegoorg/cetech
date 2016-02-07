using Common;
using SharpBgfx;
using System;
using System.Collections.Generic;

static class Program {
    static void Main () {
        // create a UI thread and kick off a separate render thread
        var sample = new Sample("Stencil Test", 1280, 720);
        sample.Run(RenderThread);
    }

    static unsafe void RenderThread (Sample sample) {
        // initialize the renderer
        Bgfx.Init(RendererBackend.Direct3D11);
        Bgfx.Reset(sample.WindowWidth, sample.WindowHeight, ResetFlags.Vsync);

        // enable debug text
        Bgfx.SetDebugFeatures(DebugFeatures.DisplayText);

        // load shaders
        var programTextureLighting = ResourceLoader.LoadProgram("vs_stencil_texture_lighting", "fs_stencil_texture_lighting");
        var programColorLighting = ResourceLoader.LoadProgram("vs_stencil_color_lighting", "fs_stencil_color_lighting");
        var programColorTexture = ResourceLoader.LoadProgram("vs_stencil_color_texture", "fs_stencil_color_texture");
        var programColorBlack = ResourceLoader.LoadProgram("vs_stencil_color", "fs_stencil_color_black");
        var programTexture = ResourceLoader.LoadProgram("vs_stencil_texture", "fs_stencil_texture");

        // load meshes
        var bunnyMesh = ResourceLoader.LoadMesh("bunny.bin");
        var columnMesh = ResourceLoader.LoadMesh("column.bin");
        var hplaneMesh = new Mesh(MemoryBlock.FromArray(StaticMeshes.HorizontalPlane), PosNormalTexcoordVertex.Layout, StaticMeshes.PlaneIndices);
        var vplaneMesh = new Mesh(MemoryBlock.FromArray(StaticMeshes.VerticalPlane), PosNormalTexcoordVertex.Layout, StaticMeshes.PlaneIndices);

        // load textures
        var figureTex = ResourceLoader.LoadTexture("figure-rgba.dds");
        var flareTex = ResourceLoader.LoadTexture("flare.dds");
        var fieldstoneTex = ResourceLoader.LoadTexture("fieldstone-rgba.dds");

        // create uniforms
        var colorTextureHandle = new Uniform("u_texColor", UniformType.Int1);
        var uniforms = new Uniforms();
        uniforms.SubmitConstUniforms();

        // light colors
        uniforms.LightColor = new[] {
            new Vector4(1.0f, 0.7f, 0.2f, 0.0f), // yellow
            new Vector4(0.7f, 0.2f, 1.0f, 0.0f), // purple
            new Vector4(0.2f, 1.0f, 0.7f, 0.0f), // cyan
            new Vector4(1.0f, 0.4f, 0.2f, 0.0f)  // orange
        };

        // camera
        var camera = new Camera(60.0f, sample.WindowWidth, sample.WindowHeight, 0.1f, 100.0f);
        camera.Position = new Vector3(0.0f, 18.0f, -40.0f);

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
            Bgfx.DebugTextWrite(0, 1, DebugColor.White, DebugColor.Blue, "SharpBgfx/Samples/13-Stencil");
            Bgfx.DebugTextWrite(0, 2, DebugColor.White, DebugColor.Cyan, "Description: Stencil reflections.");
            Bgfx.DebugTextWrite(0, 3, DebugColor.White, DebugColor.Cyan, "Frame: {0:F3} ms", elapsed * 1000);

            // clear the background
            Bgfx.SetViewClear(BaseId, ClearTargets.Color | ClearTargets.Depth | ClearTargets.Stencil, 0x30303000);
            Bgfx.SetViewRect(BaseId, 0, 0, sample.WindowWidth, sample.WindowHeight);
            Bgfx.Touch(BaseId);

            // set view params for each pass
            var viewMtx = camera.GetViewMatrix();
            var projMtx = camera.GetProjectionMatrix();
            for (byte i = PassId0; i <= PassId4; i++) {
                Bgfx.SetViewRect(i, 0, 0, sample.WindowWidth, sample.WindowHeight);
                Bgfx.SetViewTransform(i, (float*)&viewMtx, (float*)&projMtx);
            }

            // first pass - draw ground plane
            var floorMtx = FloorTransform;
            hplaneMesh.Submit(PassId0, programColorBlack, &floorMtx, StateGroups[PrebuiltRenderState.StencilReflectionCraftStencil], uniforms);

            // second pass - reflected objects
            Bgfx.SetViewClear(PassId1, ClearTargets.Depth, 0);
            uniforms.AmbientPass = true;
            uniforms.LightingPass = true;
            uniforms.Color = new Vector4(0.70f, 0.65f, 0.60f, 0.8f);
            uniforms.LightCount = LightCount;

            // light positions
            var lightPositions = new Vector4[LightCount];
            var reflectedLights = new Vector4[LightCount];
            for (int i = 0; i < lightPositions.Length; i++) {
                var v3 = new Vector3(
                    (float)Math.Sin(time * 1.1 + i * 0.03 + i * 1.07 * Math.PI / 2) * 20.0f,
                    8.0f + (1.0f - (float)Math.Cos(time * 1.5 + i * 0.29 + 1.49f * Math.PI / 2)) * 4.0f,
                    (float)Math.Cos(time * 1.3 + i * 0.13 + i * 1.79 * Math.PI / 2) * 20.0f
                );

                lightPositions[i] = new Vector4(v3, 15.0f);
                reflectedLights[i] = new Vector4(Vector3.Transform(v3, ReflectionTransform), 15.0f);
            }

            uniforms.LightPosRadius = reflectedLights;
            var bunnyMtx =
                Matrix4x4.CreateScale(5) *
                Matrix4x4.CreateRotationY(time - 1.56f) *
                Matrix4x4.CreateTranslation(0.0f, 2.0f, 0.0f);
            var reflectedBunnyMtx = bunnyMtx * ReflectionTransform;
            bunnyMesh.Submit(PassId1, programColorLighting, &reflectedBunnyMtx, StateGroups[PrebuiltRenderState.StencilReflectionDrawReflected], uniforms);

            for (int i = 0; i < 4; i++) {
                var mtx = ColumnTransforms[i] * ReflectionTransform;
                columnMesh.Submit(PassId1, programColorLighting, &mtx, StateGroups[PrebuiltRenderState.StencilReflectionDrawReflected], uniforms);
            }

            // third pass - blend the plane and reflections
            uniforms.LightPosRadius = lightPositions;
            hplaneMesh.Submit(PassId2, programTextureLighting, &floorMtx, StateGroups[PrebuiltRenderState.StencilReflectionBlendPlane], uniforms, fieldstoneTex, colorTextureHandle);

            // fourth pass - draw the solid objects
            bunnyMesh.Submit(PassId3, programColorLighting, &bunnyMtx, StateGroups[PrebuiltRenderState.StencilReflectionDrawScene], uniforms);
            for (int i = 0; i < 4; i++) {
                var mtx = ColumnTransforms[i];
                columnMesh.Submit(PassId3, programColorLighting, &mtx, StateGroups[PrebuiltRenderState.StencilReflectionDrawScene], uniforms);
            }

            // fifth pass - draw the lights as objects
            for (int i = 0; i < LightCount; i++) {
                var c = uniforms.LightColor[i];
                uniforms.Color = new Vector4(c.X, c.Y, c.Z, 0.8f);

                var p = lightPositions[i];
                var mtx = Matrix4x4.CreateScale(1.5f) * Matrix4x4.CreateBillboard(new Vector3(p.X, p.Y, p.Z), camera.Position, Vector3.UnitY, -Vector3.UnitZ);
                vplaneMesh.Submit(PassId4, programColorTexture, &mtx, StateGroups[PrebuiltRenderState.CustomBlendLightTexture], uniforms, flareTex, colorTextureHandle);
            }

            // advance to the next frame. Rendering thread will be kicked to
            // process submitted rendering primitives.
            Bgfx.Frame();
        }

        // clean up
        bunnyMesh.Dispose();
        columnMesh.Dispose();
        hplaneMesh.Dispose();
        vplaneMesh.Dispose();

        figureTex.Dispose();
        fieldstoneTex.Dispose();
        flareTex.Dispose();

        programTextureLighting.Dispose();
        programColorLighting.Dispose();
        programColorTexture.Dispose();
        programColorBlack.Dispose();
        programTexture.Dispose();

        colorTextureHandle.Dispose();
        uniforms.Dispose();

        Bgfx.Shutdown();
    }

    const int LightCount = 4;

    const byte BaseId = 0;
    const byte PassId0 = 1;
    const byte PassId1 = 2;
    const byte PassId2 = 3;
    const byte PassId3 = 4;
    const byte PassId4 = 5;

    const float ColumnDistance = 14.0f;

    static readonly Matrix4x4 FloorTransform = Matrix4x4.CreateScale(20.0f);
    static readonly Matrix4x4 ReflectionTransform = Matrix4x4.CreateReflection(Vector3.UnitY, -0.01f);
    static readonly Matrix4x4[] ColumnTransforms = new[] {
        Matrix4x4.CreateTranslation(ColumnDistance, 0.0f, ColumnDistance),
        Matrix4x4.CreateTranslation(-ColumnDistance, 0.0f, ColumnDistance),
        Matrix4x4.CreateTranslation(ColumnDistance, 0.0f, -ColumnDistance),
        Matrix4x4.CreateTranslation(-ColumnDistance, 0.0f, -ColumnDistance)
    };

    enum PrebuiltRenderState {
        StencilReflectionCraftStencil,
        StencilReflectionDrawReflected,
        StencilReflectionBlendPlane,
        StencilReflectionDrawScene,
        ProjectionShadowsDrawAmbient,
        ProjectionShadowsCraftStencil,
        ProjectionShadowsDrawDiffuse,
        CustomBlendLightTexture,
        CustomDrawPlaneBottom
    }

    static Dictionary<PrebuiltRenderState, RenderStateGroup> StateGroups = new Dictionary<PrebuiltRenderState, RenderStateGroup> {
        { PrebuiltRenderState.StencilReflectionCraftStencil, new RenderStateGroup(
            RenderState.DepthWrite |
            RenderState.DepthTestLess |
            RenderState.Multisampling |
            RenderState.ColorWrite,
            uint.MaxValue,
            StencilFlags.TestAlways |
            StencilFlags.ReferenceValue(1) |
            StencilFlags.ReadMask(0xff) |
            StencilFlags.FailSReplace |
            StencilFlags.FailZReplace |
            StencilFlags.PassZReplace,
            StencilFlags.None)
        },
        { PrebuiltRenderState.StencilReflectionDrawReflected, new RenderStateGroup(
            RenderState.ColorWrite |
            RenderState.AlphaWrite |
            RenderState.DepthWrite |
            RenderState.DepthTestLess |
            RenderState.CullCounterclockwise |
            RenderState.Multisampling |
            RenderState.BlendAlpha,
            uint.MaxValue,
            StencilFlags.TestEqual |
            StencilFlags.ReferenceValue(1) |
            StencilFlags.ReadMask(1) |
            StencilFlags.FailSKeep |
            StencilFlags.FailZKeep |
            StencilFlags.PassZKeep,
            StencilFlags.None)
        },
        { PrebuiltRenderState.StencilReflectionBlendPlane, new RenderStateGroup(
            RenderState.ColorWrite |
            RenderState.DepthWrite |
            RenderState.DepthTestLess |
            RenderState.BlendFunction(RenderState.BlendOne, RenderState.BlendSourceColor) |
            RenderState.CullClockwise |
            RenderState.Multisampling,
            uint.MaxValue,
            StencilFlags.None,
            StencilFlags.None)
        },
        { PrebuiltRenderState.StencilReflectionDrawScene, new RenderStateGroup(
            RenderState.ColorWrite |
            RenderState.DepthWrite |
            RenderState.DepthTestLess |
            RenderState.CullClockwise |
            RenderState.Multisampling,
            uint.MaxValue,
            StencilFlags.None,
            StencilFlags.None)
        },
        { PrebuiltRenderState.CustomBlendLightTexture, new RenderStateGroup(
            RenderState.ColorWrite |
            RenderState.AlphaWrite |
            RenderState.DepthWrite |
            RenderState.DepthTestLess |
            RenderState.CullClockwise |
            RenderState.Multisampling |
            RenderState.BlendFunction(RenderState.BlendSourceColor, RenderState.BlendInverseSourceColor),
            uint.MaxValue,
            StencilFlags.None,
            StencilFlags.None)
        }
    };
}