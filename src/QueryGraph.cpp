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
   if(!nodes.count(alias)){
      return -1;
   }
   return nodes.at(alias).getCardinality();
}
//---------------------------------------------------------------------------
double QueryGraph::getSelectivity(const std::string& alias1, const std::string& alias2)
// Get the selectivity of a join between two relations according to the formulae we learned in the exercises
{
   if(!adjacencyList.count(alias1) || !adjacencyList.count(alias2)) {
      return 1;
   }


   auto test = adjacencyList.at(alias1);
   for(auto i: test){
      if((i.one.first.name == alias1 && i.two.first.name == alias2) || (i.one.first.name == alias2 && i.two.first.name == alias1)) {
         return i.getSelectivity();
      }
   }
   //for(auto t: adjacencyList[alias1]){
   //
   //}

   return 1;
}
//---------------------------------------------------------------------------
QueryGraph QueryGraph::fromJoinQuery(Database& db, const JoinQuery& jq)
// Build query graph from a parsed join query
{
   QueryGraph graph;

   map<std::string, list<pair<JoinQuery::BindingAttribute, JoinQuery::Constant>>> selectionsMap;
   for(auto selection: jq.selections) {
      //cout << selection.first.binding << selection.first.attribute<< selection.second << endl;
      selectionsMap[selection.first.binding].push_back(selection);
   }

   for(auto relation: jq.relations){
      Table& table = db.getTable(relation.table);
      std::vector<std::pair<JoinQuery::BindingAttribute, JoinQuery::Constant>> predicates;
      for(auto selection: selectionsMap[relation.binding]){
         predicates.emplace_back(selection);
      }
      Node node(relation.binding, table, predicates);
      graph.nodes.emplace(relation.binding, move(node));
   }

   for(auto join: jq.joinConditions){
      std::vector<std::pair<JoinQuery::BindingAttribute, JoinQuery::Constant>> predicates;
      Edge edge(pair<Node&, AttributeName>(graph.nodes.at(join.first.binding), join.first.attribute), pair<Node&, AttributeName>(graph.nodes.at(join.second.binding), join.second.attribute));
      graph.edges.emplace_back(edge);

      graph.adjacencyList[join.first.binding].push_back(graph.edges.back());
      graph.adjacencyList[join.second.binding].push_back(graph.edges.back());
   }

   /*
   for(auto g: graph.adjacencyList){
      cout << g.first << endl;
      for(auto edge: g.second){
         cout << edge.one.first.name << " " << edge.one.second << endl;
         cout << edge.two.first.name << " " << edge.two.second << endl;
      }
   }
   */
   return graph;
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
QueryGraph::Node::Node(std::string name, Table& table, std::vector<std::pair<JoinQuery::BindingAttribute, JoinQuery::Constant>> predicates) noexcept
   : name(name), table(table), predicates(predicates){};
//---------------------------------------------------------------------------
double QueryGraph::Node::getCardinality(){
   double cardinality = table.getCardinality();

   for(auto predicate: predicates){
      /*
      switch(predicate.second.getState()) {
         case Register::State::Bool: cout << "  " << predicate.first.attribute << " = " << predicate.second.getBool() << endl; break;
         case Register::State::Int: cout << "  " << predicate.first.attribute << " = " << predicate.second.getInt() << endl; break;
         case Register::State::Double: cout << "  " << predicate.first.attribute << " = " << predicate.second.getDouble() << endl; break;
         case Register::State::String: cout << "  " << predicate.first.attribute << " = " << predicate.second.getString() << endl; break;
         default: throw logic_error("State not allowed");
      }
      */
      double selectivity = 1.0 / table.getAttribute(table.findAttribute(predicate.first.attribute)).getUniqueValues();
      cardinality *= selectivity;
   }

   //cout << name << ": " << cardinality << endl;
   return cardinality;
}
//---------------------------------------------------------------------------
QueryGraph::Edge::Edge(std::pair<Node&, AttributeName> first, std::pair<Node&, AttributeName> second) noexcept
   : one(first), two(second){};
//---------------------------------------------------------------------------
double QueryGraph::Edge::getSelectivity(){
   // One
   double uniqueOne = one.first.table.getAttribute(one.first.table.findAttribute(one.second)).getUniqueValues();
   double uniqueTwo = two.first.table.getAttribute(two.first.table.findAttribute(two.second)).getUniqueValues();

   return 1.0 / std::max(uniqueOne, uniqueTwo);

}
//---------------------------------------------------------------------------
}

