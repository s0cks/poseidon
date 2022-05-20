#include "poseidon/local.h"
#include "poseidon/allocator/allocator.h"

namespace poseidon{
 void Allocator::Initialize(){
   Heap::Initialize();
   LocalPage::Initialize();
 }
}