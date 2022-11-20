//#ifndef POSEIDON_SCAVENGER_H
//#define POSEIDON_SCAVENGER_H
//
//#include "poseidon/heap/zone.h"
//#include "poseidon/heap/heap.h"
//#include "poseidon/utils.h"
//#include "poseidon/task_pool.h"
//
//namespace poseidon{
// class ScavengerStats{
//   friend class Scavenger;
//  private:
//   AtomicTimestamp timestamp_;
//   AtomicLong duration_ms_;
//   AtomicPointerCounter scavenged_;
//   AtomicPointerCounter promoted_;
//   AtomicPointerCounter finalized_;
//
//   ScavengerStats(AtomicTimestamp& timestamp,
//                  AtomicLong& duration_ms,
//                  AtomicPointerCounter& scavenged,
//                  AtomicPointerCounter& promoted,
//                  AtomicPointerCounter& finalized):
//    timestamp_(timestamp),
//    duration_ms_(duration_ms),
//    scavenged_(scavenged),
//    promoted_(promoted),
//    finalized_(finalized){
//   }
//  public:
//   ScavengerStats():
//    timestamp_(),
//    duration_ms_(),
//    scavenged_(),
//    promoted_(),
//    finalized_(){
//   }
//   ScavengerStats(const ScavengerStats& rhs) = default;
//   ~ScavengerStats() = default;
//
//   Timestamp timestamp() const{
//     return (Timestamp)timestamp_;
//   }
//
//   int64_t duration_ms() const{
//     return (int64_t)duration_ms_;
//   }
//
//   AtomicPointerCounter scavenged() const{
//     return scavenged_;
//   }
//
//   AtomicPointerCounter promoted() const{
//     return promoted_;
//   }
//
//   AtomicPointerCounter finalized() const{
//     return finalized_;
//   }
//
//   ScavengerStats& operator=(const ScavengerStats& rhs) = default;
//
//   friend std::ostream& operator<<(std::ostream& stream, const ScavengerStats& val){
//     stream << "ScavengerStats(";
//     stream << "start=" << ", ";
//     stream << "duration=" << val.duration_ms() << "ms, ";
//     stream << "scavenged=" << val.scavenged() << ", ";
//     stream << "promoted=" << val.promoted() << ", ";
//     stream << "finalized=" << val.finalized();
//     stream << ")";
//     return stream;
//   }
// };
//
// class Semispace;
// class ScavengerTest {
//   template<bool Parallel>
//   friend class ScavengerVisitorBase;
//
//   friend class ParallelScavengerTask;
//   friend class SerialScavenger;
//
//   friend class ScavengerTest;
//  private:
//   static void SetScavenging(bool active=true);
//
//   static inline void
//   ClearScavenging(){
//     return SetScavenging(false);
//   }
//
//   static inline void
//   Clear(Semispace* semispace) {
//     semispace->Clear();
//   }
//
//   static void CopyObject(Pointer* src, Pointer* dst);
//   static void ForwardObject(Pointer* ptr, uword forwarding_address);
//   static uword PromoteObject(OldZone* zone, Pointer* ptr);
//   static uword ScavengeObject(Semispace* zone, Pointer* ptr);
//   static uword ProcessObject(Semispace* tospace, OldZone* old_zone, Pointer* ptr);
//  public:
//   Scavenger() = delete;
//   Scavenger(const Scavenger& rhs) = delete;
//   ~Scavenger() = delete;
//
//   static void Scavenge(Heap* heap = Heap::GetCurrentThreadHeap(), bool parallel = HasWorkers());
//
//   static bool IsScavenging();
//
//   static void SerialScavenge(Heap* heap);
//   static void ParallelScavenge(Heap* heap);
//
//   static ScavengerStats GetStats();
//   static Timestamp GetLastScavengeTimestamp();
//   static int64_t GetLastScavengeDurationMillis();
//   static int64_t GetNumberOfObjectsScavengedLastScavenge();
//   static int64_t GetNumberOfBytesScavengedLastScavenge();
//   static int64_t GetNumberOfObjectsPromotedLastScavenge();
//   static int64_t GetNumberOfBytesPromotedLastScavenge();
//
//   Scavenger& operator=(const Scavenger& rhs) = delete;
// };
//}
//
//#endif //POSEIDON_SCAVENGER_H
