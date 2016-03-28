using System.Collections.Generic;

namespace CETech.World
{
    public partial class WorldManager
    {
        private static readonly int _indexBitCount = 22;
        private static readonly int _genBitCount = 8;
        private static readonly int _minFreeIndexs = 1024;

        private static Queue<int> _freeIdx;
        private static List<int> _generation;

        public static void InitImpl()
        {
            _freeIdx = new Queue<int>();
            _generation = new List<int>();

        }

        public static void ShutdownImpl()
        {
        }

        public static int CreateWorldImpl()
        {
            var id = CreateId();
            SceneGraph.InitWorld(id);
            return id;
        }

        private static void DestroyWorldImpl(int world)
        {
            SceneGraph.RemoveWorld(world);
            DestroyId(world);
        }

        public static int CreateId()
        {
            int idx;

            if (_freeIdx.Count > _minFreeIndexs)
            {
                idx = _freeIdx.Dequeue();
            }
            else
            {
                _generation.Add(0);
                idx = _generation.Count - 1;
            }

            return make_id(idx, _generation[idx]);
        }

        public static void DestroyId(int id)
        {
            var idxx = idx(id);

            _generation[idxx] += 1;
            _freeIdx.Enqueue(idxx);
        }

        public static bool Alive(int id)
        {
            return _generation[idx(id)] == gen(id);
        }


        private static int idx(int entity)
        {
            return entity >> _indexBitCount;
        }

        private static int gen(int entity)
        {
            return entity & ((1 << _genBitCount) - 1);
        }

        private static int make_id(int idx, int gen)
        {
            return (idx << _indexBitCount) | gen;
        }


    }
}