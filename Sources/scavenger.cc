#include <deque>
#include <glog/logging.h>

#include "utils.h"
#include "sweeper.h"
#include "scavenger.h"
#include "allocator.h"

#include "finalizer.h"
#include "live_object_marker.h"
#include "live_object_promoter.h"
#include "live_object_forwarder.h"

namespace poseidon{
  void Scavenger::UpdateForwarding(){//TODO: make portable between heaps
    DLOG(INFO) << "updating live object pointers....";
    LiveObjectForwarder forwarder;
    Allocator::VisitLocals(&forwarder);//TODO: visit classes and other runtime values
  }

  class CollectionStats{
   private:
    Heap::HeapStats& start_;
    Timestamp& start_ts_;
    Heap::HeapStats& finished_;
    Timestamp& finished_ts_;
   public:
    CollectionStats(Heap::HeapStats& start, Timestamp& start_ts,
                    Heap::HeapStats& finished, Timestamp& finished_ts):
                    start_(start),
                    start_ts_(start_ts),
                    finished_(finished),
                    finished_ts_(finished_ts){
    }
    ~CollectionStats() = default;

    Heap::HeapStats& GetStartStats() const{
      return start_;
    }

    Timestamp& GetStartTimestamp() const{
      return start_ts_;
    }

    Heap::HeapStats& GetFinishedStats() const{
      return finished_;
    }

    Timestamp& GetFinishedTimestamp() const{
      return finished_ts_;
    }

    Duration GetDuration() const{
      return (GetFinishedTimestamp() - GetStartTimestamp());
    }

    uint64_t GetFreedBytes() const{
      auto allocated_before = GetStartStats().GetAllocatedBytes();
      auto allocated_after = GetFinishedStats().GetAllocatedBytes();
      return allocated_before - allocated_after;
    }

    uint64_t GetTotalBytes() const{
      return GetStartStats().GetTotalBytes();
    }

    double GetFreedPercentage() const{
      return GetPercentageOf(GetFreedBytes(), GetTotalBytes());
    }
  };

  void Scavenger::MinorCollection(){
    std::deque<uword> work;

    // only collect from the eden heap
    DLOG(INFO) << "executing minor collection....";
    auto start_ts = Clock::now();
    auto num_allocated = Allocator::GetNumberOfObjectsAllocated();
    auto start_stats = Allocator::GetEdenHeap()->GetStats();

    DLOG(INFO) << "marking live objects....";
    LiveObjectMarker marker(work);
    marker.MarkLiveObjects();

    DLOG(INFO) << "scavenging live objects....";
    LiveObjectPromoter promoter;
    Allocator::GetEdenHeap()->VisitRawObjectPointers(&promoter);

    Finalizer finalizer;
    Allocator::GetEdenHeap()->GetFromSpace().VisitRawObjectPointers(&finalizer);

    UpdateForwarding();

    GenerationalSweeper::SweepHeap(Allocator::GetEdenHeap());

    auto finished_ts = Clock::now();
    auto finished_stats = Allocator::GetEdenHeap()->GetStats();
    CollectionStats stats(start_stats, start_ts, finished_stats, finished_ts);
    DLOG(INFO) << "finished minor collection. (" << stats.GetDuration() << ")";
    DLOG(INFO) << "minor collection stats:";
    DLOG(INFO) << " - duration: " << stats.GetDuration();
    DLOG(INFO) << " - freed: " << HumanReadableSize(stats.GetFreedBytes()) << "/" << HumanReadableSize(stats.GetTotalBytes()) << " (" << PrettyPrintPercentage(stats.GetFreedPercentage()) << ")";
    DLOG(INFO) << " - starting utilization: " << start_stats;
    DLOG(INFO) << " - current utilization: " << finished_stats;
    DLOG(INFO) << " - marked objects: " << marker.GetMarked() << "/" << num_allocated
        << " (" << PrettyPrintPercentage(GetPercentageOf(marker.GetMarked(), num_allocated)) << ")";
    DLOG(INFO) << " - scavenged objects: " << promoter.GetNumberOfScavengedObjects()
        << " (" << PrettyPrintPercentage(GetPercentageOf(promoter.GetNumberOfScavengedObjects(), num_allocated)) << ")";
    DLOG(INFO) << " - promoted objects: " << promoter.GetNumberOfPromotedObjects()
        << " (" << PrettyPrintPercentage(GetPercentageOf(promoter.GetNumberOfPromotedObjects(), num_allocated)) << ")";
    DLOG(INFO) << " - finalized objects: " << finalizer.GetNumberOfObjectsFinalized() << "/" << num_allocated
        << " (" << PrettyPrintPercentage(GetPercentageOf(finalizer.GetNumberOfObjectsFinalized(), num_allocated)) << ")";
  }

  void Scavenger::MajorCollection(){
    // collect from all heaps
  }
}