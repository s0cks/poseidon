#ifndef POSEIDON_HEAP_NEW_ZONE_H
#define POSEIDON_HEAP_NEW_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 class NewZone : public Zone<NewPage> { //TODO: make resizable
   template<bool Parallel>
   friend class ScavengerVisitorBase;//TODO: remove

   template<class P>
   friend class Zone;

   friend class Heap;
   friend class NewZoneTest;
  public:
   class NewZoneIterator : public ZoneIterator<NewZone> {
    public:
     explicit NewZoneIterator(NewZone* zone):
       ZoneIterator<NewZone>(zone) {
     }
     ~NewZoneIterator() override = default;

     bool HasNext() const override {
       return current_address() > 0 &&
              current_address() < zone()->GetEndingAddress() &&
              current_ptr()->IsNew() &&
              current_ptr()->GetSize() > 0;
     }
   };

   class NewZonePageIterator : public ZonePageIterator{
    public:
     explicit NewZonePageIterator(Zone<NewPage>* zone):
       ZonePageIterator(zone) {
     }
     ~NewZonePageIterator() override = default;
   };
  protected:
   int64_t semisize_; //TODO: remove
   Semispace fromspace_;
   Semispace tospace_;

   NewZone(const uword start_address, const int64_t size, const int64_t semi_size):
    Zone<NewPage>(start_address, size, GetNewPageSize()),
    fromspace_(GetStartingAddress(), semi_size),
    tospace_(GetStartingAddress() + semi_size, semi_size),
    semisize_(semi_size) {
   }
  public:
   NewZone() = delete;
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

   virtual uword TryAllocateBytes(ObjectSize size);

   virtual uword TryAllocate(const ObjectSize size) {
     return TryAllocateBytes(size); //TODO: remove
   }

   bool VisitPages(NewPageVisitor* vis) {
     NewZonePageIterator iter(this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(!vis->Visit(next))
         return false;
     }
     return true;
   }

   bool VisitMarkedPages(NewPageVisitor* vis) {
     NewZonePageIterator iter(this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(IsMarked(next) && !vis->Visit(next))
         return false;
     }
     return true;
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     return IteratePointers<NewZone, NewZoneIterator, RawObjectVisitor>(this, vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<NewZone, NewZoneIterator, RawObjectVisitor>(this, vis);
   }

   virtual void SwapSpaces();
   virtual bool MarkAllIntersectedBy(const Region& region);

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
 };
}

#endif//POSEIDON_HEAP_NEW_ZONE_H