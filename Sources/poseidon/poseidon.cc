#include <sstream>
#include "poseidon/poseidon.h"

namespace poseidon{
  static const uint64_t kB = 1;
  static const uint64_t kKB = 1024 * kB;
  static const uint64_t kMB = 1024 * kKB;
  static const uint64_t kGB = 1024 * kMB;
  static const uint64_t kTB = 1024 * kGB;

//TODO: refactor
DEFINE_uint64(new_zone_size, 16 * kMB, "The size of the new zone in bytes.");
DEFINE_uint64(old_zone_size, 128 * kMB, "The size of the old zone in bytes.");
DEFINE_uint64(old_page_size, 16 * kMB, "The size of the old zone pages in bytes.");
DEFINE_uint32(max_heap_pages, 16, "The maximum number of pages for the heap.");
DEFINE_uint64(heap_page_size, 4 * kMB, "The size of a heap page in bytes.");
DEFINE_uint64(large_object_size, 1 * kMB, "The max size of an object before it gets classified as a large object.");

    std::string GetVersion(){
        std::stringstream ss;
        ss << POSEIDON_VERSION_MAJOR << ".";
        ss << POSEIDON_VERSION_MINOR << ".";
        ss << POSEIDON_VERSION_PATCH;
        return ss.str();
    }
}