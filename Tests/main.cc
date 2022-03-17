#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/local.h"
#include "poseidon/poseidon.h"
#include "poseidon/scavenger.h"
#include "poseidon/allocator.h"

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