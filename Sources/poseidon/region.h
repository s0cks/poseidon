#ifndef POSEIDON_REGION_H
#define POSEIDON_REGION_H

#include "poseidon/platform/platform.h"

namespace poseidon {
 class Region {
  protected:
   Region() = default;
  public:
   virtual ~Region() = default;
   virtual uword GetStartingAddress() const = 0;
   virtual int64_t GetSize() const = 0;

   virtual void* GetStartingAddressPointer() const {
     return (void*)GetStartingAddress();
   }

   virtual uword GetEndingAddress() const {
     return GetStartingAddress() + GetSize();
   }

   virtual void* GetEndingAddressPointer() const {
     return (void*)GetEndingAddress();
   }

   virtual bool Contains(const uword address) const {
     return GetStartingAddress() <= address
         && GetEndingAddress() >= address;
   }

   virtual bool Contains(const Region& rhs) const {
     return Contains(rhs.GetStartingAddress()) || Contains(rhs.GetEndingAddress());
   }
 };
}

#endif // POSEIDON_REGION_H