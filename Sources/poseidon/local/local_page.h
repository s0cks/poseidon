#ifndef POSEIDON_LOCAL_PAGE_H
#define POSEIDON_LOCAL_PAGE_H

#include "poseidon/bitset.h"
#include "poseidon/raw_object.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class LocalPage : public Region {
  public:
   class LocalPageIterator {
    private:
     const LocalPage* page_;
     int64_t current_;

     inline const LocalPage* page() const {
       return page_;
     }

     inline int64_t current() const {
       return current_;
     }

     inline int64_t total() const {
       return page()->GetNumberOfLocals();
     }
    public:
     explicit LocalPageIterator(const LocalPage* page):
      page_(page),
      current_(0) {
     }
     ~LocalPageIterator() = default;

     bool HasNext() const {
       return current() < total();
     }

     RawObject* Next() {
       auto next = page()->GetLocal(current());
       current_ += 1;
       return next;
     }
   };
  protected:
   uword start_;
   uword current_;
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
     current_(start),
     size_(size) {
     Clear();
   }
   explicit LocalPage(const MemoryRegion& region):
     LocalPage(region.GetStartingAddress(), region.GetSize()) {
   }
   LocalPage(const LocalPage& rhs) = default;
   ~LocalPage() override = default;

   uword GetStartingAddress() const override {
     return start_;
   }

   uword GetCurrentAddress() const {
     return current_;
   }

   void* GetCurrentAddressPointer() const {
     return (void*)GetCurrentAddress();
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

   uword TryAllocate();
   bool VisitPointers(RawObjectVisitor* vis);
   bool VisitMarkedPointers(RawObjectVisitor* vis);

   void Clear() {
     memset(GetStartingAddressPointer(), 0, GetSize());
   }

   LocalPage& operator=(const LocalPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const LocalPage& value) {
     stream << "LocalPage(";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << value.GetNumberOfLocals() << " " << Bytes(value.GetSize()) << ", ";
     stream << ")";
     return stream;
   }
  public:
   static inline constexpr int64_t
   CalculateLocalPageSize(const int64_t num_locals) {
     return num_locals * kWordSize;
   }

   static void Initialize();
   static LocalPage* GetForCurrentThread();
   static void SetForCurrentThread(LocalPage* page);
 };
}

#endif // POSEIDON_LOCAL_PAGE_H