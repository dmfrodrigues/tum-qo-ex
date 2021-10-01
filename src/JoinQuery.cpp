#include "Config.hpp"
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "operator/Operator.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
OperatorTree::OperatorTree(unique_ptr<Operator> root, vector<unique_ptr<Register>> constants) noexcept
   : root(move(root)), constants(move(constants))
// Constructor
{
}
//---------------------------------------------------------------------------
/// Move constructor
OperatorTree::OperatorTree(OperatorTree&&) noexcept = default;
//---------------------------------------------------------------------------
/// Destructor
OperatorTree::~OperatorTree() noexcept = default;
//---------------------------------------------------------------------------
/// Move assignment
OperatorTree& OperatorTree::operator=(OperatorTree&&) noexcept = default;
//---------------------------------------------------------------------------
JoinQuery JoinQuery::parseAndAnalyseJoinQuery(Database& db, std::string_view sql)
// Parse and analyze a sql join query
{
   throw NotImplementedError{"should be implemented for task 1"};
}
//---------------------------------------------------------------------------
OperatorTree JoinQuery::buildCanonicalTree(Database& db) const
// Build a (right-deep) canonical operator tree from the join query with pushed down predicates
{
   throw NotImplementedError{"should be implemented for task 2"};
}
}
//---------------------------------------------------------------------------
