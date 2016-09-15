using System.Collections.Generic;

namespace System.Yaml
{
    internal class AnchorDictionary
    {
        private readonly Func<string, object[], bool> error;
        private readonly Dictionary<string, YamlNode> Items = new Dictionary<string, YamlNode>();
        private readonly Stack<RewindInfo> ItemsToRewind = new Stack<RewindInfo>();

        public AnchorDictionary(Func<string, object[], bool> error)
        {
            this.error = error;
        }

        public YamlNode this[string anchor_name]
        {
            get
            {
                if (!Items.ContainsKey(anchor_name))
                    Error("Anchor {0} has not been registered.", anchor_name);
                return Items[anchor_name];
            }
        }

        public int RewindDeapth
        {
            get { return ItemsToRewind.Count; }
            set
            {
                if (RewindDeapth < value)
                    throw new ArgumentOutOfRangeException();
                while (value < RewindDeapth)
                {
                    var rewind_item = ItemsToRewind.Pop();
                    if (rewind_item.old_value == null)
                    {
                        Items.Remove(rewind_item.anchor_name);
                    }
                    else
                    {
                        Items[rewind_item.anchor_name] = rewind_item.old_value;
                    }
                }
            }
        }

        private bool Error(string format, params object[] args)
        {
            return error(format, args);
        }

        public void Add(string anchor_name, YamlNode node)
        {
            if (Items.ContainsKey(anchor_name))
            {
                // override an existing anchor
                ItemsToRewind.Push(new RewindInfo(anchor_name, this[anchor_name]));
                Items[anchor_name] = node;
            }
            else
            {
                ItemsToRewind.Push(new RewindInfo(anchor_name, null));
                Items.Add(anchor_name, node);
            }
        }

        private struct RewindInfo
        {
            public RewindInfo(string anchor_name, YamlNode old_value)
            {
                this.anchor_name = anchor_name;
                this.old_value = old_value;
            }

            public readonly string anchor_name;
            public readonly YamlNode old_value;
        }
    }
}