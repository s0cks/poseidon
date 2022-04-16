#ifndef POSEIDON_OS_THREAD_LINUX_H
#define POSEIDON_OS_THREAD_LINUX_H

#ifndef POSEIDON_OS_THREAD_H
#error "Please #include <poseidon/os_thread.h> directly instead."
#endif//POSEIDON_OS_THREAD_H

#include <pthread.h>
#include "poseidon/platform.h"

#ifndef PTHREAD_KEYS_MAX
#define PTHREAD_KEYS_MAX 16
#endif//PTHREAD_KEYS_MAX

namespace poseidon{
 static const int kThreadNameMaxLength = 16;
 static const int kThreadMaxResultLength = 128;

 typedef pthread_key_t ThreadLocalKey;
 typedef pthread_t ThreadId;
 typedef void (*ThreadHandler)(uword parameter);
}

#endif //POSEIDON_OS_THREAD_LINUX_H
