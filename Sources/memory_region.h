#ifndef POSEIDON_MEMORY_REGION_H
#define POSEIDON_MEMORY_REGION_H

#include <cstring>
#include <ostream>

#include "common.h"

namespace poseidon{
  class MemoryRegion{
   public:
    enum ProtectionMode{
      kNoAccess,
      kReadOnly,
      kReadWrite,
      kReadExecute,
      kReadWriteExecute,
    };

    inline friend std::ostream&
    operator<<(std::ostream& stream, const ProtectionMode& mode){
      switch(mode){
        case MemoryRegion::kNoAccess:
          return stream << "[n/a]";
        case MemoryRegion::kReadOnly:
          return stream << "[ro]";
        case MemoryRegion::kReadWrite:
          return stream << "[rw]";
        case MemoryRegion::kReadExecute:
          return stream << "[r+]";
        case MemoryRegion::kReadWriteExecute:
          return stream << "[rw+]";
        default:
          return stream << "[unknown]";
      }
    }
   private:
    void* data_;
    uword size_;
    bool owned_ : 1;

    MemoryRegion(bool owned):
      data_(nullptr),
      size_(0),
      owned_(owned){
    }
   public:
    MemoryRegion(void* data, uword size):
      data_(data),
      size_(size),
      owned_(false){
    }
    MemoryRegion():
      MemoryRegion(false){
    }
    explicit MemoryRegion(const uword& size);
    MemoryRegion(const MemoryRegion& rhs) = default;
    ~MemoryRegion();

    uword GetSize() const{
      return size_;
    }

    uword GetStartAddress() const{
      return (uword)data_;
    }

    uword GetEndAddress() const{
      return GetStartAddress() + GetSize();
    }

    void* GetPointer() const{
      return data_;
    }

    bool Contains(const uword& addr) const{
      return addr >= GetStartAddress()
          && addr <= GetEndAddress();
    }

    void CopyFrom(const MemoryRegion& rhs) const{
      memcpy(GetPointer(), rhs.GetPointer(), rhs.GetSize());
    }

    bool Protect(const ProtectionMode& mode) const;

    MemoryRegion SubRegion(const uword offset, const uword size) const{
      if((offset + size) > GetSize())
        return {};//TODO: better default
      char* ptr = (char*)data_;
      return {&ptr[offset], size};//TODO: bounds checking
    }

    MemoryRegion& operator=(const MemoryRegion& rhs) = default;

    friend std::ostream& operator<<(std::ostream& stream, const MemoryRegion& region){
      return stream << "MemoryRegion(start=" << region.GetStartAddress() << ", size=" << region.GetSize() << ")";
    }
  };
}

#endif //POSEIDON_MEMORY_REGION_H
