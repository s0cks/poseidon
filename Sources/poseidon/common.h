#ifndef POSEIDON_COMMON_H
#define POSEIDON_COMMON_H

#include <cstdint>
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

  class AllocationSection{
   protected:
    AllocationSection() = default;
   public:
    virtual ~AllocationSection() = default;

    virtual int64_t size() const = 0;
    virtual uword Allocate(int64_t size) = 0;
    virtual uword GetStartingAddress() const = 0;

    virtual void* GetStartingAddressPointer() const{
      return (void*)GetStartingAddress();
    }

    virtual uword GetEndingAddress() const{
      return GetStartingAddress() + size();
    }

    virtual void* GetEndingAddressPointer() const{
      return (void*)GetEndingAddress();
    }

    virtual bool Contains(uword address) const{
      return GetStartingAddress() <= address
          && GetEndingAddress() >= address;
    }

    virtual void Clear(){
      memset(GetStartingAddressPointer(), 0, size());
    }
  };

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
}

#endif //POSEIDON_COMMON_H
