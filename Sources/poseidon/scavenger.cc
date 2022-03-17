#include <deque>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/scavenger.h"
#include "poseidon/allocator.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 uword Scavenger::PromoteObject(RawObject* obj){
   DLOG(INFO) << "promoting " << obj->ToString() << " to new zone.";
   auto new_ptr = (RawObject*)Heap::GetCurrentThreadHeap()->old_zone()->Allocate(obj->GetPointerSize());
   CopyObject(obj, new_ptr);
   new_ptr->SetOldBit();

//   stats_.num_promoted += 1;
//   stats_.bytes_promoted += obj->GetPointerSize();
   return new_ptr->GetAddress();
 }

 uword Scavenger::ScavengeObject(RawObject* obj){
   DLOG(INFO) << "scavenging " << obj->ToString() << " in old zone.";
//   auto new_ptr = zone()->AllocateRawObject(obj->GetPointerSize());
   auto new_ptr = new RawObject();
   CopyObject(obj, new_ptr);
   new_ptr->SetNewBit();

//   stats_.num_scavenged += 1;
//   stats_.bytes_scavenged += obj->GetPointerSize();
   return new_ptr->GetAddress();
 }

 uword Scavenger::ProcessObject(RawObject* raw){
   if(!raw->IsForwarding()){
     if(!raw->IsMarked() && raw->IsRemembered()){
       auto new_address = PromoteObject(raw);
       ForwardObject(raw, new_address);
     } else if(!raw->IsMarked() && !raw->IsRemembered()){
       auto new_address = ScavengeObject(raw);
       ForwardObject(raw, new_address);
     }
   }
   raw->SetRememberedBit();
   return raw->GetForwardingAddress();
 }

 class SerialScavenger : public Scavenger{
  protected:
   void ProcessRoots() override{
     DLOG(INFO) << "processing roots....";
//     Allocator::VisitLocals([&](RawObject** val){
//       auto old_val = (*val);
//       DLOG(INFO) << "visiting " << old_val->ToString();
//       if(old_val->IsNew() && !old_val->IsMarked()){
//         DLOG(INFO) << "processing root: " << old_val->ToString();
//         (*val) = (RawObject*)ProcessObject(old_val);
//       }
//       (*val)->SetRememberedBit();
//       return true;
//     });
     DLOG(INFO) << "processed roots.";
   }

   void ProcessToSpace() override{
     DLOG(INFO) << "processing to-space....";
     to().VisitRawObjects([&](RawObject* val){
       DLOG(INFO) << "visiting " << val->ToString();
       if(val->IsNew() && val->IsMarked() && !val->IsRemembered()){
         DLOG(INFO) << "processing " << val->ToString();
       }
       return true;
     });
   }
  public:
   explicit SerialScavenger(NewZone* zone):
     Scavenger(zone){
   }
   ~SerialScavenger() override = default;

   void ScavengeMemory() override{
     SwapSpaces();
     ProcessRoots();
     ProcessToSpace();
   }
 };

 typedef WorkStealingQueue<uword> WorkQueue;

 class ParallelScavengerTask : public Task{
  private:
   WorkQueue* queue_;

   inline WorkQueue* queue() const{
     return queue_;
   }

   inline uword next() const{
     return queue()->Steal();
   }
  public:
   explicit ParallelScavengerTask(WorkQueue* queue):
    Task(),
    queue_(queue){
   }
   ~ParallelScavengerTask() override = default;

   void Run() override{
     int idx = 3;
     do{
       uword next_address;
       while((next_address = next()) != 0){
         auto ptr = (RawObject*)next_address;
         DLOG(INFO) << "visiting " << ptr->ToString();
       }
     } while(idx-- > 0);
   }

   const char* name() const override{
     return "ParallelScavengerTask";
   }
 };

 class ParallelScavenger : public Scavenger{
  private:
   typedef WorkStealingQueue<uword> WorkQueue;
  protected:
   TaskPool pool_;
   WorkQueue queue_;

   void ProcessRoots() override{
     DLOG(INFO) << "processing roots....";
     Allocator::VisitRoots([&](uword src, uword dst){
       auto ptr = (RawObject*)dst;
       if(ptr->IsNew() && !ptr->IsRemembered()){
         DLOG(INFO) << "queueing root @ 0x" << std::hex << dst;
         queue_.Push(dst);
       }
       return true;
     });
   }

   void ProcessToSpace() override{
     DLOG(INFO) << "processing to-space....";
   }
  public:
   explicit ParallelScavenger(NewZone* zone):
    Scavenger(zone),
    pool_(TaskPool::kDefaultNumberOfWorkers){
   }
   ~ParallelScavenger() override = default;

   void ScavengeMemory() override{
     for(auto idx = 0; idx < TaskPool::kDefaultNumberOfWorkers; idx++)
       pool_.Submit(new ParallelScavengerTask(&queue_));
     ProcessRoots();
     ProcessToSpace();
   }
 };

 void Scavenger::SerialScavenge(Heap* heap){
   SerialScavenger scavenger(heap->new_zone());
   scavenger.ScavengeMemory();
 }

 void Scavenger::ParallelScavenge(Heap* heap){
   ParallelScavenger scavenger(heap->new_zone());
   scavenger.ScavengeMemory();
 }

 void Scavenger::Scavenge(Heap* heap){
#ifdef PSDN_MTA
   ParallelScavenge(heap);
#else
   SerialScavenge(heap);
#endif//PSDN_MTA
 }
}