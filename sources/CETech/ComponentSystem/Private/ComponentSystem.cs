using System.Collections.Generic;
using CETech.Develop;
using MsgPack;
using YamlDotNet.RepresentationModel;

namespace CETech
{
    public partial class ComponentSystem
    {
        private static readonly Dictionary<long, Spawner> _spawnerMap = new Dictionary<long, Spawner>();
        private static readonly Dictionary<long, Compiler> _compoilerMap = new Dictionary<long, Compiler>();

        public static void RegisterCompilerImpl(long type, Compiler compiler)
        {
            _compoilerMap[type] = compiler;
        }

        public static void CompileImpl(long type, YamlMappingNode body, ConsoleServer.ResponsePacker packer)
        {
            _compoilerMap[type](body, packer);
        }

        public static void RegisterSpawnerCompilerImpl(long type, Spawner spawner)
        {
            _spawnerMap[type] = spawner;
        }

        public static void SpawnImpl(int world, long type, int[] ent_ids, MessagePackObjectDictionary[] data)
        {
            _spawnerMap[type](world, ent_ids, data);
        }
    }
}