using CETech.Develop;
using MsgPack;
using YamlDotNet.RepresentationModel;

namespace CETech
{
    public partial class ComponentSystem
    {
        public delegate void Compiler(YamlMappingNode body, ConsoleServer.ResponsePacker packer);

        public delegate void Spawner(int world, int[] ent_ids, int[] ents_parent, MessagePackObjectDictionary[] data);

        public static void RegisterCompiler(long type, Compiler compiler)
        {
            RegisterCompilerImpl(type, compiler);
        }

        public static void Compile(long type, YamlMappingNode body, ConsoleServer.ResponsePacker packer)
        {
            CompileImpl(type, body, packer);
        }

        public static void RegisterSpawnerCompiler(long type, Spawner spawner)
        {
            RegisterSpawnerCompilerImpl(type, spawner);
        }

        public static void Spawn(int world, long type, int[] ent_ids, int[] ents_parent, MessagePackObjectDictionary[] data)
        {
            SpawnImpl(world, type, ent_ids, ents_parent, data);
        }
    }
}