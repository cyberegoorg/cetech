using Common;
using System;

class Camera {
    Matrix4x4 projection;
    Vector3 target;
    Vector3 up;

    public Vector3 Position {
        get;
        set;
    }

    public float VerticalAngle {
        get;
        set;
    }

    public float HorizontalAngle {
        get;
        set;
    }

    public Camera (float fov, int width, int height, float near, float far) {
        projection = Matrix4x4.CreatePerspectiveFieldOfView(fov * (float)Math.PI / 180.0f, (float)width / height, near, far);

        Position = new Vector3(0.0f, 0.0f, -35.0f);
        target = new Vector3(0.0f, 0.0f, -1.0f);
        up = new Vector3(0.0f, 1.0f, 0.0f);

        HorizontalAngle = 0.01f;
    }

    public Matrix4x4 GetViewMatrix () {
        return Matrix4x4.CreateLookAt(Position, target, up);
    }

    public Matrix4x4 GetProjectionMatrix () {
        return projection;
    }
}