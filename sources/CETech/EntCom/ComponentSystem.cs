using System.Yaml;
using CETech.Develop;
using MsgPack;

namespace CETech.EntCom
{
    public partial class ComponentSystem
    {
        public delegate void Compiler(YamlMapping body, ConsoleServer.ResponsePacker packer);

        public delegate void Destroyer(int world, int[] ent_ids);

        public delegate void Spawner(int world, int[] ent_ids, int[] ents_parent, MessagePackObjectDictionary[] data);

        public static void RegisterCompiler(long type, Compiler compiler, int spawn_order)
        {
            RegisterCompilerImpl(type, compiler, spawn_order);
        }

        public static int GetSpawnOrder(long type)
        {
            return GetSpawnOrderImpl(type);
        }

        public static void Compile(long type, YamlMapping body, ConsoleServer.ResponsePacker packer)
        {
            CompileImpl(type, body, packer);
        }

        public static void RegisterType(long type, Spawner spawner, Destroyer destroyer)
        {
            RegisterSpawnerImpl(type, spawner, destroyer);
        }

        public static void Spawn(int world, long type, int[] ent_ids, int[] ents_parent,
            MessagePackObjectDictionary[] data)
        {
            SpawnImpl(world, type, ent_ids, ents_parent, data);
        }

        public static void DestroyAllType(int world, int[] ent_ids)
        {
            DestroyAllTypeImpl(world, ent_ids);
        }
    }
}