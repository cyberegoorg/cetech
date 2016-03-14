namespace CETech
{
    using TaskQueue = QueueMpmc<int>;

    public enum TaskPriority
    {
        High = 0,
        Normal = 1,
        Low = 2
    }

    public enum TaskAffinity
    {
        None = 0,
        MainThead,
        Worker1,
        Worker2,
        Worker3,
        Worker4,
        Worker5,
        Worker6,
        Worker7
    }

    public partial class TaskManager
    {
        public delegate void TaskWork(object data);

        public static int NullTask = 0;

        public static int WorkerId
        {
            get { return GetWorkerId(); }
        }

        public static int OpenTaskCount
        {
            get { return GeOpenTaskCountd(); }
        }

        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static int AddBegin(string name, TaskWork taskWork, object data,
            TaskPriority priority = TaskPriority.Normal, int depend = 0, int parent = 0,
            TaskAffinity affinity = TaskAffinity.None)
        {
            return AddBeginImpl(name, taskWork, data, priority, depend, parent, affinity);
        }

        public static int AddNull(string name, TaskPriority priority = TaskPriority.Normal, int depend = 0,
            int parent = 0, TaskAffinity affinity = TaskAffinity.None)
        {
            return AddBegin(name, delegate { }, null, priority, depend, parent, affinity);
        }

        public static void AddEnd(int[] tasks)
        {
            AddEndImpl(tasks);
        }


        public static void DoWork()
        {
            DoWorkImpl();
        }

        public static void Wait(int id)
        {
            WaitImpl(id);
        }

    }
}