using System.Collections.Generic;
using System.Diagnostics;

namespace CETech
{
    public static class Config
    {
        private static readonly Dictionary<string, int> ConfigValueInt = new Dictionary<string, int>();
        private static readonly Dictionary<string, float> ConfigValueFloat = new Dictionary<string, float>();
        private static readonly Dictionary<string, string> ConfigValueString = new Dictionary<string, string>();
        private static readonly Dictionary<string, string> ConfigDescription = new Dictionary<string, string>();

        private static readonly Dictionary<string, ConfigValueType> ConfigType =
            new Dictionary<string, ConfigValueType>();

        public static void CreateValue(string name, string description, int value)
        {
            ConfigType[name] = ConfigValueType.Int;
            ConfigValueInt[name] = value;
            ConfigDescription[name] = description;
        }

        public static void CreateValue(string name, string description, float value)
        {
            ConfigType[name] = ConfigValueType.Float;
            ConfigValueFloat[name] = value;
            ConfigDescription[name] = description;
        }

        public static void CreateValue(string name, string description, string value)
        {
            ConfigType[name] = ConfigValueType.String;
            ConfigValueString[name] = value;
            ConfigDescription[name] = description;
        }

        public static void SetValue(string name, int value)
        {
            Debug.Assert(ConfigType[name] == ConfigValueType.Int);

            ConfigValueInt[name] = value;
        }

        public static void SetValue(string name, float value)
        {
            Debug.Assert(ConfigType[name] == ConfigValueType.Float);

            ConfigValueFloat[name] = value;
        }

        public static void SetValue(string name, string value)
        {
            Debug.Assert(ConfigType[name] == ConfigValueType.String);

            ConfigValueString[name] = value;
        }

        public static int GetValueInt(string name)
        {
            return ConfigValueInt[name];
        }

        public static float GetValueFloat(string name)
        {
            return ConfigValueFloat[name];
        }

        public static string GetValueString(string name)
        {
            return ConfigValueString[name];
        }

        private enum ConfigValueType
        {
            Int,
            Float,
            String
        }
    }
}