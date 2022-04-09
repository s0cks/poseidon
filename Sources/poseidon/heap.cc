#include <glog/logging.h>

#include "heap.h"
#include "utils.h"
#include "raw_object.h"

namespace poseidon{
 pthread_key_t Heap::kThreadKey = PTHREAD_KEYS_MAX;

 uword Heap::AllocateNewObject(int64_t size){
   RawObject* val = nullptr;
   if((val = (RawObject*)new_zone()->Allocate(size)) != nullptr)
     goto finish_allocation;

   DLOG(WARNING) << "couldn't allocate new object of " << Bytes(size) << ".";
   //TODO: free memory from new_zone()

   if((val = (RawObject*)new_zone()->Allocate(size)) != nullptr)
     goto finish_allocation;

   LOG(FATAL) << "cannot allocate new object of " << Bytes(size) << "!";
   return 0;

finish_allocation:
   val->SetNewBit();
   return val->GetAddress();
 }

 uword Heap::AllocateOldObject(int64_t size){
   RawObject* val = nullptr;

   // 1. Try Allocation
   if((val = (RawObject*)old_zone()->TryAllocate(size)) != nullptr)
     goto finish_allocation;

   // 2. Try Major Collection
   //TODO: free memory from old_zone

   // 3. Try Allocation Again
   if((val = (RawObject*)old_zone()->TryAllocate(size)) != nullptr)
     goto finish_allocation;

   // 4. Try Pages w/ Grow
   {
     //TODO: allocate using pages, with growth
   }

   // 5. Crash
   LOG(FATAL) << "cannot allocate " << Bytes(size) << " in heap.";
   return 0;

finish_allocation:
   val->SetNewBit();
   return val->GetAddress();
 }

 uword Heap::AllocateLargeObject(int64_t size){
   return AllocateOldObject(size);//TODO: refactor
 }

 uword Heap::TryAllocate(int64_t size){
   if(size < kWordSize)
     size = kWordSize;

   if(size >= GetLargeObjectSize()){
     DLOG(INFO) << "allocating large object of " << Bytes(size);
     return AllocateLargeObject(size);
   }
   return AllocateNewObject(size);
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
}