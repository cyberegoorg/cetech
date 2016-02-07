using System;
using Common;

static class StaticMeshes {
    public static readonly PosNormalTexcoordVertex[] HorizontalPlane = {
        new PosNormalTexcoordVertex(-1.0f, 0.0f,  1.0f, PackF4u(0.0f, 1.0f, 0.0f), 5.0f, 5.0f),
        new PosNormalTexcoordVertex( 1.0f, 0.0f,  1.0f, PackF4u(0.0f, 1.0f, 0.0f), 5.0f, 0.0f),
        new PosNormalTexcoordVertex(-1.0f, 0.0f, -1.0f, PackF4u(0.0f, 1.0f, 0.0f), 0.0f, 5.0f),
        new PosNormalTexcoordVertex( 1.0f, 0.0f, -1.0f, PackF4u(0.0f, 1.0f, 0.0f), 0.0f, 0.0f)
    };

    public static readonly PosNormalTexcoordVertex[] VerticalPlane = {
        new PosNormalTexcoordVertex(-1.0f,  1.0f, 0.0f, PackF4u(0.0f, 0.0f, -1.0f), 1.0f, 1.0f),
        new PosNormalTexcoordVertex( 1.0f,  1.0f, 0.0f, PackF4u(0.0f, 0.0f, -1.0f), 1.0f, 0.0f),
        new PosNormalTexcoordVertex(-1.0f, -1.0f, 0.0f, PackF4u(0.0f, 0.0f, -1.0f), 0.0f, 1.0f),
        new PosNormalTexcoordVertex( 1.0f, -1.0f, 0.0f, PackF4u(0.0f, 0.0f, -1.0f), 0.0f, 0.0f)
    };

    public static readonly PosNormalTexcoordVertex[] Cube = {
        new PosNormalTexcoordVertex(-1.0f,  1.0f,  1.0f, PackF4u( 0.0f,  1.0f,  0.0f), 1.0f, 1.0f),
        new PosNormalTexcoordVertex( 1.0f,  1.0f,  1.0f, PackF4u( 0.0f,  1.0f,  0.0f), 0.0f, 1.0f),
        new PosNormalTexcoordVertex(-1.0f,  1.0f, -1.0f, PackF4u( 0.0f,  1.0f,  0.0f), 1.0f, 0.0f),
        new PosNormalTexcoordVertex( 1.0f,  1.0f, -1.0f, PackF4u( 0.0f,  1.0f,  0.0f), 0.0f, 0.0f),
        new PosNormalTexcoordVertex(-1.0f, -1.0f,  1.0f, PackF4u( 0.0f, -1.0f,  0.0f), 1.0f, 1.0f),
        new PosNormalTexcoordVertex( 1.0f, -1.0f,  1.0f, PackF4u( 0.0f, -1.0f,  0.0f), 0.0f, 1.0f),
        new PosNormalTexcoordVertex(-1.0f, -1.0f, -1.0f, PackF4u( 0.0f, -1.0f,  0.0f), 1.0f, 0.0f),
        new PosNormalTexcoordVertex( 1.0f, -1.0f, -1.0f, PackF4u( 0.0f, -1.0f,  0.0f), 0.0f, 0.0f),
        new PosNormalTexcoordVertex( 1.0f, -1.0f,  1.0f, PackF4u( 0.0f,  0.0f,  1.0f), 0.0f, 0.0f),
        new PosNormalTexcoordVertex( 1.0f,  1.0f,  1.0f, PackF4u( 0.0f,  0.0f,  1.0f), 0.0f, 1.0f),
        new PosNormalTexcoordVertex(-1.0f, -1.0f,  1.0f, PackF4u( 0.0f,  0.0f,  1.0f), 1.0f, 0.0f),
        new PosNormalTexcoordVertex(-1.0f,  1.0f,  1.0f, PackF4u( 0.0f,  0.0f,  1.0f), 1.0f, 1.0f),
        new PosNormalTexcoordVertex( 1.0f, -1.0f, -1.0f, PackF4u( 0.0f,  0.0f, -1.0f), 0.0f, 0.0f),
        new PosNormalTexcoordVertex( 1.0f,  1.0f, -1.0f, PackF4u( 0.0f,  0.0f, -1.0f), 0.0f, 1.0f),
        new PosNormalTexcoordVertex(-1.0f, -1.0f, -1.0f, PackF4u( 0.0f,  0.0f, -1.0f), 1.0f, 0.0f),
        new PosNormalTexcoordVertex(-1.0f,  1.0f, -1.0f, PackF4u( 0.0f,  0.0f, -1.0f), 1.0f, 1.0f),
        new PosNormalTexcoordVertex( 1.0f,  1.0f, -1.0f, PackF4u( 1.0f,  0.0f,  0.0f), 1.0f, 1.0f),
        new PosNormalTexcoordVertex( 1.0f,  1.0f,  1.0f, PackF4u( 1.0f,  0.0f,  0.0f), 0.0f, 1.0f),
        new PosNormalTexcoordVertex( 1.0f, -1.0f, -1.0f, PackF4u( 1.0f,  0.0f,  0.0f), 1.0f, 0.0f),
        new PosNormalTexcoordVertex( 1.0f, -1.0f,  1.0f, PackF4u( 1.0f,  0.0f,  0.0f), 0.0f, 0.0f),
        new PosNormalTexcoordVertex(-1.0f,  1.0f, -1.0f, PackF4u(-1.0f,  0.0f,  0.0f), 1.0f, 1.0f),
        new PosNormalTexcoordVertex(-1.0f,  1.0f,  1.0f, PackF4u(-1.0f,  0.0f,  0.0f), 0.0f, 1.0f),
        new PosNormalTexcoordVertex(-1.0f, -1.0f, -1.0f, PackF4u(-1.0f,  0.0f,  0.0f), 1.0f, 0.0f),
        new PosNormalTexcoordVertex(-1.0f, -1.0f,  1.0f, PackF4u(-1.0f,  0.0f,  0.0f), 0.0f, 0.0f)
    };

    public static readonly ushort[] PlaneIndices = {
        0, 1, 2,
        1, 3, 2
    };

    public static readonly ushort[] CubeIndices = {
        0,  1,  2,
        1,  3,  2,
        4,  6,  5,
        5,  6,  7,

        8,  9, 10,
        9, 11, 10,
        12, 14, 13,
        13, 14, 15,

        16, 17, 18,
        17, 19, 18,
        20, 22, 21,
        21, 22, 23,
    };

    static uint PackF4u (float x, float y, float z) {
        var bytes = new byte[] {
            (byte)(x * 127.0f + 128.0f),
            (byte)(y * 127.0f + 128.0f),
            (byte)(z * 127.0f + 128.0f),
            128
        };

        return BitConverter.ToUInt32(bytes, 0);
    }
}