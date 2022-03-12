#ifndef POSEIDON_OS_THREAD_OSX_H
#define POSEIDON_OS_THREAD_OSX_H

#include <pthread.h>
#include "poseidon/platform.h"

namespace poseidon{
 static const int kThreadNameMaxLength = 16;
 static const int kThreadMaxResultLength = 128;

 typedef pthread_key_t ThreadLocalKey;
 typedef pthread_t ThreadId;
 typedef void (*ThreadHandler)(uword parameter);
}

#endif //POSEIDON_OS_THREAD_OSX_H
