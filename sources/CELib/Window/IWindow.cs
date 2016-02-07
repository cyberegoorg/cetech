namespace CELib.Window
{
    public enum WindowFlags
    {
        WindowNoflag = 0,
        WindowFullscreen = 1
    }

    public enum WindowPos
    {
        WindowposCentered = 1,
        WindowposUndefined = 2
    }

    public interface IWindow
    {
        string Title { get; set; }

        void Update();
        void Resize(int width, int height);
    }
}