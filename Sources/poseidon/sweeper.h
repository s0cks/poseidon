#ifndef POSEIDON_SWEEPER_H
#define POSEIDON_SWEEPER_H

#include "poseidon/heap/zone.h"
#include "poseidon/utils.h"
#include "poseidon/common.h"
#include "poseidon/freelist.h"

namespace poseidon{
 class SweeperStats{
   friend class Sweeper;
  private:
   Timestamp start_ts_;
   int64_t duration_ms_;

   int64_t num_finalized_;
   int64_t bytes_finalized_;
   double frag_perc_;

   SweeperStats(Timestamp ts, int64_t duration_ms, double frag_percentage, int64_t num_finalized, int64_t bytes_finalized):
     start_ts_(ts),
     duration_ms_(duration_ms),
     frag_perc_(frag_percentage),
     num_finalized_(num_finalized),
     bytes_finalized_(bytes_finalized){
   }
  public:
   SweeperStats():
    start_ts_(Clock::now()),
    duration_ms_(0),
    frag_perc_(0),
    num_finalized_(0),
    bytes_finalized_(0){
   }
   SweeperStats(const SweeperStats& rhs) = default;
   ~SweeperStats() = default;

   Timestamp start_ts() const{
     return start_ts_;
   }

   int64_t duration_ms() const{
     return duration_ms_;
   }

   int64_t num_finalized() const{
     return num_finalized_;
   }

   int64_t bytes_finalized() const{
     return bytes_finalized_;
   }

   double fragmentation_perc() const{
     return frag_perc_;
   }

   SweeperStats& operator=(const SweeperStats& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const SweeperStats& val){
     stream << "SweeperStats(";//TODO: add start_ts
     stream << "duration=" << val.duration_ms() << "ms, ";
     stream << "finalized=" << val.num_finalized() << " (" << Bytes(val.bytes_finalized()) << "), ";
     stream << "fragmentation=" << PrettyPrintPercentage(val.fragmentation_perc());
     stream << ")";
     return stream;
   }
 };

 class OldZone;
 class Sweeper{
   friend class SerialSweeper;
   friend class ParallelSweeperTask;
  private:
   static void SetSweeping(bool active=true);

   static inline void
   ClearSweeping(){
     SetSweeping(false);
   }

   static void SerialSweep(OldZone* old_zone);
   static void ParallelSweep(OldZone* old_zone);
  public:
   Sweeper() = delete;
   Sweeper(const Sweeper& rhs) = delete;
   ~Sweeper() = delete;

   static void Sweep();
   static bool IsSweeping();

   static Timestamp GetLastSweepTimestamp();
   static int64_t GetLastSweepDurationMillis();
   static double GetLastSweepFragmentationPercentage();
   static int64_t GetNumberOfBytesLastSweep();
   static int64_t GetNumberOfObjectsLastSweep();

   static SweeperStats GetStats(){
     return { GetLastSweepTimestamp(),
              GetLastSweepDurationMillis(),
              GetLastSweepFragmentationPercentage(),
              GetNumberOfObjectsLastSweep(),
              GetNumberOfBytesLastSweep() };
   }

   Sweeper& operator=(const Sweeper& rhs) = delete;
 };
}

#endif//POSEIDON_SWEEPER_H