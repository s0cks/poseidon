#ifndef POSEIDON_OS_THREAD_H
#define POSEIDON_OS_THREAD_H

#include "poseidon/platform.h"

#ifdef OS_IS_LINUX
#include "poseidon/os_thread_linux.h"
#elif OS_IS_OSX
#include "poseidon/os_thread_osx.h"
#elif OS_IS_WINDOWS
#include "poseidon/os_thread_windows.h"
#endif

#include <string>

namespace poseidon{
 ThreadId GetCurrentThreadId();
 std::string GetThreadName(const ThreadId& thread);
 bool SetThreadName(const ThreadId& thread, const std::string& name);

 bool Start(ThreadId* thread, const std::string& name, const ThreadHandler& func, uword parameter);
 bool Join(const ThreadId& thread);
 bool Compare(const ThreadId& lhs, const ThreadId& rhs);

 static inline std::string
 GetCurrentThreadName(){
   return GetThreadName(GetCurrentThreadId());
 }

 static inline bool
 SetCurrentThreadName(const std::string& name){
   return SetThreadName(GetCurrentThreadId(), name);
 }
}

#endif //POSEIDON_OS_THREAD_H
