using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace System.Yaml
{
    /// <summary>
    ///     Extend string object to have .DoubleQuoteEscape() / .DoubleQuoteUnescape().
    /// </summary>
    internal static class StringYamlDoubleQuoteEscapeExtention
    {
        /// <summary>
        ///     Escape control codes with YAML double quoted string format.
        /// </summary>
        /// <param name="s"></param>
        /// <returns></returns>
        public static string YamlDoubleQuoteEscape(this string s)
        {
            return YamlDoubleQuoteEscaping.Escape(s);
        }

        /// <summary>
        ///     Unescape control codes escaped with YAML double quoted string format.
        /// </summary>
        /// <param name="s"></param>
        /// <returns></returns>
        public static string YamlDoubleQuoteUnescape(this string s)
        {
            return YamlDoubleQuoteEscaping.Unescape(s);
        }
    }

    /// <summary>
    ///     YAML style double quoted string escape / unescape.
    /// </summary>
    internal static class YamlDoubleQuoteEscaping
    {
        private const int controlCodeMax = 0x1f;

        private static readonly string[] controlCodes = new string[controlCodeMax + 1]
        {
            @"\0", @"\x01", @"\x02", @"\x03", @"\x04", @"\x05", @"\x06", @"\a",
            @"\b", @"\t", @"\n", @"\v", @"\f", @"\r", @"\x0e", @"\x0f",
            @"\x10", @"\x11", @"\x12", @"\x13", @"\x14", @"\x15", @"\x16", @"\x17",
            @"\x18", @"\x19", @"\x1a", @"\e", @"\x1c", @"\x1d", @"\x1e", @"\x1f"
        };

        private static readonly Dictionary<char, string> escapeTable = new Dictionary<char, string>();
        private static readonly Regex escapeRegexp;
        private static readonly Regex escapeNonprintable;

        private static readonly Dictionary<string, string> unescapeTable = new Dictionary<string, string>();
        private static readonly Regex unescapeRegexp;

        /// <summary>
        ///     Initialize tables
        /// </summary>
        static YamlDoubleQuoteEscaping()
        {
            // Create (additional) escaping table
            escapeTable['\\'] = @"\\";
            escapeTable['"'] = "\\\"";
            escapeTable['/'] = @"\/";
            escapeTable['\x85'] = @"\N";
            escapeTable['\xa0'] = @"\_";
            escapeTable['\u2028'] = @"\L";
            escapeTable['\u2029'] = @"\P";

            // Create escaping regexp
            escapeRegexp = new Regex(@"[\x00-\x1f\/\x85\xa0\u2028\u2029" + "\"]");
            escapeNonprintable = new Regex(@"[\x7f-\x84\x86-\x9f\ud800-\udfff\ufffe\uffff]");

            // Create unescaping table
            for (var c = '\0'; c < controlCodes.Length; c++)
                unescapeTable[controlCodes[c]] = c.ToString();
            foreach (var c in escapeTable.Keys)
                unescapeTable[escapeTable[c]] = c.ToString();

            // Create unescaping regex
            var pattern = "";
            unescapeTable.Keys.ToList().ForEach(esc =>
            {
                if (pattern != "")
                    pattern += "|";
                pattern += Regex.Escape(esc);
            });
            unescapeRegexp = new Regex(pattern +
                                       @"|\\x[0-9a-fA-F]{2}|\\u[0-9a-fA-F]{4}|\\U[0-9a-fA-F]{8}");
        }

        /// <summary>
        ///     Escape control codes, double quotations, backslashes in the YAML double quoted string format
        /// </summary>
        public static string Escape(string s)
        {
            s = s.Replace(@"\", @"\\");
            s = escapeRegexp.Replace(s, escapeChar);
            return escapeNonprintable.Replace(s, m =>
            {
                var c = m.Value[0];
                return c < 0x100 ? string.Format(@"\x{0:x2}", (int) c) : string.Format(@"\u{0:x4}", (int) c);
            });
        }

        private static string escapeChar(Match m)
        {
            var c = m.Value[0];
            return c < controlCodes.Length ? controlCodes[c] : escapeTable[c];
        }

        /// <summary>
        ///     Unescape control codes, double quotations, backslashes escape in the YAML double quoted string format
        /// </summary>
        public static string Unescape(string s)
        {
            return unescapeRegexp.Replace(s, unescapeChar);
        }

        private static string unescapeChar(Match m)
        {
            string s;
            switch (m.Value[1])
            {
                case 'x':
                case 'u':
                case 'U':
                    s = ((char) Convert.ToInt32("0x" + m.Value.Substring(2))).ToString();
                    break;
                default:
                    s = unescapeTable[m.Value];
                    break;
            }
            return s;
        }
    }
}