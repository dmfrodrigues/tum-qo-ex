#pragma once

namespace tinydb {
struct Plan {
   Plan* left = nullptr;
   Plan* right = nullptr;
   int id = -1;
   double cardinality = 0.0;
   double cost = 0.0;
   auto operator<(const Plan& p) const { return cost < p.cost; }
};
}
