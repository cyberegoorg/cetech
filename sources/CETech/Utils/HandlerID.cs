using System.Collections.Generic;

namespace CETech
{
    public class HandlerID
    {
        private readonly int _genBitCount = 8;
        private readonly int _indexBitCount = 22;
        private readonly int _minFreeIndexs = 1024;

        private readonly Queue<int> _freeIdx = new Queue<int>();
        private readonly List<int> _generation = new List<int>();

        public int Create()
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

        public void Destroy(int entity)
        {
            var id = idx(entity);

            _generation[id] += 1;
            _freeIdx.Enqueue(id);
        }

        public bool Alive(int entity)
        {
            return _generation[idx(entity)] == gen(entity);
        }


        private int idx(int entity)
        {
            return entity >> _indexBitCount;
        }

        private int gen(int entity)
        {
            return entity & ((1 << _genBitCount) - 1);
        }

        private int make_entity(int idx, int gen)
        {
            return (idx << _indexBitCount) | gen;
        }
    }
}