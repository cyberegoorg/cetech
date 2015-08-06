#include "queue.h"
#include "string_stream.h"
#include "murmur_hash.h"
#include "hash.h"
#include "temp_allocator.h"
#include "array.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>

#define ASSERT(x) assert(x)

namespace {
	using namespace foundation;
	
	void test_memory() {
		memory_globals::init();
		Allocator &a = memory_globals::default_allocator();

		void *p = a.allocate(100);
		ASSERT(a.allocated_size(p) >= 100);
		ASSERT(a.total_allocated() >= 100);
		void *q = a.allocate(100);
		ASSERT(a.allocated_size(q) >= 100);
		ASSERT(a.total_allocated() >= 200);
		
		a.deallocate(p);
		a.deallocate(q);
		
		memory_globals::shutdown();
	}

	void test_array() {
		memory_globals::init();
		Allocator &a = memory_globals::default_allocator();

		{
			Array<int> v(a);

			ASSERT(array::size(v) == 0);
			array::push_back(v, 3);
			ASSERT(array::size(v) == 1);
			ASSERT(v[0] == 3);

			Array<int> v2(v);
			ASSERT(v2[0] == 3);
			v2[0] = 5;
			ASSERT(v[0] == 3);
			ASSERT(v2[0] == 5);
			v2 = v;
			ASSERT(v2[0] == 3);
			
			ASSERT(array::end(v) - array::begin(v) == array::size(v));
			ASSERT(*array::begin(v) == 3);
			array::pop_back(v);
			ASSERT(array::empty(v));

			for (int i=0; i<100; ++i)
				array::push_back(v, i);
			ASSERT(array::size(v) == 100);
		}

		memory_globals::shutdown();
	}

	void test_scratch() {
		memory_globals::init(256*1024);
		Allocator &a = memory_globals::default_scratch_allocator();

		char *p = (char *)a.allocate(10*1024);

		char *pointers[100];
		for (int i=0; i<100; ++i)
			pointers[i] = (char *)a.allocate(1024);
		for (int i=0; i<100; ++i)
			a.deallocate(pointers[i]);

		a.deallocate(p);

		for (int i=0; i<100; ++i)
			pointers[i] = (char *)a.allocate(4*1024);
		for (int i=0; i<100; ++i)
			a.deallocate(pointers[i]);

		memory_globals::shutdown();
	}

	void test_temp_allocator() {
		memory_globals::init();
		{
			TempAllocator128 ta;
			Array<int> a(ta);
			for (int i=0; i<100; ++i)
				array::push_back(a, i);
			ta.allocate(2*1024);
		}
		memory_globals::shutdown();
	}

	void test_hash() {
		memory_globals::init();
		{
			TempAllocator128 ta;
			Hash<int> h(ta);
			ASSERT(hash::get(h,0,99) == 99);
			ASSERT(!hash::has(h, 0));
			hash::remove(h, 0);
			hash::set(h, 1000, 123);
			ASSERT(hash::get(h,1000,0) == 123);
			ASSERT(hash::get(h,2000,99) == 99);

			for (int i=0; i<100; ++i)
				hash::set(h, i, i*i);
			for (int i=0; i<100; ++i)
				ASSERT(hash::get(h,i,0) == i*i);
			hash::remove(h, 1000);
			ASSERT(!hash::has(h, 1000));
			hash::remove(h, 2000);
			ASSERT(hash::get(h,1000,0) == 0);
			for (int i=0; i<100; ++i)
				ASSERT(hash::get(h,i,0) == i*i);
			hash::clear(h);
			for (int i=0; i<100; ++i)
				ASSERT(!hash::has(h,i));
		}
		memory_globals::shutdown();
	}

	void test_multi_hash()
	{
		memory_globals::init();
		{
			TempAllocator128 ta;
			Hash<int> h(ta);

			ASSERT(multi_hash::count(h, 0) == 0);
			multi_hash::insert(h, 0, 1);
			multi_hash::insert(h, 0, 2);
			multi_hash::insert(h, 0, 3);
			ASSERT(multi_hash::count(h, 0) == 3);

			Array<int> a(ta);
			multi_hash::get(h, 0, a);
			ASSERT(array::size(a) == 3);
			std::sort(array::begin(a), array::end(a));
			ASSERT(a[0] == 1 && a[1] == 2 && a[2] == 3);

			multi_hash::remove(h, multi_hash::find_first(h, 0));
			ASSERT(multi_hash::count(h,0) == 2);
			multi_hash::remove_all(h, 0);
			ASSERT(multi_hash::count(h, 0) == 0);
		}
		memory_globals::shutdown();
	}

	void test_murmur_hash()
	{
		const char *s = "test_string";
		uint64_t h = murmur_hash_64(s, strlen(s), 0);
		ASSERT(h == 0xe604acc23b568f83ull);
	}

	void test_pointer_arithmetic()
	{
		const char check = (char)0xfe;
		const unsigned test_size = 128;

		TempAllocator512 ta;
		Array<char> buffer(ta);
		array::set_capacity(buffer, test_size);
		memset(array::begin(buffer), 0, array::size(buffer));

		void* data = array::begin(buffer);
		for (unsigned i = 0; i != test_size; ++i) {
			buffer[i] = check;
			char* value = (char*)memory::pointer_add(data, i);
			ASSERT(*value == buffer[i]);
		}
	}

	void test_string_stream()
	{
		memory_globals::init();
		{
			using namespace string_stream;

			TempAllocator1024 ta;
			Buffer ss(ta);

			ss << "Name"; 			tab(ss, 20);	ss << "Score\n";
			repeat(ss, 10, '-');	tab(ss, 20);	repeat(ss, 10, '-'); ss << "\n";
			ss << "Niklas"; 		tab(ss, 20);	printf(ss, "%.2f", 2.7182818284f); ss << "\n";
			ss << "Jim"; 			tab(ss, 20);	printf(ss, "%.2f", 3.14159265f); ss << "\n";

			ASSERT(
				0 == strcmp(c_str(ss),
					"Name                Score\n"
					"----------          ----------\n"
					"Niklas              2.72\n"
					"Jim                 3.14\n"
				)
			);
		}
		memory_globals::shutdown();
	}

	void test_queue()
	{
		memory_globals::init();
		{
			TempAllocator1024 ta;
			Queue<int> q(ta);

			queue::reserve(q, 10);
			ASSERT(queue::space(q) == 10);
			queue::push_back(q, 11);
			queue::push_front(q, 22);
			ASSERT(queue::size(q) == 2);
			ASSERT(q[0] == 22);
			ASSERT(q[1] == 11);
			queue::consume(q, 2);
			ASSERT(queue::size(q) == 0);
			int items[] = {1,2,3,4,5,6,7,8,9,10};
			queue::push(q,items,10);
			ASSERT(queue::size(q) == 10);
			for (int i=0; i<10; ++i)
				ASSERT(q[i] == i+1);
			queue::consume(q, queue::end_front(q) - queue::begin_front(q));
			queue::consume(q, queue::end_front(q) - queue::begin_front(q));
			ASSERT(queue::size(q) == 0);
		}
	}
}

int main(int, char **)
{
	test_memory();
	test_array();
	test_scratch();
	test_temp_allocator();
	test_hash();
	test_multi_hash();
	test_murmur_hash();
	test_pointer_arithmetic();
	test_string_stream();
	test_queue();
	return 0;
}