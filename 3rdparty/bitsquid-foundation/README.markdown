# foundation

*foundation* is a minimalistic foundation library with functions for memory allocation, data manipulation, etc released under the very permissive MIT license - for your pleasure.

## Library Design

*foundation* has been written with data-oriented programming in mind (POD data is preferred over complicated classes, flat arrays are the preferred data structure, etc).

foundation is written in a "back-to-C" style of C++ programming. This means that there is a clear separation between data and code. Data defitions are found in \_types.h header files. Function definitions are found in .h header files.

When you are writing a system using foundation, your header files typically only need to include the \_types.h files. These are designed to be as compact as possible, meaning your compile times will be short. In the .cpp files you can include the heavier .h files.

*foundation* uses the following types of data:

    // Open structs
    struct Vector3 {
    	float x, y, z;
    };

    // Closed structs
    struct Object {
    	Object(); 
    	unsigned _data;
    };

    // Abstract classes
    class Allocator;

    class Allocator {
    	public:
    		void *allocate(uint32_t size, uint32_t align) = 0;
    };

* **Open structs.** Raw POD structs defined in the \_types.h file. You can directly manipulate the members of these structs.

* **Closed structs.** A closed struct is indicated by the fact that its members start with an underscore. You should not manipulate the members of a closed struct directly. Instead, use the functions in the namespace with the same name as the struct. (In this case: object::). These functions are found in the .h file, unlike the struct definition, which is in the \_types.h file.

    Note that since namespaces are "open" you can extend the functionality for the object by adding your own functions to its namespace.

* **Abstract classes.** Used for high-level systems that are only accessed through pointers and/or references. These are predeclared in the \_types.h file. The virtual interface is found in the .h file.

## Compiler flags

* **PLATFORM_BIG_ENDIAN** Should be defined if you are compiling for a big endian platform.

## Systems

### Memory

* **Allocator** A virtual base class for memory allocation. Can be subclassed to implement custom allocator behaviors.

* **memory_globals::default_allocator()** Returns a default allocator based on malloc().

* **memory_globals::default_scratch_allocator()** Returns a "scratch" allocator that can be used for temporary memory allocations. The scratch allocator allocates its memory from a fixed sized ring buffer, meaning it doesn't touch any OS resources. When the ring buffer loops around, the old memory must have been freed for the scratch buffer to be able to allocate new memory, so only use it for temporary allocations.

* **TempAllocator.** An allocator suitable for temporary allocators. The TempAllocator comes in a number of variations: TempAllocator64, TempAllocator128, etc. The number indicates how much local stack space the allocator reserves. Memory is allocated first from the local stack space and only if that is exhausted from the scratch buffer. Memory allocated with the TempAllocator does not have to be freed. It is freed automatically when the allocator is destroyed.

### Collection

* **Array<T>** Implements an array of objects. A lightweight version of std::vector that assumes that *T* is a POD-object (i.e. constructors and destructors do not have to be called and the object can be moved with memmove).

* **Queue<T>** Implements a double-ended queue/ring-buffer of POD objects. Push items to the back of the queue and pop them from the front.

* **Hash<T>** Implements a lightweight hash that assumes that *T* is a POD-object. The hash keys are always uint64_t numbers. If you want to use some other type of key, just hash it to a uint64_t first. (The hash function should not have any collisions in your domain.) The hash can be used as a regular hash, or as a multi_hash, through the *multi_hash* interface.

* **string_stream** Functions for using an Array<char> as a stream of characters that you can print formatted messages to.

### Math

* Basic data definitions for vectors, quaternions and matrices.