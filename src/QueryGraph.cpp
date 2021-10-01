#include "Config.hpp"
#include "QueryGraph.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
double QueryGraph::getCardinality(const std::string& alias)
// Get the cardinality of a relation after selection pushdown according to the formulae we learned in the exercises
{
   throw NotImplementedError{"should be implemented for task 3"};
}
//---------------------------------------------------------------------------
double QueryGraph::getSelectivity(const std::string& alias1, const std::string& alias2)
// Get the selectivity of a join between two relations according to the formulae we learned in the exercises
{
   throw NotImplementedError{"should be implemented for task 3"};
}
//---------------------------------------------------------------------------
QueryGraph QueryGraph::fromJoinQuery(Database& db, const JoinQuery& jq)
// Build query graph from a parsed join query
{
   throw NotImplementedError{"should be implemented for task 3"};
}
//---------------------------------------------------------------------------
OptimizerResult QueryGraph::runGOO(Database& db)
// Run greedy operator ordering. Left side of the join should be the smaller side. You can assume number of relations < 64
{
   throw NotImplementedError{"should be implemented for task 4"};
}
//---------------------------------------------------------------------------
OptimizerResult QueryGraph::runDP(Database& db)
// Run dynamic programming. Left side of the join should be the smaller side. You can assume number of relations < 64
{
   throw NotImplementedError{"should be implemented for task 5"};
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
