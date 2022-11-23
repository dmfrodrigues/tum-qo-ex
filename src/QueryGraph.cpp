#include "Config.hpp"
#include "QueryGraph.hpp"
#include <cmath>
#include <set>
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
   QueryGraph graph(jq);
   // cerr << "graph.joinQuery, size=" << graph.joinQuery.selections.size() << endl;
   // for(const auto &s: graph.joinQuery.selections){
   //    cerr << s.first.binding << endl;
   // }

   map<std::string, list<pair<JoinQuery::BindingAttribute, JoinQuery::Constant>>> selectionsMap;
   for(auto selection: jq.selections) {
      //cout << selection.first.binding << selection.first.attribute<< selection.second << endl;
      selectionsMap[selection.first.binding].push_back(selection);
   }

   int counter = 0;
   for(auto relation: jq.relations){
      Table& table = db.getTable(relation.table);
      std::vector<std::pair<JoinQuery::BindingAttribute, JoinQuery::Constant>> predicates;
      for(auto selection: selectionsMap[relation.binding]){
         predicates.emplace_back(selection);
      }
      Node node(relation.binding, table, predicates, counter++);
      graph.nodes.emplace(relation.binding, move(node));
   }

   for(auto join: jq.joinConditions){
      std::vector<std::pair<JoinQuery::BindingAttribute, JoinQuery::Constant>> predicates;
      Edge edge(pair<Node&, AttributeName>(graph.nodes.at(join.first.binding), join.first.attribute), pair<Node&, AttributeName>(graph.nodes.at(join.second.binding), join.second.attribute));
      graph.edges.emplace_back(edge);

      graph.adjacencyList[join.first.binding].push_back(graph.edges.back());
      graph.adjacencyList[join.second.binding].push_back(graph.edges.back());
   }

   return graph;
}

const double epsilon = 1e-9;
bool equal(double a, double b){
   return fabs(a-b) < epsilon;
}

void QueryGraph::mergePlans(
   const Pedge &pedge,
   UnionFind &uf,
   vector<Plan*> &plans,
   map<int, map<int, Pedge>> &adj,
   set<Pedge> &q
) const{
   int u = pedge.u, v = pedge.v;
   Plan *uPlan  = plans.at(uf.findSet(u ));
   Plan *vPlan = plans.at(uf.findSet(v));
   if(uPlan->cardinality > vPlan->cardinality){
      swap(u, v);
      swap(uPlan, vPlan);
   }

   // Get selectivity for this join between left side and right side
   double selectivity = adj.at(u).at(v).selectivity;

   // Union-find
   uf.unionSet(u, v);
   const int root = uf.findSet(u);

   // Build new plan and store it
   plans[root] = new Plan(uPlan, vPlan, selectivity);

   // For each node n, remake edges that connected n to u and to v
   const auto uAdj = adj.at(u);
   const auto vAdj = adj.at(v);
   //// Delete edges from/to u and v
   for(const auto &p: uAdj){
      const int n = p.first;
      q.erase(adj[n][u]); q.erase(adj[u][n]);
      adj[n].erase(u);
   }
   for(const auto &p: vAdj){
      const int n = p.first;
      q.erase(adj[n][v]); q.erase(adj[v][n]);
      adj[n].erase(v);
   }
   adj.at(u).clear();
   adj.at(v).clear();
   //// Add new edges from/to root
   for(const auto &p: uAdj){
      const int n = p.first;
      if(n == u || n == v) continue;

      const Pedge pe = p.second;

      if(!adj[root].count(n)){
         adj[root][n] = adj[n][root] = Pedge {
            root, n, 1.0, plans[root]->cardinality*plans[n]->cardinality
         };
      }
      adj[root][n].selectivity *= pe.selectivity; adj[n][root].selectivity *= pe.selectivity; 
      adj[root][n].cardinality *= pe.selectivity; adj[n][root].cardinality *= pe.selectivity; 
   }
   for(const auto &p: vAdj){
      const int n = p.first;
      if(n == u || n == v) continue;

      const Pedge pe = p.second;

      if(!adj[root].count(n)){
         adj[root][n] = adj[n][root] = Pedge {
            root, n, 1.0, plans[root]->cardinality*plans[n]->cardinality
         };
      }
      adj[root][n].selectivity *= pe.selectivity; adj[n][root].selectivity *= pe.selectivity; 
      adj[root][n].cardinality *= pe.selectivity; adj[n][root].cardinality *= pe.selectivity; 
   }

   for(const auto &p: adj.at(root)){
      q.insert(p.second);
   }
}

