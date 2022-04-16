#ifndef POSEIDON_COMMON_H
#define POSEIDON_COMMON_H

#include <cstdint>
#include <cassert>
#include <iostream>

#include "platform.h"

#define NOT_IMPLEMENTED(Level) \
  LOG(Level) << __FUNCTION__ << " is not implemented!"

namespace poseidon{
 static inline uword
 RoundUpPow2(uword x){
   x = x - 1;
   x = x | (x >> 1);
   x = x | (x >> 2);
   x = x | (x >> 4);
   x = x | (x >> 8);
   x = x | (x >> 16);
#ifdef ARCHITECTURE_IS_X64
   x = x | (x >> 32);
#endif
   return x + 1;
 }

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
    GCLOG(3) << "starting " << (Name) << "...."; \
    auto start_ts = Clock::now();    \
    Section;                         \
    auto finish_ts = Clock::now();   \
    LOG(INFO) << (Name) << " finished in " << (finish_ts - start_ts) << "."; \
  } while(0);

#ifdef PSDN_DEBUG

#define DTIMED_SECTION(Name, Section) \
 do {                                 \
    GCLOG(1) << "starting " << (Name) << "...."; \
    auto start_ts = Clock::now();     \
    Section;                          \
    auto finish_ts = Clock::now();    \
    GCLOG(1) << (Name) << " finished in " << (finish_ts - start_ts) << "."; \
 } while(0);

#else

#define DTIMED_SECTION(Name, Section) \
 do {                                 \
  Section;                            \
 } while(0);

#endif

}

#endif //POSEIDON_COMMON_H
