using System;

namespace CETech
{
    public partial class Window
    {
        public int _height;
        private string _title;
        public int _width;


        public Window(string title, WindowPos x, WindowPos y, int width, int height, int flags)
        {
            Width = width;
            Height = height;

            PlatformCtor(title, x, y, width, height, flags);
        }

        public string Title
        {
            get { return _title; }
            set
            {
                _title = value;
                PlarformSetTitle(value);
            }
        }

        public IntPtr NativeWindowPtr
        {
            get { return PlatformNativePtr(); }
        }

        public int Height
        {
            get { return _height; }
            set { _height = value; }
        }

        public int Width
        {
            get { return _width; }
            set { _width = value; }
        }

        public void Update()
        {
            PlatformUpdate();
        }

        public void Resize(int width, int height)
        {
            PlatformResize(width, height);
        }

        ~Window()
        {
            PlatformDtor();
        }
    }
}