//---------------------------------------------------------------------------
OptimizerResult QueryGraph::runGOO(Database& db)
// Run greedy operator ordering. Left side of the join should be the smaller side. You can assume number of relations < 64
{
   // cerr << __LINE__ << endl;
   // cerr << "joinQuery, size=" << joinQuery.selections.size() << endl;
   // for(const auto &s: joinQuery.selections){
   //    cerr << s.first.binding << endl;
   // }
   // cerr << __LINE__ << endl;

   map<int, string> id2binding;

   UnionFind uf(nodes.size());

   vector<Plan*> plans(nodes.size());
   for(const auto &p: nodes){
      const Node &node = p.second;
      plans[node.id] = new Plan(node.id, node.getCardinality());
      id2binding[node.id] = node.name;
   }

   map<int, map<int, Pedge>> adj;
   
   for(const Edge &e: edges){
      int u = e.one.first.id, v = e.two.first.id;
      if(!adj[u].count(v))
         adj[u][v] = adj[v][u] = Pedge{
            u, v, 1.0, plans[u]->cardinality * plans[v]->cardinality
         };
      adj[u][v].selectivity *= e.getSelectivity(); adj[v][u].selectivity *= e.getSelectivity();
      adj[u][v].cardinality *= e.getSelectivity(); adj[v][u].cardinality *= e.getSelectivity();
   }

   set<Pedge> q;
   for(const auto &p1: adj){
      const int &u = p1.first;
      for(const auto &p2: p1.second){
         const int &v = p2.first;

         if(u < v) q.insert(adj[u][v]);
      }
   }

   while(!q.empty()){
      const double cardinality = q.begin()->cardinality;
      vector<Pedge> candidates;
      while(!q.empty() && equal(cardinality, q.begin()->cardinality)){
         candidates.push_back(*q.begin());
         q.erase(q.begin());
      }

      int bestCandidate = -1;
      uint64_t bestBitset = ~uint64_t(0);
      for(int i = 0; i < candidates.size(); ++i){
         const Pedge &e = candidates[i];
         uint64_t uBitset = plans.at(uf.findSet(e.u))->getBitset();
         uint64_t vBitset = plans.at(uf.findSet(e.v))->getBitset();
         uint64_t bitset = uBitset | vBitset;
         if(bitset <= bestBitset){
            bestCandidate = i;
            bestBitset = bitset;
         }
      }

      Pedge pedge = candidates.at(bestCandidate);
      for(int i = 0; i < candidates.size(); ++i){
         if(i != bestCandidate)
            q.insert(candidates[i]);
      }

      mergePlans(pedge, uf, plans, adj, q);
   }

   Plan *plan = plans[uf.findSet(0)];

   

   return OptimizerResult{plan->calculateCost(), joinQuery.buildOperatorTree(db, plan)};
}
//---------------------------------------------------------------------------
OptimizerResult QueryGraph::runDP(Database& db)
// Run dynamic programming. Left side of the join should be the smaller side. You can assume number of relations < 64
{
   throw NotImplementedError{"should be implemented for task 5"};
}
//---------------------------------------------------------------------------
QueryGraph::Node::Node(std::string name, Table& table, std::vector<std::pair<JoinQuery::BindingAttribute, JoinQuery::Constant>> predicates, int id) noexcept
   : name(name), table(table), predicates(predicates), id(id){};
//---------------------------------------------------------------------------
double QueryGraph::Node::getCardinality() const{
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
double QueryGraph::Edge::getSelectivity() const {
   // One
   double uniqueOne = one.first.table.getAttribute(one.first.table.findAttribute(one.second)).getUniqueValues();
   double uniqueTwo = two.first.table.getAttribute(two.first.table.findAttribute(two.second)).getUniqueValues();

   return 1.0 / std::max(uniqueOne, uniqueTwo);

}
//---------------------------------------------------------------------------
}

