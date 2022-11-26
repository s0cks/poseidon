#ifndef POSEIDON_REGION_H
#define POSEIDON_REGION_H

#define DEFINE_REGION_TYPE \
  public:                  \
    virtual uword GetStartingAddress() const; \
    virtual void* GetStartingAddressPointer() const { return (void*) GetStartingAddress(); } \
    virtual ObjectSize GetSize() const;       \
    virtual uword GetEndingAddress() const { return GetStartingAddress() + GetSize(); }      \
    virtual void* GetEndingAddressPointer() const { return (void*) GetEndingAddress(); }

#endif // POSEIDON_REGION_H