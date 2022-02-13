#ifndef POSEIDON_PLATFORM_H
#define POSEIDON_PLATFORM_H

#include <cstdint>

#if defined(_M_X64) || defined(__x86_64__)
#define ARCHITECTURE_IS_X64 1
#elif defined(_M_IX86) || defined(__i386__)
#define ARCHITECTURE_IS_X32 1
#elif defined(__ARMEL__)
#define ARCHITECTURE_IS_ARM 1
#elif defined(__aarch64__)
#define ARCHITECTURE_IS_ARM64 1
#endif

#if defined(__linux__) || defined(__FreeBSD__)
#define OS_IS_LINUX 1
#elif defined(__APPLE__)
#define OS_IS_OSX 1
#elif defined(_WIN32)
#define OS_IS_WINDOWS 1
#endif

namespace poseidon{
  static constexpr int kBitsPerByteLog2 = 3;
  static constexpr int kBitsPerByte = 1 << kBitsPerByteLog2;

  typedef uintptr_t uword;
  typedef intptr_t word;

  static constexpr uint64_t kWordSize = sizeof(word);
}

#endif //POSEIDON_PLATFORM_H