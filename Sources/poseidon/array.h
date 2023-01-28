#ifndef PSDN_ARRAY_H
#define PSDN_ARRAY_H

#include "poseidon/utils.h"
#include "poseidon/platform/platform.h"

namespace poseidon {
 template<typename T>
 class Array {
  public:
   static const constexpr word kDefaultInitialCapacity = 8;
  protected:
   word length_;
   word capacity_;
   T* data_;

   void Resize(const word new_len) {
     if(new_len > Capacity()) {
       auto new_cap = static_cast<word>(RoundUpPowTwo(new_len));
       auto new_data = reinterpret_cast<T*>(realloc(data_, new_cap * sizeof(T)));
       LOG_IF(FATAL, !new_data) << "failed to realloc(data_)";
       capacity_ = new_cap;
       data_ = new_data;
     }
     length_ = new_len;
   }
  public:
   explicit Array(const word init_cap = kDefaultInitialCapacity):
    length_(0),
    capacity_(0),
    data_(nullptr) {
     auto new_cap = static_cast<word>(RoundUpPowTwo(init_cap));
     auto new_data = reinterpret_cast<T*>(malloc(new_cap * sizeof(T)));
     LOG_IF(FATAL, !new_data) << "failed to malloc(data_)";
     capacity_ = new_cap;
     data_ = new_data;
   }
   ~Array() {
     if(data_)
       free(data_);
   }

   word Length() const {
     return length_;
   }

   word Capacity() const {
     return capacity_;
   }

   T& operator[](const word index) const {
     return data_[index];
   }

   T& Last() const {
     return operator[](Length() - 1);
   }

   T& Pop() {
     T& result = Last();
     length_--;
     return result;
   }

   void Add(const T& value) {
     Resize(Length() + 1);
     Last() = value;
   }

   void Clear() {
     length_ = 0;
   }

   bool IsEmpty() const {
     return Length() == 0;
   }
 };
}

#endif //PSDN_ARRAY_H