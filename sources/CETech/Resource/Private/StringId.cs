namespace CETech.Resource
{
    public static partial class StringId
    {
        private static long FromStringImpl(string str)
        {
            var s1 = str.Substring(0, str.Length/2);
            var s2 = str.Substring(str.Length/2);

            var x = (long) s1.GetHashCode() << 0x20 | s2.GetHashCode();

            return x;
        }
    }
}