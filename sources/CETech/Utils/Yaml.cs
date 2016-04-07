using System.Yaml;

namespace CETech.Utils
{
    public class Yaml
    {
        public static void merge(YamlMapping root, YamlMapping parent)
        {
            foreach (var parent_item in parent)
            {
                var node_type = parent_item.Value.ShorthandTag();

                if (!root.ContainsKey(parent_item.Key))
                {
                    root[parent_item.Key] = parent_item.Value;
                }
                else
                {
                    if (node_type == "!!map")
                    {
                        merge((YamlMapping) root[parent_item.Key], (YamlMapping) parent_item.Value);
                    }
                }
            }
        }
    }
}