#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/poseidon.h"
#include "poseidon/task_pool.h"
#include "poseidon/allocator.h"

namespace poseidon{
 class EchoTask : public Task{
  private:
   RelaxedAtomic<uint64_t>& counter_;
  public:
   EchoTask(RelaxedAtomic<uint64_t>& counter):
    Task(),
    counter_(counter){
   }
   ~EchoTask() override = default;

   const char * name() const override{
     return "EchoTask";
   }

   void Run() override{
     LOG(INFO) << "echo #" << (counter_ += 1) << " from " << GetCurrentThreadName() << ".";
   }
 };
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

  Heap::Initialize();
  Allocator::Initialize();

  TaskPool pool;

  sleep(5);

  static constexpr const uint64_t kNumberOfEchoTasks = 128;

  RelaxedAtomic<uint64_t> counter(0);

  for(auto idx = 0; idx < kNumberOfEchoTasks; idx++){
    DLOG(INFO) << "submitting EchoTask #" << idx;
    pool.Submit(new EchoTask(counter));
  }

  sleep(15);

  LOG(INFO) << "count: " << counter;
  return RUN_ALL_TESTS();
}