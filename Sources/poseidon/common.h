#ifndef POSEIDON_COMMON_H
#define POSEIDON_COMMON_H

#include <cstdint>
#include <iostream>

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

 static constexpr const int64_t kB = 1;
 static constexpr const int64_t kKB = kB * 1024;
 static constexpr const int64_t kMB = kKB * 1024;
 static constexpr const int64_t kGB = kMB * 1024;
 static constexpr const int64_t kTB = kGB * 1024;

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

#endif //POSEIDON_COMMON_H
