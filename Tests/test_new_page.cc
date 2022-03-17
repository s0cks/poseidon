#include <gtest/gtest.h>

#include "poseidon/heap.h"

namespace poseidon{
 TEST(NewPageTest, TestGetIndexFromPageDimensions){
   auto heap = new Heap();
   ASSERT_EQ(NewPage::GetIndexFromPageDimensions(heap->GetStartingAddress(), heap->GetStartingAddress() + (0 * kNewZonePageSize)), 0);
   ASSERT_EQ(NewPage::GetIndexFromPageDimensions(heap->GetStartingAddress(), heap->GetStartingAddress() + (1 * kNewZonePageSize)), 1);
   ASSERT_EQ(NewPage::GetIndexFromPageDimensions(heap->GetStartingAddress(), heap->GetStartingAddress() + (2 * kNewZonePageSize)), 2);
   ASSERT_EQ(NewPage::GetIndexFromPageDimensions(heap->GetStartingAddress(), heap->GetStartingAddress() + (3 * kNewZonePageSize)), 3);
   ASSERT_EQ(NewPage::GetIndexFromPageDimensions(heap->GetStartingAddress(), heap->GetStartingAddress() + (4 * kNewZonePageSize)), 4);
 }

 TEST(NewPageTest, Test){
   auto heap = new Heap();
   auto val = (RawObject*)heap->Allocate(sizeof(uword));
   DLOG(INFO) << "val: " << val->ToString();

   heap->new_zone()->VisitPages([&](NewPage* page){
     DLOG(INFO) << "visiting pointers in " << (*page);
     page->VisitPointers([&](RawObject* val){
       DLOG(INFO) << " - " << val->ToString();
       return true;
     });
     return true;
   });
 }
}