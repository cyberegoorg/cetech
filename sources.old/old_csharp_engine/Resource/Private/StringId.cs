namespace CETech.Resource
{
    public static partial class StringId64
    {
        //TODO: MURMUR!!!!
        private static long FromStringImpl(string str)
        {
            var len_half = str.Length/2;
            var s1 = str.Substring(0, len_half);
            var s2 = str.Substring(len_half);

            var x = (long) s1.GetHashCode() << 0x20 | s2.GetHashCode();

            return x;
        }
    }
}