#ifndef POSEIDON_STATS_H
#define POSEIDON_STATS_H

#include "poseidon/utils.h"
#include "poseidon/platform.h"
#include "poseidon/relaxed_atomic.h"

namespace poseidon{
 class MinorCollectionStats{
   friend class Scavenger;
  private:
   uword start_;
   int64_t size_;

   RelaxedAtomic<int64_t> num_scavenged_;
   RelaxedAtomic<int64_t> bytes_scavenged_;

   RelaxedAtomic<int64_t> num_promoted_;
   RelaxedAtomic<int64_t> bytes_promoted_;

   RelaxedAtomic<int64_t> num_finalized_;
   RelaxedAtomic<int64_t> bytes_finalized_;
  public:
   MinorCollectionStats(uword start, int64_t size):
    start_(start),
    size_(size),
    num_scavenged_(),
    bytes_scavenged_(),
    num_promoted_(),
    bytes_promoted_(),
    num_finalized_(),
    bytes_finalized_(){
   }
   MinorCollectionStats():
     MinorCollectionStats(0, 0){
   }
   MinorCollectionStats(const MinorCollectionStats& rhs) = default;
   ~MinorCollectionStats() = default;

   uword start() const{
     return start_;
   }

   int64_t size() const{
     return size_;
   }

   int64_t num_scavenged() const{
     return (int64_t)num_scavenged_;
   }

   int64_t bytes_scavenged() const{
     return (int64_t)bytes_scavenged_;
   }

   int64_t num_promoted() const{
     return (int64_t)num_promoted_;
   }

   int64_t bytes_promoted() const{
     return (int64_t)bytes_promoted_;
   }

   int64_t num_finalized() const{
     return (int64_t)num_finalized_;
   }

   int64_t bytes_finalized() const{
     return (int64_t)bytes_finalized_;
   }

   double GetPercentageOfBytesScavenged() const{
     return GetPercentageOf(bytes_scavenged(), size());
   }

   double GetPercentageOfBytesPromoted() const{
     return GetPercentageOf(bytes_promoted(), size());
   }

   double GetPercentageOfBytesFinalized() const{
     return GetPercentageOf(bytes_finalized(), size());
   }

   MinorCollectionStats& operator=(const MinorCollectionStats& rhs) = default;
 };
}

#endif//POSEIDON_STATS_H