#ifndef POSEIDON_MEMORY_REGION_H
#define POSEIDON_MEMORY_REGION_H

#include <cstring>
#include <ostream>
#include <glog/logging.h>

#include "utils.h"
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
    uword start_;
    uword size_;
   public:
    /**
     * Create an empty {@link MemoryRegion}.
     */
    MemoryRegion():
      start_(0),
      size_(0){
    }

    /**
     * Create a new {@link MemoryRegion} of a specific size.
     *
     * @param size The size of the new {@link MemoryRegion}
     */
    explicit MemoryRegion(uint64_t size);

    /**
     * Create a new {@link MemoryRegion} using the specified starting address & size.
     *
     * @param start The starting address of the {@link MemoryRegion}
     * @param size The size of the {@link MemoryRegion}
     */
    MemoryRegion(uword start, uword size):
      start_(start),
      size_(size){
    }

    /**
     * Create a {@link MemoryRegion} that is a sub-section of the parent region at a specific offset.
     *
     * @param parent The parent {@link MemoryRegion}
     * @param offset The offset in the parent {@link MemoryRegion}
     * @param size The size of the {@link MemoryRegion}
     */
    MemoryRegion(const MemoryRegion* parent, uint64_t offset, uint64_t size)://TODO: Refactor
      start_(0),
      size_(0){
      if(size >= parent->GetSize()){
        LOG(WARNING) << "cannot allocate MemoryRegion of " << HumanReadableSize(size) << ", size is larger than parent.";
        return;
      }
      auto start = parent->GetStartAddress() + offset;
      if(!parent->Contains(start)){
        DLOG(WARNING) << "cannot allocate MemoryRegion of " << HumanReadableSize(size) << " at offset " << offset << ", parent doesn't contain starting address: " << ((void*)start);
        return;
      }
#ifdef PSDN_DEBUG
      auto end = start + size;
      assert(parent->Contains(end));
#endif//PSDN_DEBUG
      start_ = start;
      size_ = size;
    }
    /**
     * Create a {@link MemoryRegion} that is a sub-section of the parent region.
     *
     * @param parent The parent {@link MemoryRegion}
     * @param size The size of the {@link MemoryRegion}
     */
    MemoryRegion(const MemoryRegion* parent, uint64_t size):
      MemoryRegion(parent, 0, size){
    }

    MemoryRegion(const MemoryRegion& rhs):
      start_(rhs.GetStartAddress()),
      size_(rhs.GetSize()){
    }
    ~MemoryRegion();

    uword GetSize() const{
      return size_;
    }

    uword GetStartAddress() const{
      return (uword)start_;
    }

    uword GetEndAddress() const{
      return GetStartAddress() + GetSize();
    }

    void* GetPointer() const{
      return (void*)start_;
    }

    bool Contains(const uword& addr) const{
      return addr >= GetStartAddress()
          && addr <= GetEndAddress();
    }

    void CopyFrom(const MemoryRegion& rhs) const{
      memcpy(GetPointer(), rhs.GetPointer(), rhs.GetSize());
    }

    const uint8_t* bytes_begin() const{
      return (uint8_t*)GetStartAddress();
    }

    const uint8_t* bytes_end() const{
      return (uint8_t*)GetEndAddress();
    }

    /**
     * Set the protection mode for the {@link MemoryRegion}
     *
     * @param mode The {@link ProtectionMode} for the {@link MemoryRegion}
     * @return true if the {@link ProtectionMode} was successfully set, false otherwise.
     */
    bool Protect(const ProtectionMode& mode) const;

    MemoryRegion& operator=(const MemoryRegion& rhs){
      if(this == &rhs)
        return *this;
      start_ = rhs.GetStartAddress();
      size_ = rhs.GetSize();
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream, const MemoryRegion& region){
      return stream << "MemoryRegion(start=" << region.GetStartAddress() << ", size=" << region.GetSize() << ")";
    }
  };
}

#endif //POSEIDON_MEMORY_REGION_H
