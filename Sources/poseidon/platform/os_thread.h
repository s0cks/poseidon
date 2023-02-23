#ifndef POSEIDON_OS_THREAD_H
#define POSEIDON_OS_THREAD_H

#include <glog/logging.h>
#include "poseidon/platform/platform.h"

#ifdef OS_IS_LINUX
#include "poseidon/platform/os_thread_linux.h"
#elif OS_IS_OSX
#include "poseidon/platform/os_thread_osx.h"
#elif OS_IS_WINDOWS
#include "poseidon/platform/os_thread_windows.h"
#endif

#include <string>

namespace poseidon{
 ThreadId GetCurrentThreadId();
 std::string GetThreadName(const ThreadId& thread);
 bool SetThreadName(const ThreadId& thread, const std::string& name);
 bool InitializeThreadLocal(ThreadLocalKey& key);
 bool SetCurrentThreadLocal(const ThreadLocalKey& key, const void* value);
 void* GetCurrentThreadLocal(const ThreadLocalKey& key);

 bool StartThread(ThreadId* thread, const std::string& name, const ThreadHandler& func, uword parameter);
 bool JoinThread(const ThreadId& thread);
 bool Compare(const ThreadId& lhs, const ThreadId& rhs);

 static inline std::string
 GetCurrentThreadName(){
   return GetThreadName(GetCurrentThreadId());
 }

 static inline bool
 SetCurrentThreadName(const std::string& name){
   return SetThreadName(GetCurrentThreadId(), name);
 }

 class OSThread {
  protected:
   std::string name_;
   ThreadId thread_id_;
  public:
   OSThread() = delete;
   explicit OSThread(std::string name):
    name_(std::move(name)),
    thread_id_() {
   }
   OSThread(const OSThread& rhs) = delete;
   ~OSThread() = default;

   virtual void Run() = 0;

   std::string name() const {
     return name_;
   }

   ThreadId thread_id() const {
     return thread_id_;
   }

   void Join();
   void Start();

   OSThread& operator=(const OSThread& rhs) = delete;

   friend std::ostream& operator<<(std::ostream& stream, const OSThread& os_thread) {
     stream << "OSThread(";
     stream << "name=" << os_thread.name();
     stream << ")";
     return stream;
   }
 };

 template<typename T>
 class ThreadLocal {
  protected:
   std::string name_;
   ThreadLocalKey key_;
  public:
   explicit ThreadLocal(std::string name):
    name_(std::move(name)),
    key_() {
     LOG_IF(FATAL, !InitializeThreadLocal(key_)) << "cannot initialize ThreadLocal `" << name_ << "` in thread: " << GetCurrentThreadName();
   }
   ThreadLocal(ThreadLocal<T>& rhs) = default;
   ~ThreadLocal() = default;

   std::string name() const {
     return name_;
   }

   ThreadLocalKey& key() {
     return key_;
   }

   ThreadLocalKey key() const {
     return key_;
   }

   T* Get() const {
     return (T*) GetCurrentThreadLocal(key());
   }

   void Set(T* value) {
     SetCurrentThreadLocal(key(), value);
   }

   explicit operator bool() const {
     return Get() != nullptr;
   }

   T* operator->() {
     return GetCurrentThreadLocal(key());
   }

   explicit operator T() const {
     return GetCurrentThreadLocal(key());
   }

   ThreadLocal& operator=(const ThreadLocal<T>& rhs) = default;

   ThreadLocal<T>& operator=(T* rhs) {
     Set(rhs);
     return *this;
   }
 };
}

#endif //POSEIDON_OS_THREAD_H
