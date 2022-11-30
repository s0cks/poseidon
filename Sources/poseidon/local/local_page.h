#ifndef POSEIDON_LOCAL_PAGE_H
#define POSEIDON_LOCAL_PAGE_H

#include "poseidon/bitset.h"
#include "poseidon/pointer.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class LocalPage : public Section { //TODO: make linked list
   friend class SerialScavenger;
  public:
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
              current_address() < page()->GetEndingAddress();
     }

     Pointer* Next() override {
       auto next = current_ptr();
       current_ += kWordSize;
       return (*next);
     }
   };
  protected:
   uword current_;
   BitSet slots_;

   uword GetLocalAddressAt(const word index) const {
     if(index < 0 || index > GetNumberOfLocals())
       return UNALLOCATED;
     return GetStartingAddress() + (index * kWordSize);
   }

   Pointer** GetLocalAt(const word index) const {
     return (Pointer**) GetLocalAddressAt(index);
   }
  public:
   LocalPage() = default;
   LocalPage(const uword start, const word size):
     Section(start, size),
     current_(start) {

     SetWritable();
     Clear();
   }
   explicit LocalPage(const MemoryRegion& region):
    LocalPage(region.GetStartingAddress(), region.GetSize()) {
   }
   explicit LocalPage(const word size):
    LocalPage(MemoryRegion(CalculateLocalPageSize(size))) {
   }
   LocalPage(const LocalPage& rhs) = default;
   ~LocalPage() override = default;

   uword GetCurrentAddress() const {
     return current_;
   }

   void* GetCurrentAddressPointer() const {
     return (void*)GetCurrentAddress();
   }

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

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitPointers(const std::function<bool(Pointer*)>& vis) override;
   bool VisitNewPointers(RawObjectVisitor* vis);
   bool VisitNewPointers(const std::function<bool(Pointer*)>& vis);
   bool VisitOldPointers(RawObjectVisitor* vis);
   bool VisitOldPointers(const std::function<bool(Pointer*)>& vis);
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(const std::function<bool(Pointer*)>& vis) override;

   LocalPage& operator=(const LocalPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const LocalPage& value) {
     stream << "LocalPage(";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << value.GetNumberOfLocals() << " " << Bytes(value.GetSize()) << ", ";
     stream << ")";
     return stream;
   }
  public:
   static inline constexpr word
   CalculateLocalPageSize(const word num_locals) {
     return num_locals * kWordSize;
   }

   static void SetForCurrentThread(LocalPage* page);
   static LocalPage* GetForCurrentThread();

   static inline bool
   ExistsInCurrentThread() {
     return GetForCurrentThread() != nullptr;
   }

   static inline void
   RemoveFromCurrentThread() {
     LOG(ERROR) << "cannot remove existing LocalPage so just overwriting it";
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     SetForCurrentThread(nullptr);
   }

   static inline void
   InitializeForCurrentThread(const MemoryRegion& region) {
     if(ExistsInCurrentThread())
       RemoveFromCurrentThread();
     return SetForCurrentThread(new LocalPage(region));
   }

   static inline void
   InitializeForCurrentThread(const word size) {
     if(ExistsInCurrentThread())
       RemoveFromCurrentThread();
     return SetForCurrentThread(new LocalPage(size));
   }
 };
}

#endif // POSEIDON_LOCAL_PAGE_H