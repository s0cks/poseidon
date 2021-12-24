#ifndef POSEIDON_UTILS_H
#define POSEIDON_UTILS_H

#include <cstdint>
#include <cstdio>
#include <cmath>
#include <cstring>

#include "platform.h"

namespace poseidon{
  static const uword kUWordOne = 1U;

  class BitVector{
   public:
    static const uint64_t kBitsPerByte = 8;
    static const uint64_t kBitsPerWord = sizeof(word) * 8;
   private:
    uint64_t size_;
    uint64_t asize_;
    uword* data_;
   public:
    explicit BitVector(const uint64_t& size):
      size_(size),
      asize_(1 + ((size - 1) / kBitsPerWord)),
      data_(nullptr){
      data_ = (uword*)(malloc(sizeof(uword)*size));
      Clear();
    }
    BitVector(const BitVector& rhs):
      size_(rhs.size_),
      asize_(rhs.asize_),
      data_(new uword[rhs.size_]){
      memcpy(data_, rhs.data_, sizeof(uword)*rhs.size_);
    }
    ~BitVector(){
      delete[] data_;
    }

    void Clear(){
      for(int idx = 0; idx < asize_; idx++)
        data_[idx] = 0x0;
    }

    void Add(word i){
      data_[i / kBitsPerWord] |= static_cast<uword>(1 << (i % kBitsPerWord));
    }

    void Remove(word i){
      data_[i / kBitsPerWord] &= ~static_cast<uword>(1 << (i % kBitsPerWord));
    }

    void Intersect(const BitVector& rhs){
      for(int i = 0; i < asize_; i++)
        data_[i] = data_[i] & rhs[i];
    }

    uword operator[](const int& i) const{
      return data_[i];
    }

    bool Contains(word i) const{
      uword block = data_[i / kBitsPerWord];
      return (block & static_cast<uword>(1 << (i % kBitsPerWord)));
    }

    bool AddAll(const BitVector& rhs){
      bool changed = false;
      for(int i = 0; i < asize_; i++){
        uword before = data_[i];
        uword after =data_[i] |= rhs[i];
        if(before != after){
          changed = true;
          data_[i] = after;
        }
      }
      return changed;
    }
  };

  template<typename S, typename T, int Position, int Size=(sizeof(S)*kBitsPerByte) - Position>
  class BitField{
   public:
    static S Mask(){
      return (kUWordOne << Size) - 1;
    }

    static S MaskInPlace(){
      return Mask() << Position;
    }

    static T Decode(S val){
      const auto u = static_cast<typename std::make_unsigned<S>::type>(val);
      return static_cast<T>((u >> Position) & Mask());
    }

    static S Encode(T val){
      const auto u = static_cast<typename std::make_unsigned<S>::type>(val);
      return (u & Mask()) << Position;
    }

    static S Update(T val, S original){
      return Encode(val) | (~MaskInPlace() & original);
    }
  };

  static inline const char*
  HumanReadableSize(uint64_t nbytes){
    static const char* kSuffix[] = {
        "b",
        "kb",
        "mb",
        "gb",
        "tb",
    };
    static int kSuffixLength = sizeof(kSuffix) / sizeof(kSuffix[0]);

    uint8_t suffix = 0;
    double remainder = static_cast<double>(nbytes);

    while(remainder >= 1024 && suffix < kSuffixLength){
      suffix++;
      remainder /= 1024;
    }

    static char result[128];
    if(remainder - floor(remainder) == 0.0){
      sprintf(result, "%d%s", (int)remainder, kSuffix[suffix]);
    } else{
      sprintf(result, "%.2lf%s", remainder, kSuffix[suffix]);
    }

    return result;
  }
}

#endif //POSEIDON_UTILS_H
