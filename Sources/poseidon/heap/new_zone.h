#ifndef POSEIDON_HEAP_NEW_ZONE_H
#define POSEIDON_HEAP_NEW_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/heap/semispace.h"
#include "poseidon/heap/page_table.h"

namespace poseidon{
 class NewZone : public Zone {//TODO: pages?
   template<bool Parallel>
   friend class ScavengerVisitorBase;//TODO: remove

   friend class NewZoneTest;
  protected:
   static inline uword
   GetFromspaceAddress(const MemoryRegion& region) {
     return region.GetStartingAddress();
   }

   static inline uword
   GetTospaceAddress(const MemoryRegion& region) {
     return region.GetStartingAddress() + GetNewZoneSemispaceSize();
   }
  protected:
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

   void InitializePageTable(const MemoryRegion& region, int64_t num_pages, int64_t page_size) override;

   inline MemoryRegion
   GetFromspaceRegion() const {
     return {fromspace(), semisize()};
   }

   inline Semispace
   GetFromspace() const {
     return Semispace(GetFromspaceRegion());
   }

   inline MemoryRegion
   GetTospaceRegion() const {
     return {tospace(), semisize()};
   }

   inline Semispace
   GetTospace() const {
     return Semispace(GetTospaceRegion());
   }

   inline void*
   tospace_ptr() const {
     return (void*)tospace();
   }

   inline void*
   fromspace_ptr() const {
     return (void*)fromspace();
   }
  public:
   NewZone():
     Zone(),
     fromspace_(0),
     tospace_(0),
     semisize_(0) {
   }
   explicit NewZone(const MemoryRegion& region):
    Zone(region, GetNewPageSize()),
    fromspace_(GetFromspaceAddress(region)),
    tospace_(GetTospaceAddress(region)),
    semisize_(GetNewZoneSemispaceSize()) {
     InitializePageTable(region, GetNumberOfNewPages(), GetNewPageSize());
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
     stream << "fromspace=" << val.fromspace_ptr() << ", ";
     stream << "tospace=" << val.tospace_ptr() << ", ";
     stream << "semi-size=" << Bytes(val.semisize());
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_HEAP_NEW_ZONE_H