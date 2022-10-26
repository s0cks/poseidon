#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 void NewZone::InitializePageTable(const MemoryRegion& region, int64_t num_pages, int64_t page_size) {
   for(auto idx = 0; idx < num_pages; idx++) {
     const auto pageOffset = (idx * page_size);
     const auto pageRegion = MemoryRegion::Subregion(region, pageOffset, page_size);
     const auto page = new NewPage(idx, pageRegion);
     LOG(INFO) << "creating page: " << (*page);
     PutPage(page);
   }
 }

 uword NewZone::TryAllocate(int64_t size){
   LOG(INFO) << "allocating " << Bytes(size) << " in " << (*this);
   auto total_size = static_cast<int64_t>(sizeof(RawObject)) + size;
   if((GetCurrentAddress() + total_size) >= (fromspace() + tospace())){
     PSDN_CANT_ALLOCATE(ERROR, total_size, (*this));
     Collector::MinorCollection();
   }

   if((GetCurrentAddress() + total_size) >= (fromspace() + tospace())){
     PSDN_CANT_ALLOCATE(FATAL, total_size, (*this));
   }

   auto ptr = new (GetCurrentAddressPointer())RawObject(ObjectTag::NewWithSize(size));
   current_ += total_size;
   MarkAll(ptr);
   return ptr->GetStartingAddress();
 }
}