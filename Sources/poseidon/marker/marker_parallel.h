#ifndef PSDN_MARKER_PARALLEL_H
#define PSDN_MARKER_PARALLEL_H

#include <random>
#include "poseidon/wsq.h"
#include "poseidon/marker/marker_visitor.h"

namespace poseidon {
 typedef int8_t QueueId;

 class ParallelMarkerTask;
 class ParallelMarker : public MarkerVisitor<true> {
  public:
   friend class ParallelMarkerTask;

   static constexpr const word kDefaultQueueSize = 1024;
  private:
   WorkStealingQueue<uword> queue_;

   ParallelMarkerTask** tasks_;
   word num_tasks_;

   std::default_random_engine engine_;
   std::uniform_int_distribution<QueueId> distribution_;

   inline WorkStealingQueue<uword>& queue() {
     return queue_;
   }

   inline bool HasWork() {
     return !queue().IsEmpty();
   }

   inline QueueId GetRandomQueueIndex() {
     return distribution_(engine_);
   }

   uword StealNextAddress();
  public:
   ParallelMarker() = delete;
   explicit ParallelMarker(Marker* marker);
   ParallelMarker(const ParallelMarker& rhs) = delete;
   ~ParallelMarker() override = default;

   void MarkAllRoots();

   ParallelMarker& operator=(const ParallelMarker& rhs) = delete;
 };
}

#endif //PSDN_MARKER_PARALLEL_H