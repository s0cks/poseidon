#ifndef POSEIDON_HEAP_ZONE_H
#define POSEIDON_HEAP_ZONE_H

#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Zone : public AllocationSection{
   friend class ZoneTest;
   friend class RawObject;
   friend class Scavenger;
  protected:
   MemoryRegion region_;
   RelaxedAtomic<MemoryRegion::ProtectionMode> mode_;

   inline MemoryRegion::ProtectionMode
   GetMode() const{
     return (MemoryRegion::ProtectionMode)mode_;
   }

   inline void
   SetMode(const MemoryRegion::ProtectionMode& mode){
     if(!region_.Protect(mode)){
       LOG(FATAL) << "failed to set " << (*this) << " to " << mode;
       return;
     }
     DLOG(INFO) << "set " << (*this) << " to " << mode;
     mode_ = mode;
   }

   inline void
   SetWriteable(){
     if(IsWritable()){
       DLOG(WARNING) << (*this) << " is already writeable.";
       return;
     }
     return SetMode(MemoryRegion::kReadWrite);
   }

   inline void
   SetReadOnly(){
     if(IsReadOnly()){
       DLOG(WARNING) << (*this) << " is already read-only.";
       return;
     }
     return SetMode(MemoryRegion::kReadOnly);
   }
  public:
   Zone():
    AllocationSection(),
    region_(),
    mode_(MemoryRegion::kReadOnly){
   }

   /**
    * Create a {@link Zone} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Zone}
    * @param size The size of the {@link Zone}
    */
   Zone(uword start, int64_t size)://TODO: cleanup?
    AllocationSection(start, size),
    region_(start, size),
    mode_(MemoryRegion::kReadOnly){
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

   bool IsReadOnly() const{
     return GetMode() == MemoryRegion::kReadOnly;
   }

   bool IsWritable() const{
     return GetMode() == MemoryRegion::kReadWrite;
   }

   Zone& operator=(const Zone& rhs){
     if(*this == rhs)
       return *this;
     AllocationSection::operator=(rhs);
     region_ = rhs.region_;
     mode_ = rhs.GetMode();
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Zone& zone){
     stream << "Zone(";
     stream << "starting_address=" << zone.GetStartingAddressPointer() << ", ";
     stream << "total_size=" << Bytes(zone.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif //POSEIDON_HEAP_ZONE_H
