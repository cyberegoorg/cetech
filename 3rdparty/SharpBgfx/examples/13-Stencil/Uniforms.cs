using Common;
using SharpBgfx;
using System;

unsafe class Uniforms : IDisposable, IUniformGroup {
    const int MaxLights = 5;

    Uniform parametersHandle;
    Uniform ambientHandle;
    Uniform diffuseHandle;
    Uniform specularHandle;
    Uniform colorHandle;
    Uniform lightPosRadiusHandle;
    Uniform lightRgbInnerRHandle;

    public bool AmbientPass {
        get;
        set;
    }

    public bool LightingPass {
        get;
        set;
    }

    public int LightCount {
        get;
        set;
    }

    public Vector4 Color {
        get;
        set;
    }

    public Vector4[] LightPosRadius { get; set; }
    public Vector4[] LightColor { get; set; }

    public Uniforms () {
        parametersHandle = new Uniform("u_params", UniformType.Vector4);
        ambientHandle = new Uniform("u_ambient", UniformType.Vector4);
        diffuseHandle = new Uniform("u_diffuse", UniformType.Vector4);
        specularHandle = new Uniform("u_specular_shininess", UniformType.Vector4);
        colorHandle = new Uniform("u_color", UniformType.Vector4);
        lightPosRadiusHandle = new Uniform("u_lightPosRadius", UniformType.Vector4, MaxLights);
        lightRgbInnerRHandle = new Uniform("u_lightRgbInnerR", UniformType.Vector4, MaxLights);

        LightPosRadius = new Vector4[MaxLights];
        LightColor = new Vector4[MaxLights];
        for (int i = 0; i < MaxLights; i++) {
            LightPosRadius[i] = new Vector4(0.0f, 0.0f, 0.0f, 1.0f);
            LightColor[i] = new Vector4(1.0f);
        }

        Color = new Vector4(1.0f);
    }

    public void SubmitConstUniforms () {
        var ambient = new Vector4(0.02f, 0.02f, 0.02f, 0.0f);
        var diffuse = new Vector4(0.2f, 0.2f, 0.2f, 0.0f);
        var shininess = new Vector4(1.0f, 1.0f, 1.0f, 10.0f);

        Bgfx.SetUniform(ambientHandle, &ambient);
        Bgfx.SetUniform(diffuseHandle, &diffuse);
        Bgfx.SetUniform(specularHandle, &shininess);
    }
    
    public void SubmitPerDrawUniforms () {
        var color = Color;
        var param = new Vector4(
            AmbientPass ? 1.0f : 0.0f,
            LightingPass ? 1.0f : 0.0f,
            LightCount,
            0.0f
        );

        Bgfx.SetUniform(parametersHandle, &param);
        Bgfx.SetUniform(colorHandle, &color);

        fixed (Vector4* ptr = LightPosRadius)
            Bgfx.SetUniform(lightPosRadiusHandle, ptr, MaxLights);
        fixed (Vector4* ptr = LightColor)
            Bgfx.SetUniform(lightRgbInnerRHandle, ptr, MaxLights);
    }

    public void Dispose () {
        parametersHandle.Dispose();
        ambientHandle.Dispose();
        diffuseHandle.Dispose();
        specularHandle.Dispose();
        colorHandle.Dispose();
        lightPosRadiusHandle.Dispose();
        lightRgbInnerRHandle.Dispose();
    }
}