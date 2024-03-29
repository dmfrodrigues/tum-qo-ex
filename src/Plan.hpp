#pragma once
#include <string>

namespace tinydb {
struct Plan {
   Plan* left = nullptr;
   Plan* right = nullptr;
   int id = -1;
   std::string name = "";
   double cardinality = 0.0;
   mutable double cost = -1.0;
   explicit Plan(int id, double cardinality): id(id), cardinality(cardinality){}
   explicit Plan(int id, double cardinality, std::string name): id(id), cardinality(cardinality), name(name){}
   explicit Plan(Plan *left, Plan *right, double selectivity):
      left(left),
      right(right),
      cardinality(left->cardinality * right->cardinality * selectivity)
      {}
   ~Plan(){
      delete left;
      delete right;
   }

   uint64_t getBitset() const {
      if(left) return left->getBitset() | right->getBitset();
      else return (uint64_t(1) << id);
   }

   double calculateCost() const {
      if(cost != -1.0) return cost;

      if(left){
         double c1 = left->calculateCost();
         double c2 = right->calculateCost();
         return cost = c1+c2+cardinality;
      } else {
         return cost = 0.0;
      }
   }

   std::string toString() const{
      if(left) return ("(" + left->toString() + " JOIN " + right->toString() + ")");
      return name != "" ? name : std::to_string(id);
   }
};
}
