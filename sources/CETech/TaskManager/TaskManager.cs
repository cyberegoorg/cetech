using System;
using System.Threading;
using CETech.Utils;

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

    public class TaskManager
    {
        public delegate void TaskWork(object data);

        public static int NullTask = 0;

        private static readonly Task[] _taskPool = new Task[4096];
        private static readonly int[] _openTasks = new int[4096];
        private static int _openTaskCount;

        private static readonly TaskQueue[] _gloalQueue = new TaskQueue[3];
        private static TaskQueue[] _workersQueue;

        private static Thread[] _workers;

        [ThreadStatic] private static int _worker_id;

        private static bool _Run;

        private static SpinLock _doneLock = new SpinLock();

        public static int WorkerId
        {
            get { return _worker_id; }
        }

        public static int OpenTaskCount
        {
            get { return _openTaskCount; }
        }

        public static void Init()
        {
            var core_count = Environment.ProcessorCount;

            var main_threads_count = 1 + 1;
            var worker_count = core_count - main_threads_count;

            Log.Info("task_manager", "Core count: {0}" + Environment.NewLine +
                                     "  Main thread count: {1}" + Environment.NewLine +
                                     "  Worker count: {2}", core_count, main_threads_count, worker_count);

            for (var i = 0; i < _gloalQueue.Length; i++)
            {
                _gloalQueue[i] = new TaskQueue(4096);
            }

            _workersQueue = new TaskQueue[(worker_count + 1)*3];
            for (var i = 0; i < _workersQueue.Length; i++)
            {
                _workersQueue[i] = new TaskQueue(4096);
            }

            if (worker_count > 0)
            {
                _workers = new Thread[worker_count];

                for (var i = 0; i < worker_count; ++i)
                {
                    var next_i = i + 1;

                    Log.Debug("task_manager", "Creating worker {0}", next_i);

                    _workers[i] = new Thread(_task_worker);
                    _workers[i].Start(next_i);
                    _workers[i].Name = string.Format("Worker{0}", next_i);
                }
            }

            _Run = true;
        }

        public static void Shutdown()
        {
            _Run = false;

            for (var i = 0; i < _workers.Length; ++i)
            {
                _workers[i].Join();
            }
        }

        public static int AddBegin(string name, TaskWork taskWork, object data,
            TaskPriority priority = TaskPriority.Normal, int depend = 0, int parent = 0,
            TaskAffinity affinity = TaskAffinity.None)
        {
            var task = _new_task();

            _taskPool[task].Name = name;
            _taskPool[task].Priority = priority;
            _taskPool[task].TaskWork = taskWork;
            _taskPool[task].JobCount = 2;
            _taskPool[task].Depend = depend;
            _taskPool[task].Parent = parent;
            _taskPool[task].Affinity = affinity;
            _taskPool[task].TaskData = data;

            if (parent != 0)
            {
                Interlocked.Increment(ref _taskPool[parent].JobCount);
            }

            _openTasks[_openTaskCount] = task;
            Interlocked.Increment(ref _openTaskCount);

            return task;
        }

        public static int AddNull(string name, TaskPriority priority = TaskPriority.Normal, int depend = 0,
            int parent = 0, TaskAffinity affinity = TaskAffinity.None)
        {
            return AddBegin(name, delegate { }, null, priority, depend, parent, affinity);
        }

        public static void AddEnd(int[] tasks)
        {
            for (var i = 0; i < tasks.Length; ++i)
            {
                Interlocked.Decrement(ref _taskPool[tasks[i]].JobCount);
            }

            for (var i = 0; i < tasks.Length; ++i)
            {
                _push_task(tasks[i]);
            }
        }


        public static void DoWork()
        {
            var t = _task_pop_new_work();

            if (t == 0)
            {
                return;
            }

            _taskPool[t].TaskWork(_taskPool[t].TaskData);

            _mark_task_job_done(t);
        }

        public static void Wait(int id)
        {
            while (!_task_is_done(id))
            {
                DoWork();
            }
        }

        private static void _task_worker(object o)
        {
            // Wait for run signal 0 -> 1
            while (!_Run)
            {
            }

            _worker_id = (int) o;

            Log.Info("task_worker", "Worker {0} init", _worker_id);

            while (_Run)
            {
                DoWork();
            }

            Log.Info("task_worker", "Worker {0} shutdown", _worker_id);
        }

        private static bool _task_is_done(int task)
        {
            var ret = true;
            var count = _openTaskCount;

            for (var i = 0; i < count; ++i)
            {
                if (_openTasks[i] != task)
                {
                    continue;
                }

                ret = false;
                break;
            }

            return ret;
        }

        private static int _task_pop_new_work()
        {
            int popedTask;

            for (var i = 0; i < 3; ++i)
            {
                var q = _workersQueue[_worker_id*3 + i];

                popedTask = _try_pop(q);
                if (popedTask != 0)
                {
                    return popedTask;
                }
            }

            for (var i = 0; i < 3; ++i)
            {
                var q = _gloalQueue[i];

                popedTask = _try_pop(q);
                if (popedTask != 0)
                {
                    return popedTask;
                }
            }

            return 0;
        }


        private static bool can_work_on(int task)
        {
            return (_taskPool[task].JobCount == 1) &&
                   (_taskPool[task].Depend == 0 || _task_is_done(_taskPool[task].Depend));
        }

        private static int _try_pop(TaskQueue queue)
        {
            int poped_task;

            if (queue.Pop(out poped_task))
            {
                if (poped_task != 0)
                {
                    if (can_work_on(poped_task))
                    {
                        return poped_task;
                    }

                    _push_task(poped_task);
                }
            }

            return 0;
        }

        private static void _mark_task_job_done(int task)
        {
            if (_taskPool[task].Parent != 0)
            {
                Interlocked.Decrement(ref _taskPool[_taskPool[task].Parent].JobCount);
            }


            var gotLock = false;
            try
            {
                _doneLock.Enter(ref gotLock);

                for (var i = 0; i < _openTaskCount; ++i)
                {
                    if (_openTasks[i] == task)
                    {
                        var idx = Interlocked.Decrement(ref _openTaskCount);
                        _openTasks[i] = _openTasks[idx];
                        _taskPool[task].Used = false;
                        break;
                    }
                }
            }
            finally
            {
                if (gotLock) _doneLock.Exit();
            }
        }

        private static void _push_task(int i)
        {
            switch (_taskPool[i].Affinity)
            {
                case TaskAffinity.None:
                    _gloalQueue[(int) _taskPool[i].Priority].Push(i);
                    break;

                default:
                    _workersQueue[((int) _taskPool[i].Affinity - 1)*3 + (int) _taskPool[i].Priority].Push(i);
                    break;
            }
        }

        private static int _new_task()
        {
            for (var i = 1; i < 4096; ++i)
            {
                if (_taskPool[i].Used)
                {
                    continue;
                }

                _taskPool[i].Used = true;
                return i;
            }

            throw new OutOfMemoryException();
        }

        internal struct Task
        {
            public TaskWork TaskWork;
            public object TaskData;
            public int JobCount;

            public string Name;

            public int Depend;
            public int Parent;

            public TaskPriority Priority;
            public TaskAffinity Affinity;
            public bool Used;
        }
    }
}