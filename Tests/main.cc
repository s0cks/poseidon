#include <gtest/gtest.h>
#include <glog/logging.h>
#include "poseidon/poseidon.h"
#include "poseidon/allocator.h"
#include "poseidon/collector.h"

int main(int argc, char** argv){
  using namespace poseidon;
  ::google::InitGoogleLogging(argv[0]);
  ::google::LogToStderr();
  ::testing::InitGoogleTest(&argc, argv);
  LOG(INFO) << "Running unit tests for poseidon v" << poseidon::GetVersion() << "....";

#ifdef PSDN_MTA
  LOG(WARNING) << "*** Using Multi-Threaded Algorithm ***";
#endif//PSDN_MTA

  Heap::Initialize();
  Allocator::Initialize();

  int* test = Allocator::New<int>();

  return RUN_ALL_TESTS();
}