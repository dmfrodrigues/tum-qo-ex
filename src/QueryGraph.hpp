#ifndef H_QueryGraph
#define H_QueryGraph
//---------------------------------------------------------------------------
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "operator/Operator.hpp"
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

   public:
   /// Get the cardinality of a relation after selection pushdown according to the formulae we learned in the exercises. Return -1 if binding does not exist.
   double getCardinality(const std::string& binding);
   /// Get the selectivity of a join between two relations according to the formulae we learned in the exercises
   double getSelectivity(const std::string& binding1, const std::string& binding2);

   /// Run greedy operator ordering. Left side of the join should be the smaller side. You can assume number of relations < 64
   OptimizerResult runGOO(Database& db);
   /// Run dynamic programming. Left side of the join should be the smaller side. You can assume number of relations < 64
   OptimizerResult runDP(Database& db);

   /// Build query graph from a parsed join query
   static QueryGraph fromJoinQuery(Database& db, const JoinQuery& jq);
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
