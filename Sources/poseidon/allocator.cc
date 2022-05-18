#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/marker.h"
#include "poseidon/sweeper.h"
#include "poseidon/allocator.h"
#include "poseidon/os_thread.h"
#include "poseidon/scavenger.h"

namespace poseidon{
 static RelaxedAtomic<bool> major_collection_(false);

 void Allocator::SetMajorCollection(){
   major_collection_ = true;
 }

 void Allocator::ClearMajorCollection(){
   major_collection_ = false;
 }

 bool Allocator::IsMajorCollectionActive(){
   return (bool)major_collection_;
 }

 void Allocator::Initialize(){
   Heap::Initialize();
   LocalPage::Initialize();
 }

 void Allocator::MinorCollection(){
   if(Scavenger::IsScavenging()){
     DLOG(WARNING) << "scavenger is already running, skipping minor collection.";
     return;
   }

   TIMED_SECTION("MinorCollection", {
     Scavenger::Scavenge();
   })
 }

 void Allocator::MajorCollection(){
   TIMED_SECTION("MajorCollection", {
     Marker::MarkAllLiveObjects();

     Sweeper::Sweep();
   });
 }
}