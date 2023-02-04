#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/flags.h"
#include "poseidon/runtime.h"
#include "poseidon/poseidon.h"
#include "poseidon/type.h"
#include "poseidon/collector/scavenger.h"
#include "poseidon/allocator/allocator.h"

#define PRINT_SIZEOF(Level, Type) \
  DLOG(Level) << "sizeof(" << #Type << ") := " << (sizeof(Type));
#define PRINT_ALLOC_SIZE(Level, Type) \
  DLOG(Level) << "sizeof(" << #Type << ") := " << Bytes((Type::GetClassAllocationSize()));

int main(int argc, char** argv){
  using namespace poseidon;
  ::google::InitGoogleLogging(argv[0]);
  ::google::LogToStderr();

  ::testing::InitGoogleTest(&argc, argv);
  ::google::ParseCommandLineFlags(&argc, &argv, false);
  LOG(INFO) << "Running unit tests for poseidon v" << poseidon::GetVersion() << "....";

  SetCurrentThreadName("poseidon");

  Heap::Initialize();
  Class::Initialize();
  Runtime::Initialize();

#ifdef PSDN_DEBUG
  PRINT_SIZEOF(INFO, word);
  PRINT_SIZEOF(INFO, uword);
  PRINT_SIZEOF(INFO, RelaxedAtomic<uword>);
  PRINT_SIZEOF(INFO, Pointer);
  PRINT_SIZEOF(INFO, RawPointerTag);
  PRINT_SIZEOF(INFO, RelaxedAtomic<RawPointerTag>);
  PRINT_SIZEOF(INFO, PointerTag);
  PRINT_SIZEOF(INFO, FreePointer);
  PRINT_SIZEOF(INFO, Instance);

  PRINT_ALLOC_SIZE(INFO, Bool);
  PRINT_ALLOC_SIZE(INFO, UInt8);
  PRINT_ALLOC_SIZE(INFO, Int8);

  flags::FlagsPrinter::PrintFlags();
#endif //PSDN_DEBUG
  return RUN_ALL_TESTS();
}