#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

namespace poseidon{
 class Heap;
 class Marker{
   template<bool Parallel>
   friend class MarkerVisitorBase;

   friend class ParallelMarkTask;
  private:
   RelaxedAtomic<int64_t> marked_;

   inline void MarkObject(RawObject* ptr){
     DLOG(INFO) << "marking " << ptr->ToString() << ".";
     marked_ += 1;
     ptr->SetMarkedBit();
   }
  public:
   Marker() = default;
   Marker(const Marker& rhs) = delete;
   ~Marker() = default;

   void MarkLiveObjects();

   Marker& operator=(const Marker& rhs) = delete;
 };
}

#endif //POSEIDON_MARKER_H