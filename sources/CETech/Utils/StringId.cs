namespace CETech
{
    public static class StringId
    {
        public static long FromString(string str)
        {
            var s1 = str.Substring(0, str.Length/2);
            var s2 = str.Substring(str.Length/2);

            var x = (long) s1.GetHashCode() << 0x20 | s2.GetHashCode();

            return x;
        }

        public static string ToHexString(long stringid)
        {
            return string.Format("{0:x}", stringid);
        }
    }
}