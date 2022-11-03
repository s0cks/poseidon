#ifndef POSEIDON_LOCAL_PAGE_H
#define POSEIDON_LOCAL_PAGE_H

#include "poseidon/bitset.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class LocalPage : public Region {
  protected:
   uword start_;
   int64_t size_;
   BitSet slots_;

   uword GetLocalAddressAt(const int64_t index) const {
     if(index < 0 || index > GetNumberOfLocals())
       return UNALLOCATED;
     return GetStartingAddress() + (index * kWordSize);
   }

   RawObject** GetLocalAt(const int64_t index) const {
     return (RawObject**) GetLocalAddressAt(index);
   }
  public:
   LocalPage() = default;
   LocalPage(const uword start, const int64_t size):
     Region(),
     start_(start),
     size_(size) {
   }
   explicit LocalPage(const MemoryRegion& region):
     LocalPage(region.GetStartingAddress(), region.GetSize()) {
   }
   LocalPage(const LocalPage& rhs) = default;
   ~LocalPage() override = default;

   uword GetStartingAddress() const override {
     return start_;
   }

   int64_t GetSize() const override {
     return size_;
   }

   int64_t GetNumberOfLocals() const {
     return GetSize() / kWordSize;
   }

   RawObject* GetLocal(const int64_t index) const {
     return *GetLocalAt(index);
   }

   void SetLocal(const int64_t index, RawObject* ptr) {
     *GetLocalAt(index) = ptr;
   }

   LocalPage& operator=(const LocalPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const LocalPage& value) {
     stream << "LocalPage(";
     stream << ")";
     return stream;
   }
  public:
   static inline constexpr int64_t
   CalculateLocalPageSize(const int64_t num_locals) {
     return num_locals * kWordSize;
   }
 };
}

#endif // POSEIDON_LOCAL_PAGE_H