namespace CELib.Window
{
    public interface IWindow
    {
        string Title { get; set; }

        void Update();
        void Resize(int width, int height);
    }
}