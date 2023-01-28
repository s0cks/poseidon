#include "memory_region.h"
#ifdef OS_IS_OSX

#include <sys/mman.h>
#include <glog/logging.h>

#include "poseidon/utils.h"

#undef MAP_FAILED
#define MAP_FAILED reinterpret_cast<void*>(-1)

namespace poseidon{
  MemoryRegion::MemoryRegion(const word size, const ProtectionMode mode):
    MemoryRegion(){
    void* addr = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0);
    LOG_IF(FATAL, addr == MAP_FAILED) << "failed to mmap MemoryRegion of " << Bytes(size) << ": " << strerror(errno);
    start_ = (uword)addr;
    size_ = size;
    DLOG(INFO) << "allocated " << (*this);
    Protect(mode);
  }

  void MemoryRegion::FreeRegion(){
    if(size_ > 0){
      int err = munmap(GetStartingAddressPointer(), GetSize());
      LOG_IF(FATAL, err != 0)  << "failed to munmap MemoryRegion of " << Bytes(GetSize()) << ": " << strerror(err);
      DLOG(INFO) << "freed MemoryRegion (" << Bytes(GetSize()) << ")";
    }
  }

  bool MemoryRegion::Protect(const ProtectionMode& mode) const{
    int protection = 0;
    switch(mode){
      case kNoAccess:
        protection = PROT_NONE;
        break;
      case kReadOnly:
        protection = PROT_READ;
        break;
      case kReadWrite:
        protection = PROT_READ|PROT_WRITE;
        break;
      case kReadExecute:
        protection = PROT_READ|PROT_EXEC;
        break;
      case kReadWriteExecute:
        protection = PROT_READ|PROT_WRITE|PROT_EXEC;
        break;
    }
    int err = mprotect(GetStartingAddressPointer(), GetSize(), protection);
    LOG_IF(FATAL, err != 0) << "failed to set ProtectionMode `" << mode << "` to " << (*this) << ": " << strerror(err);
    return true;
  }
}

#endif//OS_IS_OSX