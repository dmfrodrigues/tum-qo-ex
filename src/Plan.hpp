#pragma once

namespace tinydb {
struct Plan {
   Plan* left = nullptr;
   Plan* right = nullptr;
   int id = -1;
   double cardinality = 0.0;
   double cost = 0.0;
   explicit Plan(int id, double cardinality): id(id), cardinality(cardinality){}
   explicit Plan(Plan *left, Plan *right, double selectivity):
      left(left),
      right(right),
      cardinality(left->cardinality * right->cardinality * selectivity)
      {}
   auto operator<(const Plan& p) const { return cost < p.cost; }
   ~Plan(){
      delete left;
      delete right;
   }

   uint64_t getBitset() const {
      if(left) return left->getBitset() | right->getBitset();
      else return (uint64_t(1) << id);
   }
};
}
