#include "poseidon/allocator/allocator.h"

namespace poseidon{
 void Allocator::Initialize(){
   Heap::InitializeForCurrentThread();
 }
}