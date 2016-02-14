namespace CETech
{
    public struct StringId64
    {
        private readonly long _hash;

        public StringId64(string input)
        {
            _hash = GetHashCodeInt64(input);
        }

        public StringId64(long hash)
        {
            _hash = hash;
        }

        public static bool operator ==(StringId64 a, StringId64 b)
        {
            return a._hash == b._hash;
        }

        public static bool operator !=(StringId64 a, StringId64 b)
        {
            return !(a == b);
        }

        private static long GetHashCodeInt64(string input)
        {
            var s1 = input.Substring(0, input.Length/2);
            var s2 = input.Substring(input.Length/2);

            var x = (long) s1.GetHashCode() << 0x20 | s2.GetHashCode();

            return x;
        }

        public static implicit operator long(StringId64 strid)
        {
            return strid._hash;
        }

        public override string ToString()
        {
            return string.Format("{0:x}", _hash);
        }
    }
}