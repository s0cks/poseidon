#ifndef POSEIDON_REGION_H
#define POSEIDON_REGION_H

#include "poseidon/platform/platform.h"

namespace poseidon {
 class Region {
  public:
   static inline int
   Compare(const Region& lhs, const Region& rhs) {
     if(lhs.GetStartingAddress() < rhs.GetStartingAddress())
       return -1;
     else if(lhs.GetStartingAddress() > rhs.GetStartingAddress())
       return +1;

     if(lhs.GetSize() < rhs.GetSize())
       return -1;
     else if(lhs.GetSize() > rhs.GetSize())
       return +1;
     return 0;
   }
  protected:
   Region() = default;
  public:
   virtual ~Region() = default;
   virtual uword GetStartingAddress() const = 0;

   virtual void* GetStartingAddressPointer() const {
     return (void*)GetStartingAddress();
   }

   virtual uword GetEndingAddress() const {
     return GetStartingAddress() + GetSize();
   }

   virtual void* GetEndingAddressPointer() const {
     return (void*)GetEndingAddress();
   }

   virtual int64_t GetSize() const = 0;

   virtual bool Contains(const uword address) const {
     return GetStartingAddress() <= address
         && GetEndingAddress() >= address;
   }

   virtual bool Intersects(const Region& rhs) const {
     return Contains(rhs.GetStartingAddress()) || Contains(rhs.GetEndingAddress());
   }

   friend bool operator==(const Region& lhs, const Region& rhs) {
     return Compare(lhs, rhs) == 0;
   }

   friend bool operator!=(const Region& lhs, const Region& rhs) {
     return Compare(lhs, rhs) != 0;
   }

   friend bool operator<(const Region& lhs, const Region& rhs) {
     return Compare(lhs, rhs) < 0;
   }
 };
}

#endif // POSEIDON_REGION_H