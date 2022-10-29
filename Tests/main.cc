#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/local.h"
#include "poseidon/runtime.h"
#include "poseidon/poseidon.h"
#include "poseidon/collector/scavenger.h"
#include "poseidon/allocator/allocator.h"

int main(int argc, char** argv){
  using namespace poseidon;
  ::google::InitGoogleLogging(argv[0]);
  ::google::LogToStderr();

  ::testing::InitGoogleTest(&argc, argv);
  ::google::ParseCommandLineFlags(&argc, &argv, false);
  LOG(INFO) << "Running unit tests for poseidon v" << poseidon::GetVersion() << "....";

  Runtime::Initialize();
  LocalPage::Initialize();
  return RUN_ALL_TESTS();
}