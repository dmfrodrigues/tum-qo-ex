#include "Config.hpp"
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "QueryGraph.hpp"
#include "Register.hpp"
#include "catch2/catch.hpp"
#include <string>
#include <vector>

using namespace std;
using namespace tinydb;

extern string sql02;
extern string sql04;
extern string sql17;
extern string sql19;

Register make_reg(const string& v);
Register make_reg(int v);
Register make_reg(double v);
Register make_reg(bool v);

vector<vector<Register>> executeImpl(const std::unique_ptr<Operator>&);

TEST_CASE("task4") {
   SECTION("GOO", "[sql][GOO]") {
      auto sqlToGraph = [](Database& db, string_view sql) {
         auto jq = JoinQuery::parseAndAnalyseJoinQuery(db, sql);
         return QueryGraph::fromJoinQuery(db, jq);
      };

      Database db;
      db.createUniDb("tmp.db");
      db.runStats();

      auto graph = sqlToGraph(db, sql02);
      auto optimized = graph.runGOO(db);

      REQUIRE(optimized.tree.root);
      CHECK(optimized.cost == Approx(3.7142857));
      CHECK(executeImpl(optimized.tree.root) == vector<vector<Register>>{{make_reg("Ethik"s)}, {make_reg("Glaube und Wissen"s)}, {make_reg("Grundzüge"s)}, {make_reg("Grundzüge"s)}, {make_reg("Mäeutik"s)}});

      graph = sqlToGraph(db, sql04);
      optimized = graph.runGOO(db);

      REQUIRE(optimized.tree.root);
      CHECK(optimized.cost == Approx(0.0));
      CHECK(executeImpl(optimized.tree.root) == vector<vector<Register>>{{make_reg(5041), make_reg("Ethik"s)}});

      graph = sqlToGraph(db, sql17);
      optimized = graph.runGOO(db);

      REQUIRE(optimized.tree.root);
      CHECK(optimized.cost == Approx(26.0));
      CHECK(executeImpl(optimized.tree.root) == vector<vector<Register>>{{make_reg("Carnap"s), make_reg("Bioethik"s)}, {make_reg("Carnap"s), make_reg("Der Wiener Kreis"s)}, {make_reg("Carnap"s), make_reg("Ethik"s)}, {make_reg("Carnap"s), make_reg("Wissenschaftstheorie"s)}, {make_reg("Feuerbach"s), make_reg("Glaube und Wissen"s)}, {make_reg("Feuerbach"s), make_reg("Grundzüge"s)}, {make_reg("Fichte"s), make_reg("Grundzüge"s)}, {make_reg("Jonas"s), make_reg("Glaube und Wissen"s)}, {make_reg("Schopenhauer"s), make_reg("Grundzüge"s)}, {make_reg("Schopenhauer"s), make_reg("Logik"s)}, {make_reg("Theophrastos"s), make_reg("Ethik"s)}, {make_reg("Theophrastos"s), make_reg("Grundzüge"s)}, {make_reg("Theophrastos"s), make_reg("Mäeutik"s)}});

      graph = sqlToGraph(db, sql19);
      optimized = graph.runGOO(db);

      REQUIRE(optimized.tree.root);
      CHECK(optimized.cost == Approx(1.821428));
      CHECK(executeImpl(optimized.tree.root) == vector<vector<Register>>{{make_reg(226), make_reg(2)}, {make_reg(226), make_reg(2)}});

      {
         JoinQuery jq;
         auto qg = QueryGraph::fromJoinQuery(db, jq);
         auto res = qg.runGOO(db);
         CHECK(!res.tree.root);
      }
   }
}
