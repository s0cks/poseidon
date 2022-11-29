#ifndef POSEIDON_HEAP_NEW_ZONE_H
#define POSEIDON_HEAP_NEW_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/zone/zone.h"
#include "poseidon/heap/page/new_page.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 class NewZone : public Zone { //TODO: make resizable
   template<bool Parallel>
   friend class ScavengerVisitorBase;//TODO: remove

   friend class Zone;

   friend class Heap;
   friend class NewZoneTest;
  public:
   class NewZoneIterator : public ZonePointerIterator {
    public:
     explicit NewZoneIterator(const NewZone* zone):
       ZonePointerIterator(zone) {
     }
     ~NewZoneIterator() override = default;

     bool HasNext() const override {
       return zone()->Contains(current_address()) &&
              current_ptr()->IsNew() &&
              !current_ptr()->IsFree() &&
              current_ptr()->GetSize() > 0;
     }
   };
  protected:
   int64_t semisize_; //TODO: remove
   Semispace fromspace_;
   Semispace tospace_;

   NewZone(const uword start_address, const int64_t size, const int64_t semi_size):
    Zone(start_address, size, flags::GetNewPageSize()),
    fromspace_(GetStartingAddress(), semi_size),
    tospace_(GetStartingAddress() + semi_size, semi_size),
    semisize_(semi_size) {
   }
  public:
   NewZone() = delete;
   explicit NewZone(const MemoryRegion& region, const int64_t semi_size = flags::GetNewZoneSemispaceSize()):
    NewZone(region.GetStartingAddress(), region.GetSize(), semi_size) {
   }
   explicit NewZone(const int64_t size, const int64_t semi_size = flags::GetNewZoneSemispaceSize()):
    NewZone(MemoryRegion(size), semi_size) {
   }
   NewZone(const NewZone& rhs) = delete;
   ~NewZone() override = default;

   Semispace fromspace() const {
     return fromspace_;
   }

   Semispace& fromspace() {
     return fromspace_;
   }

   Semispace tospace() const {
     return tospace_;
   }

   Semispace& tospace() {
     return tospace_;
   }

   int64_t semisize() const{
     return semisize_;
   }

   virtual void SwapSpaces();

   virtual Pointer* TryAllocatePointer(word size);
   uword TryAllocateBytes(word size) override;
   uword TryAllocateClassBytes(Class* cls) override;

   template<typename T>
   T* TryAllocate() {
     return (T*) TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClass() {
     return (T*) TryAllocateClassBytes(T::GetClass());
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     return IteratePointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitPointers(const std::function<bool(Pointer*)>& vis) override {
     return IteratePointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitMarkedPointers(const std::function<bool(Pointer*)>& vis) override {
     return IterateMarkedPointers<NewZone, NewZoneIterator>(vis);
   }

   friend std::ostream& operator<<(std::ostream& stream, const NewZone& val){
     stream << "NewZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer() << ", ";
     stream << "fromspace=" << val.fromspace() << ", ";
     stream << "tospace=" << val.tospace()  << ", ";
     stream << "semi-size=" << Bytes(val.semisize());
     stream << ")";
     return stream;
   }
  public:
   static NewZone* New(const MemoryRegion& region);

   static constexpr ObjectSize
   GetMinimumObjectSize() {
     return kWordSize;
   }

   static constexpr ObjectSize
   GetMaximumObjectSize() {
     return Semispace::GetMaximumObjectSize();
   }
 };
}

#endif//POSEIDON_HEAP_NEW_ZONE_H