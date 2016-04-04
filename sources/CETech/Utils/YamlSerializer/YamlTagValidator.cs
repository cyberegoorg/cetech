using System.Linq;
using System.Text.RegularExpressions;

namespace System.Yaml
{
    /// <summary>
    ///     Validates a text as a global tag in YAML.
    ///     <a href="http://www.faqs.org/rfcs/rfc4151.html">RFC4151 - The 'tag' URI Scheme</a>>
    /// </summary>
    internal class YamlTagValidator : Parser<YamlTagValidator.Status>
    {
        private readonly Func<char, bool> alphaNumCharset = Charset(c =>
            c < 0x100 && (
                ('0' <= c && c <= '9') ||
                ('A' <= c && c <= 'Z') ||
                ('a' <= c && c <= 'z')
                )
            );

        private readonly Regex dateRegex =
            new Regex(@"(19[89][0-9]|20[0-4][0-9])(-(0[1-9]|1[0-2])(-(0[1-9]|[12][0-9]|3[01]))?)?");

        private readonly Func<char, bool> hexDigCharset = Charset(c =>
            c < 0x100 && (
                ('0' <= c && c <= '9') ||
                ('A' <= c && c <= 'F') ||
                ('a' <= c && c <= 'f')
                )
            );

        private readonly Func<char, bool> numCharset = Charset(c =>
            c < 0x100 && '0' <= c && c <= '9'
            );

        private readonly Func<char, bool> pcharCharsetSub = Charset(c =>
            c < 0x100 && (
                ('0' <= c && c <= '9') ||
                ('A' <= c && c <= 'Z') ||
                ('a' <= c && c <= 'z') ||
                "-._~!$&'()*+,;=:@".Contains(c)
                )
            );

        public static YamlTagValidator Default { get; } = new YamlTagValidator();

        /// <summary>
        ///     Validates a text as a global tag in YAML.
        /// </summary>
        /// <param name="tag">A candidate for a global tag in YAML.</param>
        /// <returns>True if <paramref name="tag" /> is  a valid global tag.</returns>
        public bool IsValid(string tag)
        {
            text = tag;
            p = 0;
            return TagUri();
        }

        private bool TagUri()
        {
            return
                Accept("tag:") &&
                taggingEntity() &&
                Accept(':') &&
                specific() &&
                Optional(
                    Accept('#') &&
                    fragment()
                    ) &&
                EndOfString();
        }

        private bool taggingEntity()
        {
            return
                RewindUnless(() =>
                    authorityName() &&
                    Accept(',') &&
                    date()
                    );
        }

        private bool authorityName()
        {
            return
                emailAddress() ||
                DNSname();
        }

        private bool DNSname()
        {
            return
                DNScomp() &&
                Repeat(() =>
                    Accept('.') &&
                    DNScomp()
                    );
        }

        private bool DNScomp()
        {
            return RewindUnless(() =>
                OneAndRepeat(alphaNum) &&
                Repeat(() => RewindUnless(() =>
                    Accept('-') &&
                    OneAndRepeat(alphaNum)
                    ))
                );
        }

        private bool alphaNum()
        {
            return
                Accept(alphaNumCharset);
        }

        private bool emailAddress()
        {
            return RewindUnless(() =>
                OneAndRepeat(() => alphaNum() || Accept('-') || Accept('.') || Accept('_')) &&
                Accept('@') &&
                DNSname()
                );
        }

        private bool date()
        {
            return
                Accept(dateRegex);
        }

        private bool num()
        {
            return
                Accept(numCharset);
        }

        private bool specific()
        {
            return
                Repeat(() => pchar() || Accept('/') || Accept('?'));
        }

        private bool fragment()
        {
            return
                Repeat(() => pchar() || Accept('/') || Accept('?'));
        }

        private bool EndOfString()
        {
            return text.Length == p;
        }

        private bool pchar()
        {
            return
                Accept(pcharCharsetSub) ||
                RewindUnless(() =>
                    Accept('%') &&
                    hexDig() &&
                    hexDig()
                    );
        }

        private bool hexDig()
        {
            return Accept(hexDigCharset);
        }

        /// <summary>
        ///     Not used in this parser
        /// </summary>
        public struct Status
        {
        }
    }
}