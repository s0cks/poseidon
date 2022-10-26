#ifndef POSEIDON_HELPERS_H
#define POSEIDON_HELPERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "poseidon/local.h"
#include "poseidon/heap/heap.h"

#include "helpers/assertions.h"

namespace poseidon{
 using namespace ::testing;

 class MockFreeListNodeVisitor : public FreeListNodeVisitor {
  public:
   MockFreeListNodeVisitor():
    FreeListNodeVisitor() {
     ON_CALL(*this, Visit)
      .WillByDefault([](FreeListNode* val) {
        return true;
      });
   }
   ~MockFreeListNodeVisitor() override = default;
   MOCK_METHOD(bool, Visit, (FreeListNode*), (override));
 };

 class MockRawObjectVisitor : public RawObjectVisitor{
  public:
   MockRawObjectVisitor():
    RawObjectVisitor(){
     ON_CALL(*this, Visit)
      .WillByDefault([](RawObject* val){
        return true;
      });
   }
   ~MockRawObjectVisitor() override = default;

   MOCK_METHOD(bool, Visit, (RawObject*), (override));
 };

 class MockPageVisitor : public PageVisitor {
  public:
   MockPageVisitor():
    PageVisitor() {
     ON_CALL(*this, VisitPage)
     .WillByDefault([](Page* val) {
       return true;
     });
   }
   ~MockPageVisitor() override = default;

   MOCK_METHOD(bool, VisitPage, (Page*), (override));
 };

 static inline uword
 FailAllocation(int64_t size){
   DLOG(ERROR) << "allocating new object (" << Bytes(size) << ") failed.";
   return 0;
 }

 static inline uword
 SystemAlloc(int64_t size){
   return (uword)malloc(size);
 }

 static inline uword
 SystemAllocNewObject(int64_t size){
   DLOG(INFO) << "allocating new object (" << Bytes(size) << ") using malloc.";
   auto total_size = sizeof(RawObject) + size;
   auto val = (RawObject*)SystemAlloc(size);
   if(!val){
     LOG(FATAL) << "failed to allocate new object (" << Bytes(size) << ") using system [malloc].";
     return 0;
   }

   memset((void*)val, 0, total_size);
   val->SetPointerSize(size);
   return val->GetStartingAddress();
 }

 template<class S>
 static inline RawObject*
 TryAllocateNewWordInSection(S* section){
   return (RawObject*)section->TryAllocate(kWordSize);
 }

 template<class S>
 static inline RawObject*
 TryAllocateNewWord(S* section, word value){
   auto val = TryAllocateNewWordInSection(section);
   if(val != nullptr)
     *((word*)val->GetPointer()) = value;
   return val;
 }

 template<class S>
 static inline RawObject*
 TryAllocateMarkedWord(S* section, word value = 0){
   auto val = TryAllocateNewWord(section, value);
   if(val != nullptr)
     val->SetMarkedBit();
   return val;
 }

 template<class S>
 static inline RawObject*
 TryAllocateNewRememberedWord(S* section, word value = 0){
   auto val = TryAllocateNewWord(section, value);
   if(val != nullptr)
     val->SetRememberedBit();
   return val;
 }
}

#endif //POSEIDON_HELPERS_H
