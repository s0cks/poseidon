#include "memory_region.h"
#if defined(OS_IS_LINUX)

#include <sys/mman.h>
#include <glog/logging.h>
#include "utils.h"

#undef MAP_FAILED
#define MAP_FAILED reinterpret_cast<void*>(-1)

namespace poseidon{
  MemoryRegion::MemoryRegion(const uword& size):
    MemoryRegion(true){
    void* addr = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0);
    if(addr == MAP_FAILED){
      LOG(ERROR) << "failed to mmap memory region of " << size << " bytes: " << strerror(errno);
      return;
    }

    data_ = addr;
    size_ = size;
  }

  MemoryRegion::~MemoryRegion(){
    if(owned_ && data_ != nullptr && size_ > 0){
      int err;
      if((err = munmap(data_, size_)) != 0){
        LOG(ERROR) << "failed to munmap memory region of " << size_ << " bytes: " << strerror(err);
      }
      DLOG(INFO) << "freed MemoryRegion (" << HumanReadableSize(size_) << ")";
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

    int err;
    if((err = mprotect(GetPointer(), GetSize(), protection)) != 0){
      LOG(ERROR) << "failed to " << mode << " protect memory region of " << GetSize() << " bytes @" << GetPointer() << ": " << strerror(err);
      return false;
    }
    return true;
  }
}

#endif//OS_IS_LINUX