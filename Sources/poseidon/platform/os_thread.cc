#include "poseidon/platform/os_thread.h"

namespace poseidon {
 static inline void
 HandleOSThread(uword data) {
   auto thread = (OSThread*)data;
   thread->Run();
 }

 void OSThread::Start() {
   DLOG(INFO) << "starting " << (*this) << "....";
   LOG_IF(FATAL, !StartThread(&thread_id_, name(), &HandleOSThread, (uword)this)) << "failed to start " << (*this) << ".";
 }

 void OSThread::Join() {
   DLOG(INFO) << "joining " << (*this) << "....";
   LOG_IF(FATAL, !JoinThread(thread_id())) << "failed to join " << (*this) << ".";
 }
}