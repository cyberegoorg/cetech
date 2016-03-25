using System.Collections.Generic;

namespace CETech
{
    public partial class EntityManager
    {
        private static readonly int _indexBitCount = 22;
        private static readonly int _genBitCount = 8;
        private static readonly int _minFreeIndexs = 1024;

        private static Queue<int> _freeIdx;
        private static List<int> _generation;

        private static void InitImpl()
        {
            _freeIdx = new Queue<int>();
            _generation = new List<int>();
        }

        private static void ShutdownImpl()
        {
        }

        public static int CreateImpl()
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

            return make_entity(idx, _generation[idx]);
        }

        public static void DestroyImpl(int entity)
        {
            var id = idx(entity);

            _generation[id] += 1;
            _freeIdx.Enqueue(id);
        }

        public static bool AliveImpl(int entity)
        {
            return _generation[idx(entity)] == gen(entity);
        }


        private static int idx(int entity)
        {
            return entity >> _indexBitCount;
        }

        private static int gen(int entity)
        {
            return entity & ((1 << _genBitCount) - 1);
        }

        private static int make_entity(int idx, int gen)
        {
            return (idx << _indexBitCount) | gen;
        }
    }
}