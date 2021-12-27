#include <gtest/gtest.h>
#include <glog/logging.h>
#include "poseidon.h"
#include "allocator.h"
#include "local.h"
#include "scavenger.h"
#include "raw_object_printer.h"
#include "object.h"
#include "heap_printer.h"

int main(int argc, char** argv){
  using namespace poseidon;
  ::google::InitGoogleLogging(argv[0]);
  ::google::LogToStderr();
  ::testing::InitGoogleTest(&argc, argv);
  LOG(INFO) << "Running unit tests for poseidon v" << poseidon::GetVersion() << "....";

  Allocator::Initialize();
  Class::Initialize();
  DLOG(INFO) << "sizeof(RawObject) := " << sizeof(RawObject);
  DLOG(INFO) << "sizeof(uint64_t) := " << sizeof(uint64_t);
  DLOG(INFO) << "sizeof(Instance) := " << sizeof(Instance);
  DLOG(INFO) << "sizeof(Class) := " << Class::CLASS_CLASS->GetAllocationSize();
  DLOG(INFO) << "sizeof(Byte) := " << Class::CLASS_BYTE->GetAllocationSize();
  DLOG(INFO) << "sizeof(Int) := " << Class::CLASS_INT->GetAllocationSize();
  DLOG(INFO) << "sizeof(Bool) := " << Class::CLASS_BOOL->GetAllocationSize();

  Local<Bool> true_val = Bool::NewLocal(true);
  Local<Bool> false_val = Bool::NewLocal(false);

  Local<Int> a = Int::NewLocal(11111);
  LOG(INFO) << "a: " << a->Get();
  Local<Int> b = Int::NewLocal(10000);
  LOG(INFO) << "b: " << b->Get();

  static const uint64_t kMaxInts = 645270; // Max: 645275
  for(auto i = 0; i < kMaxInts; i++){
    Int::New(i);
  }

  Scavenger::MinorCollection();
  //Scavenger::MinorCollection();

  LOG(INFO) << "a: " << a->Get();
  LOG(INFO) << "b: " << b->Get();
  LOG(INFO) << "true: " << (true_val->Get() ? "true" : "false");
  LOG(INFO) << "false: " << (false_val->Get() ? "true" : "false");
  return RUN_ALL_TESTS();
}