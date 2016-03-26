using System.Collections.Generic;
using System.Diagnostics;

namespace CETech
{
    public static partial class ConfigSystem
    {
        private static Dictionary<string, int> ConfigValueInt;
        private static Dictionary<string, float> ConfigValueFloat;
        private static Dictionary<string, string> ConfigValueString;
        private static Dictionary<string, string> ConfigDescription;

        private static readonly Dictionary<string, ConfigValueType> ConfigType =
            new Dictionary<string, ConfigValueType>();

        private static ConfigValueType GetValueTypeImpl(string name)
        {
            return ConfigType[name];
        }

        private static void CreateValueImpl(string name, string description, int value)
        {
            ConfigType[name] = ConfigValueType.Int;
            ConfigValueInt[name] = value;
            ConfigDescription[name] = description;
        }

        private static void CreateValueImpl(string name, string description, float value)
        {
            ConfigType[name] = ConfigValueType.Float;
            ConfigValueFloat[name] = value;
            ConfigDescription[name] = description;
        }

        private static void CreateValueImpl(string name, string description, string value)
        {
            ConfigType[name] = ConfigValueType.String;
            ConfigValueString[name] = value;
            ConfigDescription[name] = description;
        }

        private static void SetValueImpl(string name, int value)
        {
            Debug.Assert(ConfigType[name] == ConfigValueType.Int);

            ConfigValueInt[name] = value;
        }

        private static void SetValueImpl(string name, float value)
        {
            Debug.Assert(ConfigType[name] == ConfigValueType.Float);

            ConfigValueFloat[name] = value;
        }

        private static void SetValueImpl(string name, string value)
        {
            Debug.Assert(ConfigType[name] == ConfigValueType.String);

            ConfigValueString[name] = value;
        }

        private static int GetValueIntImpl(string name)
        {
            return ConfigValueInt[name];
        }

        private static float GetValueFloatImpl(string name)
        {
            return ConfigValueFloat[name];
        }

        private static string GetValueStringImpl(string name)
        {
            return ConfigValueString[name];
        }

        private static void InitImpl()
        {
            ConfigValueInt = new Dictionary<string, int>();
            ConfigValueFloat = new Dictionary<string, float>();
            ConfigValueString = new Dictionary<string, string>();
            ConfigDescription = new Dictionary<string, string>();
        }

        private static void ShutdownImpl()
        {

        }
    }
}