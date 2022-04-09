#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/local.h"
#include "poseidon/runtime.h"
#include "poseidon/poseidon.h"
#include "poseidon/scavenger.h"
#include "poseidon/allocator.h"

namespace poseidon{
 static inline Local<word>
 AllocateWord(word value){
   auto raw_ptr = (RawObject*)Allocator::Allocate(sizeof(word));
   *((word*)raw_ptr->GetPointer()) = value;

   Local<uword> handle;
   handle = raw_ptr->GetAddress();
   return handle;
 }

 static inline Local<uword>
 AllocateLargeObject(int64_t size){
   auto raw_ptr = (RawObject*)Allocator::Allocate(sizeof(uint8_t) * size);

   Local<uword> handle;
   handle = raw_ptr->GetAddress();
   return handle;
 }
}

int main(int argc, char** argv){
  using namespace poseidon;
  ::google::InitGoogleLogging(argv[0]);
  ::google::LogToStderr();
  ::testing::InitGoogleTest(&argc, argv);
  ::google::ParseCommandLineFlags(&argc, &argv, false);
  LOG(INFO) << "Running unit tests for poseidon v" << poseidon::GetVersion() << "....";

  Runtime::Initialize();
  Allocator::Initialize();

  auto v1 = (RawObject*)Allocator::Allocate(sizeof(word));
  *((word*)v1->GetPointer()) = 1;
  auto v2 = (RawObject*)Allocator::Allocate(sizeof(word));
  *((word*)v2->GetPointer()) = 100000;

  auto h1 = AllocateWord(100);
  auto h2 = AllocateWord(1000);
  auto h3 = AllocateWord(10);

  auto l1 = AllocateLargeObject(128 * kKB);
  (*((uword*)l1.Get())) = 10;

  auto l2 = AllocateLargeObject(128 * kKB);
  (*((uword*)l2.Get())) = 1000;

  DLOG(INFO) << "h1 (before): " << (*h1.Get()) << " (" << h1.raw()->ToString() << ").";
  DLOG(INFO) << "h2 (before): " << (*h2.Get()) << " (" << h2.raw()->ToString() << ").";
  DLOG(INFO) << "h3 (before): " << (*h3.Get()) << " (" << h3.raw()->ToString() << ").";
  DLOG(INFO) << "v1 (before): " << (*((word*)v1->GetPointer())) << " (" << v1->ToString() << ").";
  DLOG(INFO) << "v2 (before): " << (*((word*)v2->GetPointer())) << " (" << v2->ToString() << ").";
  DLOG(INFO) << "l1 (before): " << (*l1.Get()) << " (" << l1.raw()->ToString() << ").";
  DLOG(INFO) << "l2 (before): " << (*l2.Get()) << " (" << l2.raw()->ToString() << ").";

  static constexpr const int64_t kNumberOfRoots = 32;
  static constexpr const int64_t kNumberOfGarbage = 65546;

  for(auto idx = 0; idx < kNumberOfRoots; idx++){
    auto r = AllocateWord(idx);
    auto l = AllocateLargeObject(32 * kKB);
  }

  for(auto idx = 0; idx < kNumberOfGarbage; idx++){
    auto v = (RawObject*)Allocator::Allocate(sizeof(word));
    *((word*)v->GetPointer()) = idx;
  }

  Allocator::MinorCollection();
  Allocator::MinorCollection();

//
//  for(auto idx = 0; idx < kNumberOfGarbage; idx++){
//    auto v = (RawObject*)Heap::GetCurrentThreadHeap()->old_zone()->Allocate(sizeof(word));
//    *((word*)v->GetPointer()) = idx;
//  }

  DLOG(INFO) << "h1 (after): " << (*h1.Get()) << " (" << h1.raw()->ToString() << ").";
  DLOG(INFO) << "h2 (after): " << (*h2.Get()) << " (" << h2.raw()->ToString() << ").";
  DLOG(INFO) << "h3 (after): " << (*h3.Get()) << " (" << h3.raw()->ToString() << ").";
  DLOG(INFO) << "v1 (after): " << (*((word*)v1->GetPointer())) << " (" << v1->ToString() << ").";
  DLOG(INFO) << "v2 (after): " << (*((word*)v2->GetPointer())) << " (" << v2->ToString() << ").";
  DLOG(INFO) << "l1 (after): " << (*l1.Get()) << " (" << l1.raw()->ToString() << ").";
  DLOG(INFO) << "l2 (after): " << (*l2.Get()) << " (" << l2.raw()->ToString() << ").";

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