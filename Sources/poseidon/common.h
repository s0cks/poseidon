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
}

#endif //POSEIDON_COMMON_H
