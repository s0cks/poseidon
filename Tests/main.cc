#include <gtest/gtest.h>
#include <glog/logging.h>
#include "poseidon/poseidon.h"
#include "poseidon/collector.h"
#include "poseidon/object.h"

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

  auto val = Int::NewLocal(10);
  DLOG(INFO) << "value (before): " << val->Get() << " (" << val.GetRawObjectPointer()->ToString() << ").";

  for(auto j = 0; j < 10; j++){
    for(auto i = 0; i < 65536; i++)
      Int::New(i);
  }

  DLOG(INFO) << "value (after): " << val->Get() << " (" << val.GetRawObjectPointer()->ToString() << ").";

  return RUN_ALL_TESTS();
}