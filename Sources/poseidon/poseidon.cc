#include <sstream>
#include "poseidon/common.h"
#include "poseidon/poseidon.h"

namespace poseidon{
//TODO: refactor
DEFINE_int64(new_zone_size, 16 * kMB, "The size of the new zone in bytes.");
DEFINE_int64(old_page_size, 16 * kMB, "The size of the old zone pages in bytes.");
DEFINE_int32(max_heap_pages, 16, "The maximum number of pages for the heap.");
DEFINE_int64(heap_page_size, 1 * kMB, "The size of a heap page in bytes.");
DEFINE_int64(large_object_size, 1 * kMB, "The max size of an object before it gets classified as a large object.");

    std::string GetVersion(){
        std::stringstream ss;
        ss << POSEIDON_VERSION_MAJOR << ".";
        ss << POSEIDON_VERSION_MINOR << ".";
        ss << POSEIDON_VERSION_PATCH;
        return ss.str();
    }
}