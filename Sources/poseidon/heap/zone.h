#ifndef POSEIDON_POSEIDON_ZONE_H
#define POSEIDON_POSEIDON_ZONE_H

#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Zone : public Section{
   friend class ZoneTest;
   friend class RawObject;
   friend class Scavenger;
  private:
   class ZoneIterator : public RawObjectPointerIterator{
    private:
     const Zone* zone_;
     uword ptr_;

     /**
      * Returns The current {@link RawObject} pointer for this {@link RawObjectPointerIterator}.
      *
      * @return The current {@link RawObject} pointer for this {@link RawObjectPointerIterator}
      */
     RawObject* current_ptr() const{
       return (RawObject*)ptr_;
     }

     /**
      * Returns The next {@link RawObject} pointer for this {@link RawObjectPointerIterator}.
      *
      * @return The next {@link RawObject} pointer for this {@link RawObjectPointerIterator}
      */
     RawObject* next_ptr() const{
       return (RawObject*)ptr_ + current_ptr()->GetTotalSize();
     }
    public:
     /**
      * An {@link RawObjectPointerIterator} for a {@link Zone}.
      *
      * @param zone The {@link Zone} to iterate
      */
     explicit ZoneIterator(const Zone* zone):
      zone_(zone),
      ptr_(zone->GetStartingAddress()){
     }
     ~ZoneIterator() override = default; // Destructor

     const Zone* zone() const{
       return zone_;
     }

     /**
      * Check for whether or not this {@link RawObjectPointerIterator} has any more {@link RawObject} pointers to visit.
      *
      * @return True if there are more {@link RawObject} pointers to visit, false otherwise
      */
     bool HasNext() const override{
       auto next = next_ptr();
#ifdef PSDN_DEBUG
       assert(zone()->Contains(next->GetAddress()));
#endif//PSDN_DEBUG
       return next->GetPointerSize() > 0;
     }

     /**
      * Returns the next {@link RawObject} pointer in the {@link Zone}.
      *
      * @return The next {@link RawObject} pointer in the {@link Zone}
      */
     RawObject* Next() override{
       auto next = current_ptr();
       ptr_ += next->GetTotalSize();
       return next;
     }
   };
  protected:
   uword current_;
  public:
   Zone():
    Section(),
    current_(0){
   }

   /**
    * Create a {@link Zone} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Zone}
    * @param size The size of the {@link Zone}
    */
   Zone(uword start, int64_t size)://TODO: cleanup?
    Section(start, size),
    current_(start){
   }

   Zone(MemoryRegion* region, int64_t offset, int64_t size):
    Zone(region->GetStartingAddress() + offset, size){
   }

   Zone(MemoryRegion* region, int64_t size):
    Zone(region, 0, size){
   }

   explicit Zone(MemoryRegion* region):
    Zone(region, region->size()){
   }

   Zone(const Zone& rhs) = default;
   ~Zone() override = default;

   uword GetCurrentAddress() const{
     return current_;
   }

   void* GetCurrentAddressPointer() const{
     return (void*)GetCurrentAddress();
   }

   virtual uword TryAllocate(int64_t size);
   void VisitObjectPointers(RawObjectVisitor* vis) const;
   void VisitObjectPointers(const std::function<bool(RawObject*)>& vis) const;
   void VisitMarkedObjectPointers(RawObjectVisitor* vis) const;
   void VisitMarkedObjectPointers(const std::function<bool(RawObject*)>& vis) const;

   Zone& operator=(const Zone& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const Zone& zone){
     stream << "Zone(";
     stream << "starting_address=" << zone.GetStartingAddressPointer() << ", ";
     stream << "total_size=" << Bytes(zone.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif //POSEIDON_POSEIDON_ZONE_H
