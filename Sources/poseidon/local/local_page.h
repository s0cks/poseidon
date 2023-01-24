#ifndef POSEIDON_LOCAL_PAGE_H
#define POSEIDON_LOCAL_PAGE_H

#include "poseidon/bitset.h"
#include "poseidon/pointer.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class LocalPage : public AllocationSection { //TODO: make linked list
   friend class SerialScavenger;
  public:
   static constexpr const word kMinimumSize = 1;
   static constexpr const word kDefaultSize = 32;
   static constexpr const word kMaximumSize = 65535;

   class LocalPageIterator : public RawObjectPointerIterator {
    protected:
     const LocalPage* page_;
     uword current_;

     inline const LocalPage* page() const {
       return page_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline Pointer** current_ptr() const {
       return (Pointer**)current_address();
     }
    public:
     explicit LocalPageIterator(const LocalPage* page):
      RawObjectPointerIterator(),
      page_(page),
      current_(page->GetStartingAddress()) {
     }
     ~LocalPageIterator() override = default;

     bool HasNext() const override {
       return current_address() >= page()->GetStartingAddress() &&
              current_address() < page()->GetCurrentAddress();
     }

     Pointer* Next() override {
       auto next = current_ptr();
       current_ += kWordSize;
       return (*next);
     }
   };

   static inline Region
   CalculateLocalPageRegion(const uword start, const word num_locals) {
     static constexpr const auto kLocalPageSize = static_cast<RegionSize>(sizeof(LocalPage));
     return { start, CalculateLocalPageSize(num_locals) + kLocalPageSize };
   }

   static inline Region
   CalculateLocalPageDataRegion(const Region& region, const word num_locals) {
     static constexpr const auto kLocalPageSize = static_cast<RegionSize>(sizeof(LocalPage));
     return Region::Subregion(region, kLocalPageSize, CalculateLocalPageSize(num_locals));
   }
  protected:
   BitSet slots_;

   explicit LocalPage(const word num_locals):
    AllocationSection(CalculateLocalPageDataRegion(CalculateLocalPageRegion((uword) this, num_locals), num_locals)),
    slots_(num_locals) {
   }

   uword GetLocalAddressAt(const word index) const {
     if(index < 0 || index > GetNumberOfLocals())
       return UNALLOCATED;
     return GetStartingAddress() + (index * kWordSize);
   }

   Pointer** GetLocalAt(const word index) const {
     return (Pointer**)GetLocalAddressAt(index);
   }
  public:
   LocalPage() = delete;
   LocalPage(const LocalPage& rhs) = delete;
   ~LocalPage() override = default;

   word GetNumberOfLocals() const {
     return GetSize() / kWordSize;
   }

   Pointer* GetLocal(const word index) const {
     if(index < 0 || index > GetNumberOfLocals())
       return UNALLOCATED;
     return *GetLocalAt(index);
   }

   void SetLocal(const word index, Pointer* ptr) {
     if(index < 0 || index > GetNumberOfLocals())
       return;
     *GetLocalAt(index) = ptr;
   }

   uword TryAllocate();

   inline bool
   IsInitialized() const {
     return GetSize() > 0 && GetCurrentAddress() > 0;
   }

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitPointers(const std::function<bool(Pointer*)>& vis) override;
   bool VisitNewPointers(RawObjectVisitor* vis);
   bool VisitNewPointers(const std::function<bool(Pointer*)>& vis);
   bool VisitOldPointers(RawObjectVisitor* vis);
   bool VisitOldPointers(const std::function<bool(Pointer*)>& vis);
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(const std::function<bool(Pointer*)>& vis) override;

   LocalPage& operator=(const LocalPage& rhs) = delete;

   friend std::ostream& operator<<(std::ostream& stream, const LocalPage& value) {
     stream << "LocalPage(";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(value.GetSize()) << ", ";
     stream << "num_locals=" << value.GetNumberOfLocals() << ", ";
     stream << "end=" << value.GetEndingAddressPointer();
     stream << ")";
     return stream;
   }
  public:
   static inline constexpr ObjectSize
   CalculateLocalPageSize(const word num_locals) {
     return static_cast<ObjectSize>(sizeof(LocalPage)) + (num_locals * kWordSize);
   }
  public:
   void* operator new(size_t sz, word num_locals) noexcept {
     PSDN_ASSERT(sz == sizeof(LocalPage));
     MemoryRegion region(CalculateLocalPageSize(num_locals));
     LOG_IF(FATAL, !region.Protect(MemoryRegion::kReadWrite)) << "cannot protect " << region << " for new local page w/ " << num_locals << " locals";
     region.ClearRegion();
     return region.GetStartingAddressPointer();
   }

   void operator delete(void* ptr) noexcept {
     MemoryRegion region((uword) ptr, 1); //TODO: fix this
     region.FreeRegion();
   }

   static inline LocalPage*
   New(const word num_locals = kDefaultSize) {
     return new (num_locals)LocalPage(num_locals);
   }
 };

 LocalPage* GetLocalPageForCurrentThread();

 inline bool LocalPageExistsForCurrentThread() {
   return GetLocalPageForCurrentThread() != nullptr;
 }

 void SetLocalPageForCurrentThread(LocalPage* page);

 inline void
 RemoveLocalPageForCurrentThread() {
   return SetLocalPageForCurrentThread(nullptr);
 }
}

#endif // POSEIDON_LOCAL_PAGE_H