using System.Runtime.InteropServices;

namespace CETech.Utils
{
    public class ByteUtils
    {
        public static byte[] ToByteArray(object obj)
        {
            var len = Marshal.SizeOf(obj);
            var arr = new byte[len];
            var ptr = Marshal.AllocHGlobal(len);

            Marshal.StructureToPtr(obj, ptr, true);
            Marshal.Copy(ptr, arr, 0, len);
            Marshal.FreeHGlobal(ptr);

            return arr;
        }

        public static T FromBytes<T>(byte[] arr, int start_idx = 0)
        {
            var str = default(T);

            var size = Marshal.SizeOf(str);
            var ptr = Marshal.AllocHGlobal(size);

            Marshal.Copy(arr, start_idx, ptr, size);

            str = (T) Marshal.PtrToStructure(ptr, str.GetType());
            Marshal.FreeHGlobal(ptr);

            return str;
        }
    }
}