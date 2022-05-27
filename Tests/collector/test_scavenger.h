#ifndef POSEIDON_TEST_SCAVENGER_H
#define POSEIDON_TEST_SCAVENGER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "memory_region_test.h"
#include "poseidon/collector/scavenger.h"

namespace poseidon{
 using namespace ::testing;

 class ScavengerTest : public HeapTest{
  protected:
   Semispace from_;
   Semispace to_;

   explicit ScavengerTest(int64_t size = GetTotalInitialHeapSize()):
    HeapTest(size),
    from_(new_zone()->fromspace(), new_zone()->semisize()),
    to_(new_zone()->tospace(), new_zone()->semisize()){
   }

   inline Semispace* from(){
     return &from_;
   }

   inline Semispace* to(){
     return &to_;
   }

   inline RawObject* ScavengeObject(RawObject* ptr){
     return (RawObject*)Scavenger::ScavengeObject(to(), ptr);
   }

   inline RawObject* PromoteObject(RawObject* ptr){
     return (RawObject*)Scavenger::PromoteObject(old_zone(), ptr);
   }

   inline RawObject* ProcessObject(RawObject* ptr){
     return (RawObject*)Scavenger::ProcessObject(to(), old_zone(), ptr);
   }

   inline void SerialScavenge(){
     return Scavenger::SerialScavenge(heap());
   }

   inline void ParallelScavenge(){
     return Scavenger::ParallelScavenge(heap());
   }
  public:
   ~ScavengerTest() override = default;
 };
}

#endif//POSEIDON_TEST_SCAVENGER_H