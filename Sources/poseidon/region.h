#ifndef POSEIDON_REGION_H
#define POSEIDON_REGION_H

#include "poseidon/platform/platform.h"
#include "poseidon/common.h"

namespace poseidon {
 typedef word RegionSize;

 class Region {
  protected:
   uword start_;
   RegionSize size_;
  public:
   Region() = default;
   Region(const uword start, const RegionSize size):
    start_(start),
    size_(size) {
   }
   Region(const Region& rhs) = default;
   virtual ~Region() = default;

   virtual uword GetStartingAddress() const {
     return start_;
   }

   void* GetStartingAddressPointer() const {
     return (void*) GetStartingAddress();
   }

   uword GetEndingAddress() const {
     return GetStartingAddress() + GetSize();
   }

   void* GetEndingAddressPointer() const {
     return (void*) GetEndingAddress();
   }

   virtual RegionSize GetSize() const {
     return size_;
   }

   virtual bool Contains(const uword address) const {
     return GetStartingAddress() <= address &&
            GetEndingAddress() >= address;
   }

   virtual bool Contains(const uword start, const RegionSize size) const {
     return Contains(start) && Contains(start + size);
   }

   virtual inline bool Contains(const Region* region) const {
     return Contains(region->GetStartingAddress(), region->GetSize());
   }

   virtual inline bool Contains(const Region& region) const {
     return Contains(&region);
   }

   virtual bool Intersects(const uword start, const RegionSize size) const {
     return Contains(start) || Contains(start + size);
   }

   virtual inline bool Intersects(const Region* rhs) const {
     return Intersects(rhs->GetStartingAddress(), rhs->GetSize());
   }

   virtual inline bool Intersects(const Region& rhs) const {
     return Intersects(&rhs);
   }

   Region& operator=(const Region& rhs) {
     if(*this == rhs)
       return *this;
     start_ = rhs.start_;
     size_ = rhs.size_;
     return *this;
   }

   friend Region operator+(const Region& lhs, const RegionSize& rhs) {
     return { lhs.GetStartingAddress() + (lhs.GetSize() * rhs), lhs.GetSize() };
   }

   friend Region operator-(const Region& lhs, const RegionSize& rhs) {
     return { lhs.GetStartingAddress() - (lhs.GetSize() * rhs), lhs.GetSize() };
   }

   friend std::ostream& operator<<(std::ostream& stream, const Region& region) {
     stream << "Region(";
     stream << "start=" << region.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(region.GetSize());
     stream << ")";
     return stream;
   }

   friend bool operator==(const Region& lhs, const Region& rhs) {
     return lhs.GetStartingAddress() == rhs.GetStartingAddress() &&
            lhs.GetSize() == rhs.GetSize();
   }

   friend bool operator!=(const Region& lhs, const Region& rhs) {
     return !operator==(lhs, rhs);
   }
  public:
   static inline Region
   Subregion(const Region& parent, const RegionSize offset, const RegionSize size) {
     return { parent.GetStartingAddress() + offset, size };
   }

   static inline Region
   Subregion(const Region& parent, const RegionSize size) {
     return Subregion(parent, 0, size);
   }
 };
}

#endif // POSEIDON_REGION_H