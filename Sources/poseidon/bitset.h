#ifndef POSEIDON_BITSET_H
#define POSEIDON_BITSET_H

#ifdef PSDN_DEBUG
#include <cassert>
#endif//PSDN_DEBUG

#include "poseidon/platform.h"

namespace poseidon{
 class BitSet{
  private:
   uword* words_;
   int64_t num_words_;

   inline void Resize(int64_t num_words){//TODO: make public
     if(num_words > 0){
       words_ = (uword*)malloc(sizeof(uword) * num_words);
       num_words_ = num_words;
     }
   }

   static inline std::ostream&
   PrintBits(std::ostream& stream, uword val){
     for(auto idx = 0; idx < kBitsPerWord; idx++){
       bool bit = (val & static_cast<uword>(1 << (idx % kBitsPerWord)));
       stream << (bit ? '1' : '0');
     }
     return stream;
   }
  public:
   explicit BitSet(int64_t num_bits):
    words_(nullptr),
    num_words_(0){
     assert(num_bits >= kBitsPerWord);
     assert(num_bits % kBitsPerWord == 0);
     Resize(num_bits / kBitsPerWord);
     ClearAll();
   }
   ~BitSet(){
     if(words_)
       free(words_);
   }

   bool Get(int64_t idx) const{
     uword blk = words_[idx / kBitsPerWord];
     return (blk & static_cast<uword>(1 << (idx % kBitsPerWord))) != 0;
   }

   void Set(int64_t idx){
     words_[idx / kBitsPerWord] |= static_cast<uword>(1 << (idx % kBitsPerWord));
   }

   void Clear(int64_t idx){
     words_[idx / kBitsPerWord] &= ~static_cast<uword>(1 << (idx % kBitsPerWord));
   }

   void ClearAll(){
     memset(words_, 0, sizeof(uword) * num_words_);
   }

   void Intersect(const BitSet& rhs){
     assert(size() == rhs.size());
     for(auto idx = 0; idx < rhs.size(); idx++)
       words_[idx] &= rhs[idx];
   }

   bool AddAll(const BitSet& rhs){
     assert(size() == rhs.size());
     bool changed = false;

     for(auto idx = 0; idx < rhs.size(); idx++){
       uword before = words_[idx];
       uword after = words_[idx] | rhs[idx];
       if(before != after){
         changed = true;
         words_[idx] = after;
       }
     }

     return changed;
   }

   int64_t size() const{
     return num_words_;
   }

   uword* words() const{
     return words_;
   }

   uword* begin() const{
     return words();
   }

   uword* end() const{
     return words() + size();
   }

   uword operator[](size_t windex) const{
     return words_[windex];
   }

   friend std::ostream& operator<<(std::ostream& stream , const BitSet& val){
     int idx = 0;
     for(auto w : val){
       PrintBits(stream, w) << " ";
       if(++idx >= 2){
         stream << std::endl;
         idx = 0;
       }
     }
     return stream;
   }
 };
}

#endif //POSEIDON_BITSET_H