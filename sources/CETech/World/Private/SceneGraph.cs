using System.Collections.Generic;
using CETech.CEMath;

namespace CETech.World
{
    public partial class SceneGraph
    {
        private static readonly Dictionary<int, WorldInstance> _worldInstance = new Dictionary<int, WorldInstance>();

        private static int getFirstNode(int world, int entity)
        {
            return _worldInstance[world].EntNode[entity];
        }

        private static void InitWorldImpl(int world)
        {
            _worldInstance[world] = new WorldInstance();
        }

        private static void RemoveWorldImpl(int world)
        {
            _worldInstance.Remove(world);
        }

        private static int GetNodeByNameRecursive(int world, int node, long name)
        {
            var world_instance = _worldInstance[world];

            if (world_instance.Name[node] == name)
            {
                return node;
            }

            var child = world_instance.FirstChild[node];
            while (is_valid(child))
            {
                var result_node = GetNodeByNameRecursive(world, child, name);

                if (result_node != int.MaxValue)
                {
                    return result_node;
                }

                child = world_instance.NextSibling[child];
            }

            return int.MaxValue;
        }

        public static int GetNodeByNameImpl(int world, int entity, long name)
        {
            var world_instance = _worldInstance[world];

            var idx = world_instance.EntNode[entity];
            return GetNodeByNameRecursive(world, idx, name);
        }

        private static int CreateImpl(int world, int entity, long[] names, int[] parents, Mat4f[] pose)
        {
            var world_instance = _worldInstance[world];

            var nodes_map = new int[names.Length];
            for (var i = 0; i < names.Length; i++)
            {
                var idx = world_instance.Position.Count;
                nodes_map[i] = idx;
                world_instance.Name.Add(names[i]);
                world_instance.Parent.Add(int.MaxValue);
                world_instance.FirstChild.Add(int.MaxValue);
                world_instance.NextSibling.Add(int.MaxValue);

                var local_pose = pose[i];
                var local_translation = Mat4f.Translation(local_pose);
                var local_rotation = Mat4f.ToQuat(local_pose);
                var local_scale = Vec3f.Unit;

                world_instance.Position.Add(local_translation);
                world_instance.Scale.Add(local_scale); // TODO: from pose?
                world_instance.Rotation.Add(local_rotation);

                world_instance.World.Add(Mat4f.Identity);

                var parent = parents[i];
                Transform(world, idx,
                    parent != int.MaxValue ? GetWorldMatrix(world, nodes_map[parent]) : Mat4f.Identity);

                if (parent != int.MaxValue)
                {
                    var parentIdx = nodes_map[parent];

                    world_instance.Parent[idx] = parentIdx;

                    if (!is_valid(world_instance.FirstChild[parentIdx]))
                    {
                        world_instance.FirstChild[parentIdx] = idx;
                    }
                    else
                    {
                        var firstChildIdx = world_instance.FirstChild[parentIdx];

                        world_instance.FirstChild[parentIdx] = idx;
                        world_instance.NextSibling[idx] = firstChildIdx;
                    }

                    world_instance.Parent[idx] = parentIdx;
                }
            }

            world_instance.EntNode[entity] = nodes_map[0];
            return nodes_map[0];
        }

        private static bool is_valid(int idx)
        {
            return idx != int.MaxValue;
        }

        private static void Transform(int world, int idx, Mat4f parent)
        {
            var world_instance = _worldInstance[world];

            var pos = world_instance.Position[idx];
            var rot = world_instance.Rotation[idx];
            var sca = world_instance.Scale[idx];

            var rm = Quatf.ToMat4F(rot);
            var sm = Mat4f.CreateScale(sca.X, sca.Y, sca.Z);

            var m = sm*rm;
            m.M41 = pos.X;
            m.M42 = pos.Y;
            m.M43 = pos.Z;

            world_instance.World[idx] = m*parent;

            var child = world_instance.FirstChild[idx];

            while (is_valid(child))
            {
                Transform(world, child, world_instance.World[idx]);
                child = world_instance.NextSibling[child];
            }
        }


