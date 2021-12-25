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
  DLOG(INFO) << "sizeof(uint64_t) := " << sizeof(uint64_t);
  DLOG(INFO) << "sizeof(Instance) := " << sizeof(Instance);
  DLOG(INFO) << "sizeof(Class) := " << Class::CLASS_CLASS->GetAllocationSize();
  DLOG(INFO) << "sizeof(Byte) := " << Class::CLASS_BYTE->GetAllocationSize();
  DLOG(INFO) << "sizeof(Int) := " << Class::CLASS_INT->GetAllocationSize();

  Int* a = Int::New();
  a->Set(11111);
  LOG(INFO) << "a: " << a->Get();

  Local<Int> b = Int::NewLocal();
  b->Set(10000);

  LOG(INFO) << "b: " << b->Get();

  for(auto i = 0; i < 500000; i++){
    auto local = Int::New();
    local->Set(i);
  }

//  HeapPrinter::Print(Allocator::GetEdenHeap());
//  HeapPrinter::Print(Allocator::GetTenuredHeap());
//  HeapPrinter::Print(Allocator::GetLargeObjectHeap());
//  LOG(INFO) << "Locals:";
//  RawObjectPrinter::PrintAllLocals();

  Scavenger::MinorCollection();

//  HeapPrinter::Print(Allocator::GetEdenHeap());
//  HeapPrinter::Print(Allocator::GetTenuredHeap());
//  HeapPrinter::Print(Allocator::GetLargeObjectHeap());
//  LOG(INFO) << "Locals:";
//  RawObjectPrinter::PrintAllLocals();

  LOG(INFO) << "a: " << a->Get();
  LOG(INFO) << "b: " << b->Get();
  return RUN_ALL_TESTS();
}