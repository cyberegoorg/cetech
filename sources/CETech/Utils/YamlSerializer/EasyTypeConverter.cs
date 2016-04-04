using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;

namespace System.Yaml.Serialization
{
    /// <summary>
    ///     Converts various types to / from string.<br />
    ///     I don't remember why this class was needed....
    /// </summary>
    /// <example>
    ///     <code>
    /// object obj = GetObjectToConvert();
    /// 
    /// // Check if the type has [TypeConverter] attribute.
    /// if( EasyTypeConverter.IsTypeConverterSpecified(type) ) {
    /// 
    ///   // Convert the object to string.
    ///   string s = EasyTypeConverter.ConvertToString(obj);
    /// 
    ///   // Convert the string to an object of the spific type.
    ///   object restored = EasyTypeConverter.ConvertFromString(s, type);
    ///   
    ///   Assert.AreEqual(obj, restored);
    /// 
    /// }
    /// </code>
    /// </example>
    internal class EasyTypeConverter
    {
        private static readonly Dictionary<Type, TypeConverter> TypeConverters = new Dictionary<Type, TypeConverter>();
        private static readonly Dictionary<Type, bool> TypeConverterSpecified = new Dictionary<Type, bool>();
        internal CultureInfo Culture;

        public EasyTypeConverter()
        {
            Culture = CultureInfo.InvariantCulture;
        }

        public static bool IsTypeConverterSpecified(Type type)
        {
            if (!TypeConverterSpecified.ContainsKey(type))
                RegisterTypeConverterFor(type);
            return TypeConverterSpecified[type];
        }

        private static TypeConverter FindConverter(Type type)
        {
            if (!TypeConverters.ContainsKey(type))
            {
                return RegisterTypeConverterFor(type);
            }
            return TypeConverters[type];
        }

        private static TypeConverter RegisterTypeConverterFor(Type type)
        {
            var converter_attr = type.GetAttribute<TypeConverterAttribute>();
            if (converter_attr != null)
            {
                // What is the difference between these two conditions?
                TypeConverterSpecified[type] = true;
                var converterType = TypeUtils.GetType(converter_attr.ConverterTypeName);
                return TypeConverters[type] = Activator.CreateInstance(converterType) as TypeConverter;
            }
            // What is the difference between these two conditions?
            TypeConverterSpecified[type] = false;
            return TypeConverters[type] = TypeDescriptor.GetConverter(type);
        }

        public string ConvertToString(object obj)
        {
            if (obj == null)
                return "null";
            var converter = FindConverter(obj.GetType());
            if (converter != null)
            {
                return converter.ConvertToString(null, Culture, obj);
            }
            return obj.ToString();
        }

        public object ConvertFromString(string s, Type type)
        {
            return FindConverter(type).ConvertFromString(null, Culture, s);
        }
    }
}