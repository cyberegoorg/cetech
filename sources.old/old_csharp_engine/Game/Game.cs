namespace CETech
{
    public partial class Game
    {
        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static void Update(float dt)
        {
            UpdateImpl(dt);
        }

        public static void Render()
        {
            RenderImpl();
        }
    }
}