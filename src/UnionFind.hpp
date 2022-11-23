#pragma once

#include <vector>

class UnionFind {
private:
   std::vector<int> p, rank;

   // This function assumes x and y are roots
   void link(int x, int y) {
      if (x == y) return;
      if (rank[x] <= rank[y]) {
         p[x] = y;
         if (rank[x] == rank[y])
            rank[y]++;
      } else
         link(y, x);
   }

public:
   UnionFind(size_t s) : p(s), rank(s, 0) {
      for (int i = 0; i < s; ++i) {
         p[i] = i;
      }
   }
   int findSet(int x) {
      if (x != p[x])
         p[x] = findSet(p[x]);
      return p[x];
   }
   void unionSet(int x, int y) { link(findSet(x), findSet(y)); }
};
