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

 class IsPointerToMatcher {
  protected:
   const uword start_;
   const ObjectSize size_;
  public:
   using is_gtest_matcher = void;

   IsPointerToMatcher(const uword start,
                      const ObjectSize size):
       start_(start),
       size_(size) {
   }

   uword starting_address() const {
     return start_;
   }

   ObjectSize size() const {
     return size_;
   }

   bool MatchAndExplain(RawObject* ptr, std::ostream*) const {
     return ptr->GetStartingAddress() == starting_address() &&
            ptr->GetSize() == size();
   }

   void DescribeTo(std::ostream* stream) const {
     (*stream) << "pointer points to " << ((void*) starting_address()) << " (" << Bytes(size()) << ")";
   }

   void DescribeNegationTo(std::ostream* stream) const {
     (*stream) << "pointer does not point to " << ((void*) starting_address()) << " (" << Bytes(size()) << ")";
   }
 };

 static inline Matcher<RawObject*>
 IsPointerTo(const uword start, const int64_t size) {
   return IsPointerToMatcher(start, size);
 }

 static inline Matcher<RawObject*>
 IsPointerTo(const RawObject* ptr) {
   return IsPointerTo(ptr->GetStartingAddress(), ptr->GetSize());
 }

 template<class T>
 static inline Matcher<RawObject*>
 IsPointerTo(const Local<T>& val) {
   return IsPointerTo(val.raw());
 }

 class MockFreeListNodeVisitor : public FreeObjectVisitor {
  public:
   MockFreeListNodeVisitor():
       FreeObjectVisitor() {
     ON_CALL(*this, Visit)
      .WillByDefault([](FreeObject* val) {
        return true;
      });
   }
   ~MockFreeListNodeVisitor() override = default;
   MOCK_METHOD(bool, Visit, (FreeObject*), (override));
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
}

#endif //POSEIDON_HELPERS_H
