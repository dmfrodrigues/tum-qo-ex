#include "Config.hpp"
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "Register.hpp"
#include "operator/Operator.hpp"
#include "catch2/catch.hpp"
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace tinydb;

template <typename T>
vector<T> sorted(vector<T> vals) {
   sort(vals.begin(), vals.end());
   return vals;
}

template <typename T>
vector<T> sorted2(vector<T> vals) {
   for (auto& v : vals) {
      if (v.first > v.second) {
         swap(v.first, v.second);
      }
   }
   sort(vals.begin(), vals.end());
   return vals;
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

extern string sql01;
extern string sql02;
extern string sql03;
extern string sql04;
extern string sql04;
string sql04a = "select vorlnr, titel from vorlesungen v where v.titel = 'Logik'";
extern string sql05;
extern string sql06;
extern string sql07;
extern string sql09;
extern string sql10;
extern string sql11;
extern string sql12;
extern string sql13;
extern string sql14;
extern string sql15;
extern string sql16;
string sql17 = "select name, titel from vorlesungen v, studenten s, hoeren h where s.matrnr=h.matrnr and v.vorlnr = h.vorlnr";

Register make_reg(const string& v) {
   Register r;
   r.setString(v);
   return r;
}
Register make_reg(int v) {
   Register r;
   r.setInt(v);
   return r;
}
Register make_reg(double v) {
   Register r;
   r.setDouble(v);
   return r;
}
Register make_reg(bool v) {
   Register r;
   r.setBool(v);
   return r;
}

vector<vector<Register>> executeImpl(const std::unique_ptr<Operator>& root) {
   vector<vector<Register>> mv;
   root->open();
   while (root->next()) {
      vector<Register> tup;
      for (const Register* r : root->getOutput()) {
         tup.emplace_back(*r);
      }
      mv.emplace_back(move(tup));
   }
   root->close();
   sort(mv.begin(), mv.end());
   return mv;
}

TEST_CASE("task2") {
   SECTION("SQL Parse and Execute", "[sql][execution]") {
      auto sqlToPlan = [](Database& db, string_view sql) {
         auto jq = JoinQuery::parseAndAnalyseJoinQuery(db, sql);
         return jq.buildCanonicalTree(db);
      };

      Database db;
      db.createUniDb("tmp.db");
      auto plan = sqlToPlan(db, sql02);

      REQUIRE(plan.root);
      CHECK(executeImpl(plan.root) == vector<vector<Register>>{{make_reg("Ethik"s)}, {make_reg("Glaube und Wissen"s)}, {make_reg("Grundzüge"s)}, {make_reg("Grundzüge"s)}, {make_reg("Mäeutik"s)}});

      plan = sqlToPlan(db, sql04);
      REQUIRE(plan.root);
      CHECK(executeImpl(plan.root) == vector<vector<Register>>{{make_reg(5041), make_reg("Ethik"s)}});

      plan = sqlToPlan(db, sql04a);
      REQUIRE(plan.root);
      CHECK(executeImpl(plan.root) == vector<vector<Register>>{{make_reg(4052), make_reg("Logik"s)}});

      plan = sqlToPlan(db, sql05);
      REQUIRE(plan.root);
      CHECK(executeImpl(plan.root).empty());

      plan = sqlToPlan(db, sql17);
      REQUIRE(plan.root);
      CHECK(executeImpl(plan.root) == vector<vector<Register>>{{make_reg("Carnap"s), make_reg("Bioethik"s)}, {make_reg("Carnap"s), make_reg("Der Wiener Kreis"s)}, {make_reg("Carnap"s), make_reg("Ethik"s)}, {make_reg("Carnap"s), make_reg("Wissenschaftstheorie"s)}, {make_reg("Feuerbach"s), make_reg("Glaube und Wissen"s)}, {make_reg("Feuerbach"s), make_reg("Grundzüge"s)}, {make_reg("Fichte"s), make_reg("Grundzüge"s)}, {make_reg("Jonas"s), make_reg("Glaube und Wissen"s)}, {make_reg("Schopenhauer"s), make_reg("Grundzüge"s)}, {make_reg("Schopenhauer"s), make_reg("Logik"s)}, {make_reg("Theophrastos"s), make_reg("Ethik"s)}, {make_reg("Theophrastos"s), make_reg("Grundzüge"s)}, {make_reg("Theophrastos"s), make_reg("Mäeutik"s)}});

      {
         JoinQuery jq;
         CHECK(!jq.buildCanonicalTree(db).root);
      }

   }
}
