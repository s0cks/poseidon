#ifndef POSEIDON_SWEEPER_STATS_H
#define POSEIDON_SWEEPER_STATS_H

namespace poseidon {
 class SweeperStats {
   friend class Sweeper;
  protected:
   SweeperStats() = default;
  public:
   SweeperStats(const SweeperStats& rhs) = default;
   ~SweeperStats() = default;

   SweeperStats& operator=(const SweeperStats& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const SweeperStats& value) {
     stream << "SweeperStats(";
     NOT_IMPLEMENTED(ERROR);
     stream << ")";
     return stream;
   }
 };
}

#endif // POSEIDON_SWEEPER_STATS_H