#ifndef PSDN_MEMORY_REGION_H
#define PSDN_MEMORY_REGION_H

#include <cstring>
#include <ostream>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/region.h"
#include "poseidon/platform/platform.h"

namespace poseidon{
  class MemoryRegion : public Region {
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
    inline void
    CopyFrom(const MemoryRegion& rhs) const{//TODO: size check / refactor
      memcpy(GetStartingAddressPointer(), rhs.GetStartingAddressPointer(), rhs.GetSize());
    }
   public:
    /**
     * Create an empty {@link MemoryRegion}.
     */
    MemoryRegion() = default;

    /**
     * Create a new {@link MemoryRegion} of a specific size.
     *
     * @param size The size of the new {@link MemoryRegion}
     */
    explicit MemoryRegion(word size, ProtectionMode mode = ProtectionMode::kNoAccess);

    /**
     * Create a new {@link MemoryRegion} using the specified starting address & size.
     *
     * @param start The starting address of the {@link MemoryRegion}
     * @param size The size of the {@link MemoryRegion}
     */
    MemoryRegion(uword start, word size):
      Region(start, size) {
    }

    /**
     * Create a {@link MemoryRegion} that is a sub-section of the parent region at a specific offset.
     *
     * @param parent The parent {@link MemoryRegion}
     * @param offset The offset in the parent {@link MemoryRegion}
     * @param size The size of the {@link MemoryRegion}
     */
    MemoryRegion(const MemoryRegion* parent, word offset, word size);

    /**
     * Create a {@link MemoryRegion} that is a sub-section of the parent region.
     *
     * @param parent The parent {@link MemoryRegion}
     * @param size The size of the {@link MemoryRegion}
     */
    MemoryRegion(const MemoryRegion* parent, word size):
      MemoryRegion(parent, 0, size){
    }

    MemoryRegion(const MemoryRegion& rhs) = default;
    ~MemoryRegion() override = default;

    virtual void FreeRegion();
    virtual void ClearRegion();

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
      size_ = rhs.GetSize();
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream, const MemoryRegion& region){
      return stream << "MemoryRegion(start=" << region.GetStartingAddressPointer() << ", size=" << Bytes(region.GetSize()) << ", end=" << region.GetEndingAddressPointer() << ")";
    }

    static inline MemoryRegion
    Subregion(const MemoryRegion& region, word offset, word size) {
      auto starting_address = region.GetStartingAddress() + offset;
      auto ending_address = starting_address + size;
      if(!region.Contains(starting_address) || !region.Contains(ending_address))
        return {};
      return {starting_address, size};
    }
  };
}

#endif //PSDN_MEMORY_REGION_H
