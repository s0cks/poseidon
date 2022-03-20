#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/local.h"
#include "poseidon/poseidon.h"
#include "poseidon/scavenger.h"
#include "poseidon/allocator.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 class ProcessOldPageTask : public Task{
  protected:
   OldZone::OldPage* page_;
   RelaxedAtomic<int64_t>* countdown_;

   inline OldZone::OldPage* page() const{
     return page_;
   }

   void Run() override{
     DLOG(INFO) << "[" << GetCurrentThreadName() << "] processing " << (*page()) << ".....";
     page()->VisitPointers([&](RawObject* val){
       DLOG(INFO) << "[" << GetCurrentThreadName() << "] marking " << val->ToString() << ".....";
       val->SetMarkedBit();
       return true;
     });
     DLOG(INFO) << "[" << GetCurrentThreadName() << "] done processing " << (*page()) << "....";
     countdown_ -= 1;
   }
  public:
   ProcessOldPageTask():
    Task(),
    page_(nullptr),
    countdown_(nullptr){
   }
   explicit ProcessOldPageTask(OldZone::OldPage* page, RelaxedAtomic<int64_t>* countdown):
    Task(),
    page_(page),
    countdown_(countdown){
   }
   ~ProcessOldPageTask() override = default;

   const char* name() const override{
     return "ProcessOldPageTask";
   }
 };

 static inline Local<word>
 AllocateWord(word value){
   auto raw_ptr = (RawObject*)Allocator::Allocate(sizeof(word));
   *((word*)raw_ptr->GetPointer()) = value;
   auto handle = Allocator::AllocateLocal<word>();
   handle = raw_ptr->GetAddress();
   return handle;
 }
}

int main(int argc, char** argv){
  using namespace poseidon;
  ::google::InitGoogleLogging(argv[0]);
  ::google::LogToStderr();
  ::testing::InitGoogleTest(&argc, argv);
  LOG(INFO) << "Running unit tests for poseidon v" << poseidon::GetVersion() << "....";

#ifdef PSDN_MTA
  LOG(WARNING) << "*** Using Multi-Threaded Algorithm ***";
#endif//PSDN_MTA

  DLOG(INFO) << "sizeof(NewPage) := " << sizeof(NewPage);
  DLOG(INFO) << "sizeof(Reference) := " << sizeof(Reference);
  DLOG(INFO) << "sizeof(word) := " << sizeof(word);

  Heap::Initialize();
  Allocator::Initialize();

  auto v1 = (RawObject*)Allocator::Allocate(sizeof(word));
  *((word*)v1->GetPointer()) = 1;
  auto v2 = (RawObject*)Allocator::Allocate(sizeof(word));
  *((word*)v2->GetPointer()) = 100000;

  auto h1 = AllocateWord(100);
  auto h2 = AllocateWord(1000);
  auto h3 = AllocateWord(10);
  DLOG(INFO) << "h1 (before): " << (*h1.Get()) << " (" << h1.raw()->ToString() << ").";
  DLOG(INFO) << "h2 (before): " << (*h2.Get()) << " (" << h2.raw()->ToString() << ").";
  DLOG(INFO) << "h3 (before): " << (*h3.Get()) << " (" << h3.raw()->ToString() << ").";
  DLOG(INFO) << "v1 (before): " << (*((word*)v1->GetPointer())) << " (" << v1->ToString() << ").";
  DLOG(INFO) << "v2 (before): " << (*((word*)v2->GetPointer())) << " (" << v2->ToString() << ").";

  Allocator::MinorCollection();

  DLOG(INFO) << "h1 (after): " << (*h1.Get()) << " (" << h1.raw()->ToString() << ").";
  DLOG(INFO) << "h2 (after): " << (*h2.Get()) << " (" << h2.raw()->ToString() << ").";
  DLOG(INFO) << "h3 (after): " << (*h3.Get()) << " (" << h3.raw()->ToString() << ").";
  DLOG(INFO) << "v1 (after): " << (*((word*)v1->GetPointer())) << " (" << v1->ToString() << ").";
  DLOG(INFO) << "v2 (after): " << (*((word*)v2->GetPointer())) << " (" << v2->ToString() << ").";


//  auto raw_ptr = (RawObject*)Allocator::Allocate(sizeof(uword));
//  (*((uword*)raw_ptr->GetPointer())) = 100;
//  DLOG(INFO) << "value: " << (*((uword*)raw_ptr->GetPointer())) << " (" << raw_ptr->ToString() << ").";
//
//  auto handle = Allocator::AllocateLocal<uword>();
//  handle = raw_ptr->GetAddress(); // returns RawObject* (start of object + header).
//  (*handle.Get()) = 1000;
//
//  DLOG(INFO) << "handle: " << (*handle.Get()) << " (" << handle.raw()->ToString() << ").";
//
//  static const int64_t kDefaultNumberOfGarbageObjects = 1024;
//  for(auto idx = 0; idx < kDefaultNumberOfGarbageObjects; idx++){
//    auto val = Allocator::New<int64_t>(idx);
//    DLOG(INFO) << "#" << idx << " := " << (*val);
//  }
//
//  auto heap = Heap::GetCurrentThreadHeap();
//  Scavenger::Scavenge(heap->new_zone());

//  TaskPool pool;
//
//  sleep(5);
//
//  static constexpr const uint64_t kNumberOfEchoTasks = 128;
//
//  RelaxedAtomic<uint64_t> counter(0);
//
//  for(auto idx = 0; idx < kNumberOfEchoTasks; idx++){
//    DLOG(INFO) << "submitting EchoTask #" << idx;
//    pool.Submit(new EchoTask(counter));
//  }
//
//  sleep(15);
//
//  LOG(INFO) << "count: " << counter;
  return RUN_ALL_TESTS();
}