        private static void InitImpl()
        {
        }

        private static void Destroyer(int world, int[] entIds)
        {
            // TODO
            var world_instance = _worldInstance[world];

            for (var i = 0; i < entIds.Length; i++)
            {
                var ent_id = entIds[i];
                world_instance.EntNode.Remove(ent_id);
            }
        }

        private static void ShutdownImpl()
        {
        }

        private static Vec3f GetPositionImpl(int world, int transform)
        {
            var world_instance = _worldInstance[world];
            return world_instance.Position[transform];
        }

        private static Quatf GetRotationImpl(int world, int transform)
        {
            var world_instance = _worldInstance[world];
            return world_instance.Rotation[transform];
        }

        private static Vec3f GetScaleImpl(int world, int transform)
        {
            var world_instance = _worldInstance[world];
            return world_instance.Scale[transform];
        }

        private static Mat4f GetWorldMatrixImpl(int world, int transform)
        {
            var world_instance = _worldInstance[world];
            return world_instance.World[transform];
        }

        private static void SetPositionImpl(int world, int transform, Vec3f pos)
        {
            var world_instance = _worldInstance[world];
            var parent_idx = world_instance.Parent[transform];
            var parent = parent_idx != int.MaxValue ? world_instance.World[parent_idx] : Mat4f.Identity;

            world_instance.Position[transform] = pos;
            Transform(world, transform, parent);
        }

        private static void SetRotationImpl(int world, int transform, Quatf rot)
        {
            var world_instance = _worldInstance[world];
            var parent_idx = world_instance.Parent[transform];
            var parent = parent_idx != int.MaxValue ? world_instance.World[parent_idx] : Mat4f.Identity;

            world_instance.Rotation[transform] = Quatf.Normalize(rot);
            Transform(world, transform, parent);
        }

        private static void SetScaleImpl(int world, int transform, Vec3f scale)
        {
            var world_instance = _worldInstance[world];
            var parent_idx = world_instance.Parent[transform];
            var parent = parent_idx != int.MaxValue ? world_instance.World[parent_idx] : Mat4f.Identity;

            world_instance.Scale[transform] = scale;
            Transform(world, transform, parent);
        }

        private static void LinkImpl(int world, int parent, int child)
        {
            var world_instance = _worldInstance[world];

            var parent_idx = getFirstNode(world, parent);
            var child_idx = getFirstNode(world, child);

            world_instance.Parent[child_idx] = parent_idx;

            var tmp = world_instance.FirstChild[parent_idx];

            world_instance.FirstChild[parent_idx] = child_idx;
            world_instance.NextSibling[child_idx] = tmp;

            var p = parent_idx != int.MaxValue ? world_instance.World[parent_idx] : Mat4f.Identity;
            Transform(world, parent_idx, p); // TODO:
            Transform(world, child_idx, GetWorldMatrix(world, parent));
        }

        private static int GetRootNode(int world, int entity)
        {
            return getFirstNode(world, entity);
        }

        public static bool HasSceneGraph(int world, int entity)
        {
            return _worldInstance[world].EntNode.ContainsKey(entity);
        }

        private class WorldInstance
        {
            public readonly Dictionary<int, int> EntNode;
            public readonly List<int> FirstChild;

            public readonly List<long> Name;
            public readonly List<int> NextSibling;
            public readonly List<int> Parent;
            public readonly List<Vec3f> Position;
            public readonly List<Quatf> Rotation;
            public readonly List<Vec3f> Scale;
            public readonly List<Mat4f> World;

            public WorldInstance()
            {
                Name = new List<long>();
                NextSibling = new List<int>();
                EntNode = new Dictionary<int, int>();
                Position = new List<Vec3f>();
                Rotation = new List<Quatf>();
                Scale = new List<Vec3f>();
                Parent = new List<int>();
                FirstChild = new List<int>();
                World = new List<Mat4f>();
            }
        }
    }
}