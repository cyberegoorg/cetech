using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace CETech
{
	/// <summary>
	/// Config system.
	/// </summary>
    public static partial class ConfigSystem
    {
		/// <summary>
		/// Config value type.
		/// </summary>
		public enum ConfigValueType
		{
			/// <summary>
			/// The int.
			/// </summary>
			Int,

			/// <summary>
			/// The float.
			/// </summary>
			Float,

			/// <summary>
			/// The string.
			/// </summary>
			String
		}

		/// <summary>
		/// Gets the type of the value.
		/// </summary>
		/// <returns>The value type.</returns>
		/// <param name="name">Name.</param>
        public static ConfigValueType GetValueType(string name)
        {
			return GetValueTypeImpl(name);
        }

		/// <summary>
		/// Create int value.
		/// </summary>
		/// <param name="name">Name.</param>
		/// <param name="description">Description.</param>
		/// <param name="value">Value.</param>
        public static void CreateValue(string name, string description, int value)
        {
			CreateValueImpl (name, description, value);
        }

		/// <summary>
		/// Create float value.
		/// </summary>
		/// <param name="name">Name.</param>
		/// <param name="description">Description.</param>
		/// <param name="value">Value.</param>
        public static void CreateValue(string name, string description, float value)
        {
			CreateValueImpl (name, description, value);
        }

		/// <summary>
		/// Creates string value.
		/// </summary>
		/// <param name="name">Name.</param>
		/// <param name="description">Description.</param>
		/// <param name="value">Value.</param>
        public static void CreateValue(string name, string description, string value)
        {
			CreateValueImpl (name, description, value);
        }

		/// <summary>
		/// Set the value.
		/// </summary>
		/// <param name="name">Name.</param>
		/// <param name="value">Value.</param>
        public static void SetValue(string name, int value)
        {
			SetValueImpl (name, value);
        }

		/// <summary>
		/// Sets the value.
		/// </summary>
		/// <param name="name">Name.</param>
		/// <param name="value">Value.</param>
        public static void SetValue(string name, float value)
        {
			SetValueImpl (name, value);
        }

		/// <summary>
		/// Sets the value.
		/// </summary>
		/// <param name="name">Name.</param>
		/// <param name="value">Value.</param>
        public static void SetValue(string name, string value)
        {
			SetValueImpl (name, value);
        }

		/// <summary>
		/// Gets the value int.
		/// </summary>
		/// <returns>The value int.</returns>
		/// <param name="name">Name.</param>
        public static int GetValueInt(string name)
        {
			return GetValueIntImpl (name);
        }

		/// <summary>
		/// Gets the value float.
		/// </summary>
		/// <returns>The value float.</returns>
		/// <param name="name">Name.</param>
        public static float GetValueFloat(string name)
        {
			return GetValueFloatImpl (name);
        }

		/// <summary>
		/// Gets the value string.
		/// </summary>
		/// <returns>The value string.</returns>
		/// <param name="name">Name.</param>
        public static string GetValueString(string name)
        {
			return GetValueStringImpl (name);
        }
    }
}