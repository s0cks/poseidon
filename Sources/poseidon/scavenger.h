#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/zone.h"
#include "poseidon/heap.h"
#include "poseidon/stats.h"
#include "poseidon/utils.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 class ScavengerStats{
   friend class Scavenger;
  private:
   AtomicTimestamp timestamp_;
   AtomicLong duration_ms_;
   AtomicPointerCounter scavenged_;
   AtomicPointerCounter promoted_;

   ScavengerStats(AtomicTimestamp& timestamp, AtomicLong& duration_ms, AtomicPointerCounter& scavenged, AtomicPointerCounter& promoted):
    timestamp_(timestamp),
    duration_ms_(duration_ms),
    scavenged_(scavenged),
    promoted_(promoted){
   }
  public:
   ScavengerStats():
    timestamp_(),
    duration_ms_(),
    scavenged_(),
    promoted_(){
   }
   ScavengerStats(const ScavengerStats& rhs) = default;
   ~ScavengerStats() = default;

   Timestamp timestamp() const{
     return (Timestamp)timestamp_;
   }

   int64_t duration_ms() const{
     return (int64_t)duration_ms_;
   }

   AtomicPointerCounter scavenged() const{
     return scavenged_;
   }

   AtomicPointerCounter promoted() const{
     return promoted_;
   }

   ScavengerStats& operator=(const ScavengerStats& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const ScavengerStats& val){
     stream << "ScavengerStats(";
     stream << "start=" << ", ";
     stream << "duration=" << val.duration_ms() << "ms, ";
     stream << "scavenged=" << val.scavenged() << ", ";
     stream << "promoted=" << val.promoted();
     stream << ")";
     return stream;
   }
 };

 class Scavenger{
   friend class ParallelScavengerTask;
   friend class SerialScavenger;
  private:
   static void SetScavenging(bool active=true);

   static inline void
   ClearScavenging(){
     return SetScavenging(false);
   }
  public:
   Scavenger() = delete;
   Scavenger(const Scavenger& rhs) = delete;
   ~Scavenger() = delete;

   static void Scavenge();
   static bool IsScavenging();
   static void SerialScavenge();
   static void ParallelScavenge();

   static ScavengerStats GetStats();
   static Timestamp GetLastScavengeTimestamp();
   static int64_t GetLastScavengeDurationMillis();
   static int64_t GetNumberOfObjectsScavengedLastScavenge();
   static int64_t GetNumberOfBytesScavengedLastScavenge();
   static int64_t GetNumberOfObjectsPromotedLastScavenge();
   static int64_t GetNumberOfBytesPromotedLastScavenge();

   Scavenger& operator=(const Scavenger& rhs) = delete;
 };
}

#endif //POSEIDON_SCAVENGER_H
