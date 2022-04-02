#ifndef POSEIDON_FLAGS_H
#define POSEIDON_FLAGS_H

#include <gflags/gflags.h>

namespace poseidon{
 DECLARE_int64(new_zone_size);
 DECLARE_int64(old_zone_size);
 DECLARE_int64(heap_page_size);
 DECLARE_int32(max_heap_pages);
 DECLARE_int64(large_object_size);

 DECLARE_int32(num_workers);
}

#endif //POSEIDON_FLAGS_H
