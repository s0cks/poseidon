#include <sstream>
#include "poseidon/poseidon.h"

namespace poseidon{
  static const uint64_t kB = 1;
  static const uint64_t kKB = 1024 * kB;
  static const uint64_t kMB = 1024 * kKB;
  static const uint64_t kGB = 1024 * kMB;
  static const uint64_t kTB = 1024 * kGB;

DEFINE_uint64(new_zone_size, 16 * kMB, "The size of the new zone in bytes.");
DEFINE_uint64(old_zone_size, 128 * kMB, "The size of the old zone in bytes.");

    std::string GetVersion(){
        std::stringstream ss;
        ss << POSEIDON_VERSION_MAJOR << ".";
        ss << POSEIDON_VERSION_MINOR << ".";
        ss << POSEIDON_VERSION_PATCH;
        return ss.str();
    }
}