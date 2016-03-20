namespace CETech
{
	/// <summary>
	/// Task manager.
	/// </summary>
    public partial class TaskManager
    {
		/// <summary>
		/// Task priority.
		/// </summary>
		public enum TaskPriority
		{
			/// <summary>
			/// The high priority.
			/// </summary>
			High = 0,

			/// <summary>
			/// The normal priority.
			/// </summary>
			Normal = 1,

			/// <summary>
			/// The low priority.
			/// </summary>
			Low = 2
		}

		/// <summary>
		/// Task affinity.
		/// </summary>
		public enum TaskAffinity
		{
			/// <summary>
			/// Without affinity.
			/// </summary>
			None = 0,

			/// <summary>
			/// The main thead.
			/// </summary>
			MainThead,

			/// <summary>
			/// The worker 1.
			/// </summary>
			Worker1,

			/// <summary>
			/// The worker 2.
			/// </summary>
			Worker2,

			/// <summary>
			/// The worker 3.
			/// </summary>
			Worker3,

			/// <summary>
			/// The worker 4.
			/// </summary>
			Worker4,

			/// <summary>
			/// The worker 5.
			/// </summary>
			Worker5,

			/// <summary>
			/// The worker 6.
			/// </summary>
			Worker6,

			/// <summary>
			/// The worker 7.
			/// </summary>
			Worker7
		}

		/// <summary>
		/// Task work.
		/// </summary>
		public delegate void TaskWork(object data);

		/// <summary>
		/// The null task.
		/// </summary>
        public static int NullTask = 0;

		/// <summary>
		/// Gets the worker identifier.
		/// </summary>
		/// <value>The worker identifier.</value>
        public static int WorkerId
        {
            get { return GetWorkerId(); }
        }

		/// <summary>
		/// Gets the open task count.
		/// </summary>
		/// <value>The open task count.</value>
        public static int OpenTaskCount
        {
            get { return GeOpenTaskCountd(); }
        }

		/// <summary>
		/// Init task manager
		/// </summary>
        public static void Init()
        {
            InitImpl();
        }

		/// <summary>
		/// Shutdown task manager
		/// </summary>
        public static void Shutdown()
        {
            ShutdownImpl();
        }


		/// <summary>
		/// Prepare tasks.
		/// </summary>
		/// <returns>Task id.</returns>
		/// <param name="name">Name.</param>
		/// <param name="taskWork">Task work.</param>
		/// <param name="data">Data.</param>
		/// <param name="priority">Priority.</param>
		/// <param name="depend">Depend.</param>
		/// <param name="parent">Parent.</param>
		/// <param name="affinity">Affinity.</param>
        public static int AddBegin(string name, TaskWork taskWork, object data,
            TaskPriority priority = TaskPriority.Normal, int depend = 0, int parent = 0,
            TaskAffinity affinity = TaskAffinity.None)
        {
            return AddBeginImpl(name, taskWork, data, priority, depend, parent, affinity);
        }


		/// <summary>
		/// Adds the null task.
		/// </summary>
		/// <returns>Task id.</returns>
		/// <param name="name">Name.</param>
		/// <param name="priority">Priority.</param>
		/// <param name="depend">Depend.</param>
		/// <param name="parent">Parent.</param>
		/// <param name="affinity">Affinity.</param>
        public static int AddNull(string name, TaskPriority priority = TaskPriority.Normal, int depend = 0,
            int parent = 0, TaskAffinity affinity = TaskAffinity.None)
        {
            return AddBegin(name, delegate { }, null, priority, depend, parent, affinity);
        }

		/// <summary>
		/// Flush tasks.
		/// </summary>
		/// <param name="tasks">Tasks.</param>
        public static void AddEnd(int[] tasks)
        {
            AddEndImpl(tasks);
        }

		/// <summary>
		/// Do some work for task manager.
		/// </summary>
        public static void DoWork()
        {
            DoWorkImpl();
        }

		/// <summary>
		/// Wait for specified task id.
		/// </summary>
		/// <param name="id">Identifier.</param>
        public static void Wait(int id)
        {
            WaitImpl(id);
        }
    }
}