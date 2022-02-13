#include <gtest/gtest.h>
#include <glog/logging.h>
#include "poseidon/poseidon.h"
#include "poseidon/scavenger.h"
#include "poseidon/object.h"
#include "poseidon/raw_object_printer.h"

int main(int argc, char** argv){
  using namespace poseidon;
  ::google::InitGoogleLogging(argv[0]);
  ::google::LogToStderr();
  ::testing::InitGoogleTest(&argc, argv);
  LOG(INFO) << "Running unit tests for poseidon v" << poseidon::GetVersion() << "....";
#ifdef PSDN_MTA
  LOG(WARNING) << "*** Using Multi-Threaded Algorithm ***";
#endif//PSDN_MTA

  Allocator::Initialize();
  Class::Initialize();
  DLOG(INFO) << "sizeof(RawObject) := " << sizeof(RawObject);
  DLOG(INFO) << "sizeof(uint64_t) := " << sizeof(uint64_t);
  DLOG(INFO) << "sizeof(Instance) := " << sizeof(Instance);
  DLOG(INFO) << "sizeof(Class) := " << Class::CLASS_CLASS->GetAllocationSize();
  DLOG(INFO) << "sizeof(Byte) := " << Class::CLASS_BYTE->GetAllocationSize();
  DLOG(INFO) << "sizeof(Int) := " << Class::CLASS_INT->GetAllocationSize();
  DLOG(INFO) << "sizeof(Bool) := " << Class::CLASS_BOOL->GetAllocationSize();
  DLOG(INFO) << "sizeof(Array) := " << Class::CLASS_ARRAY->GetAllocationSize();

  static const uint64_t kMaxInts = 489794;
  for(auto val = 0; val < kMaxInts; val++){
    Int::New(val);
  }

  auto val = Instance::NewLocalArrayInstance(10);
  val->SetAt(0, Int::New(2222));
  val->SetAt(1, Int::New(1111));

  DLOG(INFO) << "array.length := " << val->GetLength();
  DLOG(INFO) << "array[0] := " << val->GetAt<Int>(0)->Get() << " (" << val->GetAt<Int>(0)->raw()->ToString() << ")";
  DLOG(INFO) << "array[1] := " << val->GetAt<Int>(1)->Get() << " (" << val->GetAt<Int>(1)->raw()->ToString() << ")";

  Scavenger::MinorCollection();

  DLOG(INFO) << "array.length := " << val->GetLength();
  DLOG(INFO) << "array[0] := " << val->GetAt<Int>(0)->Get() << " (" << val->GetAt<Int>(0)->raw()->ToString() << ")";
  DLOG(INFO) << "array[1] := " << val->GetAt<Int>(1)->Get() << " (" << val->GetAt<Int>(1)->raw()->ToString() << ")";
  return RUN_ALL_TESTS();
}