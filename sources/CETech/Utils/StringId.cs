namespace CETech
{
    public struct StringId
    {
        public bool Equals(StringId other)
        {
            return hash == other.hash;
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            return obj is StringId && Equals((StringId) obj);
        }

        public override int GetHashCode()
        {
            return hash.GetHashCode();
        }

        private long hash;

        public StringId(string input)
        {
            hash = GetHashCodeInt64(input);
        }

        public StringId(long hash)
        {
            this.hash = hash;
        }

        public static bool operator ==(StringId a, StringId b)
        {
            return a.hash == b.hash;
        }

        public static bool operator !=(StringId a, StringId b)
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

        public static implicit operator long(StringId strid)
        {
            return strid.hash;
        }

        public override string ToString()
        {
            return string.Format("{0:x}", hash);
        }

        public long Hash
        {
            get { return hash; }
            set { hash = value; }
        }
    }
}