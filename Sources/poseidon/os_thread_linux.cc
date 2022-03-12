#include "poseidon/os_thread.h"
#ifdef OS_IS_LINUX

namespace poseidon{
 class ThreadStartData{
     private:
      const char* name_;
      ThreadHandler handler_;
      uword parameter_;
     public:
      ThreadStartData(const char* name, const ThreadHandler& function, uword parameter):
        name_(strdup(name)),
        handler_(function),
        parameter_(parameter){}
      ~ThreadStartData(){
        if(name_){
          free((void*) name_);
        }
      }

      const char* GetName() const{
        return name_;
      }

      ThreadHandler& GetFunction(){
        return handler_;
      }

      ThreadHandler GetFunction() const{
        return handler_;
      }

      uword GetParameter() const{
        return parameter_;
      }
    };

    bool SetThreadName(const ThreadId& thread, const char* name){
      char truncated_name[kThreadNameMaxLength];
      snprintf(truncated_name, kThreadNameMaxLength-1, "%s", name);
      int result;
      if((result = pthread_setname_np(thread, truncated_name)) != 0){
        LOG(WARNING) << "couldn't set thread name: " << strerror(result);
        return false;
      }
      return true;
    }

    static void*
    HandleThread(void* pdata){
      auto data = (ThreadStartData*)pdata;
      auto& func = data->GetFunction();
      uword parameter = data->GetParameter();

      if(!SetThreadName(pthread_self(), data->GetName()) != 0)
        goto exit;

      func(parameter);
      exit:
      delete data;
      pthread_exit(nullptr);
    }

    ThreadId GetCurrentThreadId(){
      return pthread_self();
    }

    bool ThreadStart(ThreadId* thread, const char* name, const ThreadHandler& func, uword parameter){
      int result;
      pthread_attr_t attrs;
      if((result = pthread_attr_init(&attrs)) != 0){
        LOG(ERROR) << "couldn't initialize the thread attributes: " << strerror(result);
        return false;
      }

      DLOG(INFO) << "starting " << name << " thread w/ parameter: " << std::hex << parameter;
      auto data = new ThreadStartData(name, func, parameter);
      if((result = pthread_create(thread, &attrs, &HandleThread, data)) != 0){
        LOG(ERROR) << "couldn't start the thread: " << strerror(result);
        return false;
      }

      if((result = pthread_attr_destroy(&attrs)) != 0){
        LOG(ERROR) << "couldn't destroy the thread attributes: " << strerror(result);
        return false;
      }
      return true;
    }

    bool ThreadJoin(const ThreadId& thread){
      std::string thread_name = GetThreadName(thread);

      char return_data[kThreadMaxResultLength];

      int result;
      if((result = pthread_join(thread, (void**)&return_data)) != 0){
        LOG(ERROR) << "couldn't join thread: " << strerror(result);
        return false;
      }

      DLOG(INFO) << thread_name << " thread finished w/ result: " << std::string(return_data, kThreadMaxResultLength);
      return true;
    }

    int ThreadEquals(const ThreadId& lhs, const ThreadId& rhs){
      return pthread_equal(lhs, rhs);
    }

    std::string GetThreadName(const ThreadId& thread){
      char name[kThreadNameMaxLength];

      int err;
      if((err = pthread_getname_np(thread, name, kThreadNameMaxLength)) != 0){
        LOG(ERROR) << "cannot get name for " << thread << " thread: " << strerror(err);
        return "unknown";
      }
      return std::string(name);
    }
}

#endif//OS_IS_LINUX