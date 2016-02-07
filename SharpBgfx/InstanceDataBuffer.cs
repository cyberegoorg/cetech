using System;

namespace SharpBgfx {
    /// <summary>
    /// Maintains a data buffer that contains instancing data.
    /// </summary>
    public unsafe struct InstanceDataBuffer : IEquatable<InstanceDataBuffer> {
        internal readonly NativeStruct* ptr;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly InstanceDataBuffer Invalid = new InstanceDataBuffer();

        /// <summary>
        /// A pointer that can be filled with instance data.
        /// </summary>
        public IntPtr Data { get { return ptr->data; } }

        /// <summary>
        /// The size of the data buffer.
        /// </summary>
        public int Size { get { return ptr->size; } }

        /// <summary>
        /// Initializes a new instance of the <see cref="InstanceDataBuffer" /> struct.
        /// </summary>
        /// <param name="count">The number of elements in the buffer.</param>
        /// <param name="stride">The stride of each element.</param>
        public InstanceDataBuffer (int count, int stride) {
            ptr = NativeMethods.bgfx_alloc_instance_data_buffer(count, (ushort)stride);
        }

        /// <summary>
        /// Checks for available space to allocate an instance buffer.
        /// </summary>
        /// <param name="count">The number of elements to allocate.</param>
        /// <param name="stride">The stride of each element.</param>
        /// <returns><c>true</c> if there is space available to allocate the buffer.</returns>
        public static bool CheckAvailableSpace (int count, int stride) {
            return NativeMethods.bgfx_check_avail_instance_data_buffer(count, (ushort)stride);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (InstanceDataBuffer other) {
            return ptr == other.ptr;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as InstanceDataBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return new IntPtr(ptr).GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Size: {0}", Size);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(InstanceDataBuffer left, InstanceDataBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(InstanceDataBuffer left, InstanceDataBuffer right) {
            return !left.Equals(right);
        }

#pragma warning disable 649
        internal struct NativeStruct {
            public IntPtr data;
            public int size;
            public int offset;
            public ushort stride;
            public ushort num;
            public ushort handle;
        }
#pragma warning restore 649
    }
}
