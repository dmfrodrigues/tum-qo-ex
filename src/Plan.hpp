#pragma once

namespace tinydb {
struct Plan {
   Plan* left = nullptr;
   Plan* right = nullptr;
   int id = -1;
   double cardinality = 0.0;
   double cost = 0.0;
   Plan(int id_): id(id_){}
   Plan(Plan *left_, Plan *right_): left(left_), right(right_){}
   auto operator<(const Plan& p) const { return cost < p.cost; }
   ~Plan(){
      delete left;
      delete right;
   }
};
}
