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
    int64_t size_;

    inline void
    CopyFrom(const MemoryRegion& rhs) const{//TODO: size check / refactor
      memcpy(GetStartingAddressPointer(), rhs.GetStartingAddressPointer(), rhs.size());
    }
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
    explicit MemoryRegion(int64_t size);

    /**
     * Create a new {@link MemoryRegion} using the specified starting address & size.
     *
     * @param start The starting address of the {@link MemoryRegion}
     * @param size The size of the {@link MemoryRegion}
     */
    MemoryRegion(uword start, int64_t size):
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
    MemoryRegion(const MemoryRegion* parent, int64_t offset, int64_t size)://TODO: Refactor
      start_(0),
      size_(0){
      if(size >= parent->size()){
        LOG(WARNING) << "cannot allocate MemoryRegion of " << Bytes(size) << ", size is larger than parent.";
        return;
      }

      auto start = parent->GetStartingAddress() + offset;
      if(!parent->Contains(start)){
        DLOG(WARNING) << "cannot allocate MemoryRegion of " << Bytes(size) << " at offset " << offset << ", parent doesn't contain starting address: " << ((void*)start);
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
    MemoryRegion(const MemoryRegion* parent, int64_t size):
      MemoryRegion(parent, 0, size){
    }

    MemoryRegion(const MemoryRegion& rhs):
      start_(rhs.GetStartingAddress()),
      size_(rhs.size()){
    }
    virtual ~MemoryRegion();

    int64_t size() const{
      return size_;
    }

    uword GetStartingAddress() const{
      return start_;
    }

    void* GetStartingAddressPointer() const{
      return (void*)GetStartingAddress();
    }

    uword GetEndingAddress() const{
      return GetStartingAddress() + size();
    }

    void* GetEndingAddressPointer() const{
      return (void*)GetEndingAddress();
    }

    bool Contains(uword address) const{
      return GetStartingAddress() <= address
          && GetEndingAddress() >= address;
    }

    /**
     * Set the protection mode for the {@link MemoryRegion}
     *
     * @param mode The {@link ProtectionMode} for the {@link MemoryRegion}
     * @return true if the {@link ProtectionMode} was successfully set, false otherwise.
     */
    virtual bool Protect(const ProtectionMode& mode) const;

    MemoryRegion& operator=(const MemoryRegion& rhs){
      if(this == &rhs)
        return *this;
      start_ = rhs.GetStartingAddress();
      size_ = rhs.size();
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream, const MemoryRegion& region){
      return stream << "MemoryRegion(start=" << region.GetStartingAddressPointer() << ", size=" << Bytes(region.size()) << ", end=" << region.GetEndingAddressPointer() << ")";
    }
  };
}

#endif //POSEIDON_MEMORY_REGION_H
