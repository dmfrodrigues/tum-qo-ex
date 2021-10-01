#include "Config.hpp"
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "QueryGraph.hpp"
#include "Register.hpp"
#include "catch2/catch.hpp"
#include "operator/Printer.hpp"
#include <set>
#include <sstream>
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

size_t print(unique_ptr<Operator>&& root) {
   size_t result = 0;
   auto printer = make_unique<Printer>(move(root), cout);
   printer->open();
   while (printer->next()) ++result;
   printer->close();
   return result;
}

namespace Catch {
template <typename F, typename S>
struct StringMaker<std::pair<F, S>> {
   static std::string convert(const std::pair<F, S>& value) {
      return "{"s +
         StringMaker<decltype(value.first)>().convert(value.first) +
         ", " +
         StringMaker<decltype(value.second)>().convert(value.second) +
         "}";
   }
};
template <>
struct StringMaker<Register> {
   static std::string convert(const Register& r) {
      stringstream result;
      result << "Register(";
      switch (r.getState()) {
         case Register::State::Bool:
            result << (r.getBool() ? "TRUE" : "FALSE");
            break;
         case Register::State::Int:
            result << r.getInt();
            break;
         case Register::State::Double:
            result << r.getDouble();
            break;
         case Register::State::String:
            result << "'" + r.getString() + "'";
            break;
         case Register::State::Unbound:
            result << "UNBOUND";
            break;
      }
      result << ")";
      return result.str();
   }
};
}


static auto sqlToGraph(Database& db, string_view sql) {
   auto jq = JoinQuery::parseAndAnalyseJoinQuery(db, sql);
   return QueryGraph::fromJoinQuery(db, jq);
};

TEST_CASE("task5") {
   SECTION("DP", "[sql][DP]") {
      Database db;
      db.createUniDb("tmp.db");
      db.runStats();

      {
         auto graph = sqlToGraph(db, sql02);
         auto optimized = graph.runDP(db);

         REQUIRE(optimized.tree.root);
         CHECK(optimized.cost == Approx(3.7142857));
         CHECK(executeImpl(optimized.tree.root) == vector<vector<Register>>{{make_reg("Ethik"s)}, {make_reg("Glaube und Wissen"s)}, {make_reg("Grundzüge"s)}, {make_reg("Grundzüge"s)}, {make_reg("Mäeutik"s)}});
      }

      {
         auto graph = sqlToGraph(db, sql04);
         auto optimized = graph.runDP(db);

         REQUIRE(optimized.tree.root);
         CHECK(optimized.cost == Approx(0.0));
         CHECK(executeImpl(optimized.tree.root) == vector<vector<Register>>{{make_reg(5041), make_reg("Ethik"s)}});
      }

      {
         auto graph = sqlToGraph(db, sql17);
         auto optimized = graph.runDP(db);

         REQUIRE(optimized.tree.root);
         CHECK(optimized.cost == Approx(26.0));
         CHECK(executeImpl(optimized.tree.root) == vector<vector<Register>>{{make_reg("Carnap"s), make_reg("Bioethik"s)}, {make_reg("Carnap"s), make_reg("Der Wiener Kreis"s)}, {make_reg("Carnap"s), make_reg("Ethik"s)}, {make_reg("Carnap"s), make_reg("Wissenschaftstheorie"s)}, {make_reg("Feuerbach"s), make_reg("Glaube und Wissen"s)}, {make_reg("Feuerbach"s), make_reg("Grundzüge"s)}, {make_reg("Fichte"s), make_reg("Grundzüge"s)}, {make_reg("Jonas"s), make_reg("Glaube und Wissen"s)}, {make_reg("Schopenhauer"s), make_reg("Grundzüge"s)}, {make_reg("Schopenhauer"s), make_reg("Logik"s)}, {make_reg("Theophrastos"s), make_reg("Ethik"s)}, {make_reg("Theophrastos"s), make_reg("Grundzüge"s)}, {make_reg("Theophrastos"s), make_reg("Mäeutik"s)}});
      }

      {
         auto graph = sqlToGraph(db, sql19);
         auto optimized = graph.runDP(db);

         REQUIRE(optimized.tree.root);
         CHECK(optimized.cost == Approx(1.821428));
         CHECK(executeImpl(optimized.tree.root) == vector<vector<Register>>{{make_reg(226), make_reg(2)}, {make_reg(226), make_reg(2)}});
      }

      {
         JoinQuery jq;
         auto qg = QueryGraph::fromJoinQuery(db, jq);
         auto res = qg.runDP(db);
         CHECK(!res.tree.root);
      }
   }
}
