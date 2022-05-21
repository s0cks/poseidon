#ifndef POSEIDON_NEW_ZONE_H
#define POSEIDON_NEW_ZONE_H

#include "poseidon/heap/zone.h"

namespace poseidon{
 class NewZone : public Zone{//TODO: pages?
   template<bool Parallel>
   friend class ScavengerVisitorBase;//TODO: remove
  public:
   static inline int64_t
   CalculateSemispaceSize(int64_t zone_size){
     return zone_size / 2;
   }
  private:
   uword fromspace_;
   uword tospace_;
   int64_t semisize_;

   /**
    * Swaps the from_ & to_ {@link Semispace}s in this {@link Zone}.
    *
    * Called during collection time.
    */
   virtual void SwapSpaces(){
     std::swap(fromspace_, tospace_);
     current_ = tospace_;
   }
  public:
   NewZone():
     Zone(),
     fromspace_(0),
     tospace_(0),
     semisize_(0){
   }

   NewZone(uword start, int64_t size):
     Zone(start, size),
     fromspace_(start),
     tospace_(start + CalculateSemispaceSize(size)),
     semisize_(CalculateSemispaceSize(size)){
   }

   NewZone(MemoryRegion* region, int64_t offset, int64_t size):
     NewZone(region->GetStartingAddress() + offset, size){//TODO: check MemoryRegion bounds
   }

   NewZone(MemoryRegion* region, int64_t size):
     NewZone(region, 0, size){
   }

   explicit NewZone(MemoryRegion* region):
     NewZone(region, region->size()){
   }

   NewZone(const NewZone& rhs) = default;
   ~NewZone() override = default;

   uword tospace() const{
     return tospace_;
   }

   uword fromspace() const{
     return fromspace_;
   }

   int64_t semisize() const{
     return semisize_;
   }

   /**
    * Allocates a new object of size bytes in the from_ Semispace of this Zone.
    *
    * @param size The size of the new object to allocate
    * @return A pointer to the beginning of the object and i's header
    */
   uword TryAllocate(int64_t size) override;

   NewZone& operator=(const NewZone& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const NewZone& val){
     stream << "NewZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "allocated=" << PrettyPrintPercentage(val.GetAllocatedPercentage()) << ", ";
     stream << "fromspace=" << val.fromspace() << ", ";
     stream << "tospace=" << val.tospace() << ", ";
     stream << "semi-size=" << val.semisize();
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_NEW_ZONE_H