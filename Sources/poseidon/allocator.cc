#include "poseidon/utils.h"
#include "poseidon/local.h"
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

   DLOG(INFO) << "starting minor collection.....";
   Scavenger scavenger(Heap::GetCurrentThreadHeap());

#ifdef PSDN_DEBUG
   auto start_ts = Clock::now();
#endif//PSDN_DEBUG

   scavenger.Scavenge();

#ifdef PSDN_DEBUG
   auto finish_ts = Clock::now();
   DLOG(INFO) << "minor collection finished in " << (finish_ts - start_ts) << ".";
//TODO:
//   DLOG(INFO) << " - " << scavenger.num_scavenged_ << " objects scavenged (" << Bytes(scavenger.bytes_scavenged_) << ").";
//   DLOG(INFO) << " - " << scavenger.num_promoted_ << " objects promoted (" << Bytes(scavenger.bytes_promoted_) << ").";
//   DLOG(INFO) << " - " << scavenger.num_finalized_ << " objects finalized (" << Bytes(scavenger.bytes_finalized_) << ").";
#endif//PSDN_DEBUG
 }

 void Allocator::MajorCollection(){

 }
}