#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "poseidon/zone.h"
#include "poseidon/flags.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 class Heap{
  public:
   static inline uint64_t
   GetNewZoneSize(){
     return FLAGS_new_zone_size;
   }

   static inline uint64_t
   GetOldZoneSize() {
     return FLAGS_old_zone_size;
   }

   static inline uint64_t
   GetTotalHeapSize(){
     return GetNewZoneSize() + GetOldZoneSize();
   }
  private:
   MemoryRegion region_;
   Zone new_zone_;
   Zone old_zone_;
  public:
   Heap():
    region_(GetTotalHeapSize()),
    new_zone_(region_, 0, GetNewZoneSize()),
    old_zone_(region_, GetNewZoneSize(), GetOldZoneSize()){
     if(!region_.Protect(MemoryRegion::kReadWrite))
       LOG(ERROR) << "cannot protect Heap MemoryRegion.";
   }
   Heap(const Heap& rhs) = delete;
   ~Heap() = default;

   MemoryRegion region() const{
     return region_;
   }

   uword GetStartingAddress() const{
     return region_.GetStartAddress();
   }

   void* GetStartingAddressPointer() const{
     return (void*)GetStartingAddress();
   }

   uword GetEndingAddress() const{
     return region_.GetEndAddress();
   }

   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   Zone new_zone() const{
     return new_zone_;
   }

   Zone old_zone() const{
     return old_zone_;
   }

   RawObject* AllocateNewObject(uint64_t size){
     auto val = new_zone_.AllocateRawObject(size);
     val->SetNewBit();
     return val;
   }

   RawObject* AllocateOldObject(uint64_t size){
     auto val = old_zone_.AllocateRawObject(size);
     val->SetOldBit();
     return val;
   }

   bool Contains(uword address) const{
     return region_.Contains(address);
   }

   void clear(){
     new_zone_.ClearZone();
     old_zone_.ClearZone();
   }

   Heap& operator=(const Heap& rhs) = delete;

   friend std::ostream& operator<<(std::ostream& stream, const Heap& heap){//TODO: implement
     return stream;
   }
 };

 class HeapPrinter{
  public:
   enum Flags : uint64_t{
     kNone = 0,
     kDetailed = 1 << 1,
     kHexDump = 1 << 2,

#ifdef PSDN_DEBUG
     kDefault = kDetailed | kHexDump,
#else
     kDefault = kNone,
#endif//PSDN_DEBUG
   };

   HeapPrinter() = delete;
   ~HeapPrinter() = delete;

   static void Print(Heap* heap, uint64_t flags = kDefault);
 };
}

#endif //POSEIDON_HEAP_H
