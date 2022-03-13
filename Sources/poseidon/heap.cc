#include <glog/logging.h>

#include "heap.h"
#include "utils.h"
#include "raw_object.h"

namespace poseidon{
 pthread_key_t Heap::kThreadKey = PTHREAD_KEYS_MAX;

 RawObject* Heap::AllocateNewObject(uint64_t size){
   RawObject* val = nullptr;
   if((val = new_zone()->AllocateRawObject(size)) != nullptr)
     goto finish_allocation;

   DLOG(WARNING) << "couldn't allocate new object of " << HumanReadableSize(size) << ".";
   //TODO: free memory from new_zone()

   if((val = new_zone()->AllocateRawObject(size)) != nullptr)
     goto finish_allocation;

   LOG(FATAL) << "cannot allocate new object of " << HumanReadableSize(size) << "!";
   return nullptr;

finish_allocation:
   val->SetNewBit();
   return val;
 }

 RawObject* Heap::AllocateOldObject(uint64_t size){
   RawObject* val = nullptr;

   // 1. Try Allocation
   if((val = old_zone()->AllocateRawObject(size)) != nullptr)
     goto finish_allocation;

   // 2. Try Major Collection
   //TODO: free memory from old_zone

   // 3. Try Allocation Again
   if((val = old_zone()->AllocateRawObject(size)) != nullptr)
     goto finish_allocation;

   // 4. Try Pages w/ Grow
   {
     auto page = GetCurrentPage();
     while(page != nullptr){
       if((val = page->Allocate(size)) != nullptr)
         goto finish_allocation;
       page = page->next();
     }

     page = CreateNewHeapPage(page);
     if((val = page->Allocate(size)) != nullptr)
       goto finish_allocation;
   }

   // 5. Crash
   LOG(FATAL) << "cannot allocate " << HumanReadableSize(size) << " in heap.";
   return nullptr;

finish_allocation:
   val->SetNewBit();
   return val;
 }

 RawObject* Heap::AllocateLargeObject(uint64_t size){
   return AllocateOldObject(size);//TODO: refactor
 }

 RawObject* Heap::AllocateObject(uint64_t size){
   if(size < kWordSize)
     size = kWordSize;

   if(size >= GetLargeObjectSize()){
     DLOG(INFO) << "allocating large object of " << HumanReadableSize(size);
     return AllocateLargeObject(size);
   }
   return AllocateNewObject(size);
 }

 RawObject* HeapPage::Allocate(uint64_t size){
   uint64_t total_size = sizeof(RawObject) + size;
   if((current_ + total_size) > GetEndingAddress()){
     DLOG(WARNING) << "cannot allocate object of size " << HumanReadableSize(size) << " in " << HumanReadableSize(region_.GetSize()) << " heap page.";
     return nullptr;
   }

   uword paddress = current_;
   current_ += total_size;
   void* ptr = (void*)paddress;
   memset(ptr, 0, total_size);
   auto raw = new (ptr)RawObject();
   raw->SetPointerSize(size);
   raw->SetOldBit();
   return raw;
 }

 static inline bool
 HasFlag(uint64_t flags, const HeapPrinter::Flags flag){
   return (flags & flag) == flag;
 }

 static inline bool
 HasDetailedFlag(uint64_t flags){
   return HasFlag(flags, HeapPrinter::kDetailed);
 }

 static inline bool
 HasHexDumpFlag(uint64_t flags){
   return HasFlag(flags, HeapPrinter::kHexDump);
 }

 static const uint64_t kDefaultFormattedHexDumpSize = 128;
 static const uint64_t kDefaultPaddingSize = 1;

 static inline std::string
 CreateHeader(const std::string& title,
              uint64_t max_length = kDefaultFormattedHexDumpSize,
              uint64_t padding_length = kDefaultPaddingSize,
              char c = '*'){
#ifdef PSDN_DEBUG
   assert(max_length % 2 == 0);
#endif//PSDN_DEBUG

   char result[max_length];

   const uint64_t kTotalLength = max_length;
   const uint64_t kPaddingLength = padding_length;
   const uint64_t kTotalPaddingLength = kPaddingLength * 2;
   const uint64_t kTitleLength = title.length();
   const uint64_t kTotalTitleAndPaddingLength = kTitleLength + kTotalPaddingLength;
   const uint64_t kFillLength = (kTotalLength - kTotalTitleAndPaddingLength) / 2;

   const uint64_t kPaddingStartPosition = 0;
   const uint64_t kTitlePosition = kPaddingStartPosition + kFillLength + kPaddingLength;
   const uint64_t kPaddingEndPosition = kTitlePosition + kTitleLength + kPaddingLength;

   memset(result, ' ', max_length);
   memset(&result[kPaddingStartPosition], c, kFillLength);
   memcpy(&result[kTitlePosition], title.data(), kTitleLength);
   memset(&result[kPaddingEndPosition], c, kFillLength + kPaddingLength);

   return {result, max_length};
 }

 static inline std::string
 CreateFooter(uint64_t max_length = kDefaultFormattedHexDumpSize, char c = '*'){
   char result[max_length];
   memset(result, c, max_length);
   return {result, max_length};
 }

 static inline std::string
 GetHexForRegion(const MemoryRegion& region){
   static const char* kHexAlphabet = "0123456789ABCDEF";
   char data[region.GetSize() * 2];

   int index = 0;
   for(auto it = region.bytes_begin(); it != region.bytes_end(); it++){
     data[index] = kHexAlphabet[(*it) >> 4];
     data[index + 1] = kHexAlphabet[(*it) & 0x0F];
     index += 2;
   }
   return {data, region.GetSize() * 2};
 }

 void HeapPrinter::Print(Heap* heap, uint64_t flags){
   LOG(INFO) << CreateHeader("Heap Dump");
   if(HasDetailedFlag(flags)){
     LOG(INFO) << "* Starting Address: " << ((void*) heap->GetStartingAddress());
     LOG(INFO) << "*   Ending Address: " << ((void*) heap->GetEndingAddress());
   }

   if((flags & kHexDump) == kHexDump){
     DLOG(INFO) << "Allocated: ";
   }
   LOG(INFO) << CreateFooter();
 }
}