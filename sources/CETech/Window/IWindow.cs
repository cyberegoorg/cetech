using System;

namespace CELib.Window
{
    public interface IWindow
    {
        string Title { get; set; }
        int Width { get; }
        int Height { get; }
        IntPtr NativeWindowPtr { get; }

        void Update();
        void Resize(int width, int height);
    }
}