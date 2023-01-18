#ifndef H_QueryGraph
#define H_QueryGraph
//---------------------------------------------------------------------------
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "operator/Operator.hpp"
#include "UnionFind.hpp"
#include <set>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// The operator tree resulting from optimizing a join query
struct OptimizerResult {
   /// The cost of the operator tree
   double cost = -1.0;
   /// An operator tree
   OperatorTree tree;
};
//---------------------------------------------------------------------------
/// A query graph representing joins as edges and relations as nodes
struct QueryGraph {
   /// TODO: Implement query graph
   JoinQuery joinQuery;

   using BindingName = std::string;
   using AttributeName = std::string;

   struct Node {
      BindingName name;
      Table& table;
      std::vector<std::pair<JoinQuery::BindingAttribute, JoinQuery::Constant>> predicates;

      int id = -1;

      double getCardinality() const;

      // Constructor
      explicit Node(std::string name, Table& table, std::vector<std::pair<JoinQuery::BindingAttribute, JoinQuery::Constant>> predicates, int id) noexcept;
   };

   struct Edge {
      std::pair<Node&, AttributeName> one;
      std::pair<Node&, AttributeName> two;

      double getSelectivity() const;

      // Constructor
      explicit Edge(std::pair<Node&, AttributeName> first, std::pair<Node&, AttributeName> second) noexcept;
   };


   public:

   std::map<std::string, Node> nodes;
   std::map<std::string, std::vector<Edge>> adjacencyList;
   std::vector<Edge> edges;

   QueryGraph(){}

   /// Get the cardinality of a relation after selection pushdown according to the formulae we learned in the exercises. Return -1 if binding does not exist.
   double getCardinality(const std::string& binding);
   /// Get the selectivity of a join between two relations according to the formulae we learned in the exercises
   double getSelectivity(const std::string& binding1, const std::string& binding2);

   /// Run greedy operator ordering. Left side of the join should be the smaller side. You can assume number of relations < 64
   OptimizerResult runGOO(Database& db);
   /// Run dynamic programming. Left side of the join should be the smaller side. You can assume number of relations < 64
   OptimizerResult runDP(Database& db);

   OptimizerResult runQuickPick(Database& db);

   /// Build query graph from a parsed join query
   static QueryGraph fromJoinQuery(Database& db, const JoinQuery& jq);

private:
   QueryGraph(const JoinQuery &joinQuery):joinQuery(joinQuery){}

   struct Pedge {
      int u, v;
      double selectivity, cardinality;
      bool operator==(const Pedge &e) const {
         return !(*this < e) && !(e < *this);
      }
      bool operator<(const Pedge &e) const {
         if(cardinality != e.cardinality) return cardinality < e.cardinality;
         else if(u != e.u) return u < e.u;
         else return v < e.v;
      }
   };

   void mergePlans(
      const Pedge &pedge,
      UnionFind &uf,
      std::vector<Plan*> &plan,
      std::map<int, std::map<int, Pedge>> &matr,
      std::set<Pedge> &q
   ) const;

};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
