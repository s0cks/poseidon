#ifndef POSEIDON_BITSET_H
#define POSEIDON_BITSET_H

#include <iostream>
#include "poseidon/common.h"
#include "poseidon/platform.h"

namespace poseidon{
 class BitSet{
  private:
   static inline std::ostream&
   PrintBits(std::ostream& stream, uword val){
     for(auto idx = 0; idx < kBitsPerWord; idx++){
       auto mask = (static_cast<uword>(1) << (idx & (kBitsPerWord - 1)));
       stream << ((val & mask) != 0 ? "1" : "0");
     }
     return stream;
   }

   static inline int64_t
   GetLengthInWords(int64_t size){
     return 1 + ((size - 1) / kBitsPerWord);
   }

   static inline int64_t
   GetLengthInBits(int64_t size){//TODO: this function is bad and makes terrible assumptions...good luck
     return size * kBitsPerWord;
   }
  private:
   uword* words_;
   int64_t num_words_;

   inline void
   Resize(int64_t size){
     if(BitSet::size() == size)
       return;
     auto nwords = GetLengthInWords(size);
     words_ = (uword*)realloc(words_, sizeof(uword) * nwords);
     num_words_ = nwords;
   }
  public:
   BitSet():
    words_(nullptr),
    num_words_(0){
   }
   explicit BitSet(int64_t size):
    words_(nullptr),
    num_words_(0){
     Resize(size);
   }
   ~BitSet() = default;

   void Set(int64_t idx, bool val){
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx < GetLengthInBits(size()));
     uword mask = (static_cast<uword>(1) << (idx & (kBitsPerWord - 1)));
     if(val){
       words_[idx >> kBitsPerWordLog2] |= mask;
     } else{
       words_[idx >> kBitsPerWordLog2] &= ~mask;
     }
   }

   bool Test(int64_t idx) const{
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= GetLengthInBits(size()));
     uword mask = (static_cast<uword>(1) << (idx & (kBitsPerWord - 1)));
     return (words_[idx >> kBitsPerWordLog2] & mask) != 0;
   }

   void Reset(){
     PSDN_ASSERT(words_ != nullptr);
     memset(words_, 0, sizeof(uword) * size());
   }

   int64_t size() const{
     return num_words_;
   }

   uword* data() const{
     return (uword*)&words_[0];
   }

   uword* begin() const{
     return data();
   }

   uword* end() const{
     return data() + size();
   }

   uword operator[](int64_t idx) const{
     return words_[idx];
   }

   friend std::ostream& operator<<(std::ostream& stream , const BitSet& val){
     for(auto idx = 0; idx < val.size(); idx++){
       PrintBits(stream, val[idx]) << " ";
       if(((idx + 1) % 2) == 0)
         stream << std::endl;
     }
     return stream;
   }
 };
}

#endif //POSEIDON_BITSET_H