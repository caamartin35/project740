#ifndef __LIB_STATS_H__
#define __LIB_STATS_H__

class Stats {
 public:
  int requests;
  int hits;
  int misses;
  int evictions;
  int used_space;
};

#endif