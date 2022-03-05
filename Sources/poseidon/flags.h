#ifndef POSEIDON_FLAGS_H
#define POSEIDON_FLAGS_H

#include <gflags/gflags.h>

namespace poseidon{
 DECLARE_uint64(new_zone_size);
 DECLARE_uint64(old_zone_size);
 DECLARE_uint64(heap_page_size);
 DECLARE_uint32(max_heap_pages);
 DECLARE_uint64(large_object_size);
}

#endif //POSEIDON_FLAGS_H
