#ifndef POSEIDON_COMMON_H
#define POSEIDON_COMMON_H

#include <cstdint>
#include <iostream>
#include <glog/logging.h>

#include "poseidon/utils/size.h"

//TODO: cleanup

#define NOT_IMPLEMENTED(Level) \
  LOG(Level) << __FUNCTION__ << " is not implemented!"

namespace poseidon{
#ifdef PSDN_DEBUG
#define PSDN_ASSERT(x) assert(x)
#else
#define PSDN_ASSERT(x)
#endif//PSDN_DEBUG

#ifdef PSDN_GCLOGS

 // collector logs are enabled:
#define GCLOG(Level) \
  DLOG(INFO)

#else

// collector logs are disabled:

#define GCLOG(Level) \
  DVLOG(Level)

#endif//PSDN_GCLOGS

#define TIMED_SECTION(Name, Section) \
  do {                               \
    auto start_ts = Clock::now();    \
    Section;                         \
    auto finish_ts = Clock::now();   \
    LOG(INFO) << (Name) << " finished in " << (finish_ts - start_ts) << "."; \
  } while(0);

#ifdef PSDN_DEBUG

#define DTIMED_SECTION(Name, Section) \
 do {                                 \
    auto start_ts = Clock::now();     \
    Section;                          \
    auto finish_ts = Clock::now();    \
    DLOG(INFO) << (Name) << " finished in " << (finish_ts - start_ts) << "."; \
 } while(0);

#else

#define DTIMED_SECTION(Name, Section) \
 do {                                 \
  Section;                            \
 } while(0);

#endif

#define PSDN_CANT_ALLOCATE(Level, TotalSize, Section) \
  LOG(Level) << "cannot allocate " << Bytes((TotalSize)) << " in " << (Section) << ".";
}

#ifndef UNALLOCATED
#define UNALLOCATED 0
#endif // UNALLOCATED

namespace poseidon {
 template<class Zone, const google::LogSeverity Severity=google::FATAL>
 static inline void
 CannotAllocate(Zone* zone, const ObjectSize object_size) {
   LOG_AT_LEVEL(Severity) << "cannot allocate " << Bytes(object_size) << " in " << (*zone) << ".";
 }
}

#define DEFINE_NON_COPYABLE_TYPE(Type) \
  public:                              \
    Type(const Type& rhs) = delete;    \
    Type& operator=(const Type& rhs) = delete;

#endif //POSEIDON_COMMON_H
