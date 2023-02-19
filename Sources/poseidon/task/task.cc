#include "poseidon/task/task.h"

namespace poseidon {
 Task::Task(Task* parent):
   parent_(nullptr),
   state_(State::kUnqueued),
   children_(1) {
   SetParent(parent);
 }
}