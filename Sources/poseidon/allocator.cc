#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/marker.h"
#include "poseidon/allocator.h"
#include "poseidon/os_thread.h"
#include "poseidon/scavenger.h"

namespace poseidon{
 void Allocator::Initialize(){
   DLOG(INFO) << "initializing....";
   Heap::Initialize();
   LocalPage::Initialize();
 }

 void Allocator::MinorCollection(){
   if(Scavenger::IsScavenging()){
     DLOG(WARNING) << "scavenger is already running, skipping minor collection.";
     return;
   }

   Scavenger scavenger(Heap::GetCurrentThreadHeap());
   TIMED_SECTION("Minor Collection.", {
     scavenger.Scavenge();
   });
 }

 void Allocator::MajorCollection(){
   //TODO: collection check.

   Marker::Mark();
 }
}