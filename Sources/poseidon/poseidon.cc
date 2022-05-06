#include <sstream>
#include "poseidon/flags.h"
#include "poseidon/poseidon.h"

namespace poseidon{
DEFINE_int64(new_zone_size, kDefaultNewZoneSize, "The size of the new zone in bytes.");

DEFINE_int64(old_zone_size, kDefaultOldZoneSize, "The size of the old zone in bytes.");
DEFINE_int64(old_page_size, kDefaultOldPageSize, "The size of the old zone pages in bytes.");

DEFINE_int64(large_object_size, kDefaultLargeObjectSize, "The max size of an object before it gets classified as a large object.");

DEFINE_int64(num_workers, kDefaultNumberOfWorkers, "The number of workers to use for collections.");

    std::string GetVersion(){
        std::stringstream ss;
        ss << POSEIDON_VERSION_MAJOR << ".";
        ss << POSEIDON_VERSION_MINOR << ".";
        ss << POSEIDON_VERSION_PATCH;
        return ss.str();
    }
}