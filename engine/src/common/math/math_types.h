// based on bitsquid foundation
#pragma once

namespace cetech1 {
    /*! 2D Vector.
    */
    struct Vector2 {
        float x, y;
    };

    /*! 3D Vector.
    */
    struct Vector3 {
        float x, y, z;
    };

    /*! 4D Vector.
    */
    struct Vector4 {
        float x, y, z, w;
    };

    /*! Quaternion.
    */
    struct Quat {
        float x, y, z, w;
    };

    /*! Matrix 3x3
    */
    struct Matrix33 {
        Vector3 x, y, z;
    };

    /*! Matrix 4x4
    */
    struct Matrix44 {
        Vector3 x, y, z, t;
    };
}
