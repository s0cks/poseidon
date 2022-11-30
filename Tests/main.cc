#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/flags.h"
#include "poseidon/runtime.h"
#include "poseidon/poseidon.h"
#include "poseidon/type/class.h"
#include "poseidon/local/local_page.h"
#include "poseidon/collector/scavenger.h"
#include "poseidon/allocator/allocator.h"

int main(int argc, char** argv){
  using namespace poseidon;
  ::google::InitGoogleLogging(argv[0]);
  ::google::LogToStderr();

  ::testing::InitGoogleTest(&argc, argv);
  ::google::ParseCommandLineFlags(&argc, &argv, false);
  LOG(INFO) << "Running unit tests for poseidon v" << poseidon::GetVersion() << "....";

  flags::FlagsPrinter::PrintFlags();

  Heap::Initialize();
  Class::Initialize();
  Runtime::Initialize();
  LocalPage::InitializeForCurrentThread(1024); //TODO: fix
  return RUN_ALL_TESTS();
}