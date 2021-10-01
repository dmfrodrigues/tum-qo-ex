#include "Config.hpp"
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "catch2/catch.hpp"
#include <set>
#include <string>
#include <vector>

using namespace std;
using namespace tinydb;

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
}

string sql01 = "select * from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s2.matrnr=s1.matrnr and s1.name = 'Schopenhauer'";
string sql02 = "select titel from vorlesungen v, studenten s, hoeren h where s.matrnr=h.matrnr and v.vorlnr = h.vorlnr and s.semester = 2";
string sql03 = "select * from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s1.name = 'Schopenhauer'";
string sql04 = "select vorlnr, titel from vorlesungen v where v.vorlnr = 5041";
string sql05 = "select vorlnr, titel from vorlesungen v where v.vorlnr = 5041 and v.vorlnr = 5031";
string sql06 = "select vorlnr, titel, persnr from vorlesungen v where v.vorlnr = 5041";
string sql07 = "select vorlnr, titel from vorlesungen v where v.vorlnr = 5041 and v.persnr = 5";
string sql09 = "select titel from vorlesungen titel where titel.titel = 'Ethik'";
string sql10 = "select matrnr from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s2.matrnr = s1.matrnr and s1.name = 'Schopenhauer'";
string sql11 = "select vorlnr titel from vorlesungen v where v.vorlnr = 5041";
string sql12 = "select, vorlnr from vorlesungen v where v.vorlnr = 5041";
string sql13 = "select vorlnr, titel from vorlesungen v where v.vorlnr 5041";
string sql14 = "select vorlnr,,,, titel from vorlesungen v where v.vorlnr 5041";
string sql15 = "select vorlnr, titel f vorlesungen v where v.vorlnr = 5041";
string sql16 = "select from vorlesungen v where v.vorlnr = 5041";

TEST_CASE("task1") {
   SECTION("Relations and bindings are correctly read", "[sql][relations]") {
      Database db;
      db.createUniDb("tmp.db");
      using RT = decltype(JoinQuery::relations);
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql01).relations == RT{{"studenten", "s1"}, {"studenten", "s2"}, {"hoeren", "h1"}, {"hoeren", "h2"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql02).relations == RT{{"vorlesungen", "v"}, {"studenten", "s"}, {"hoeren", "h"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql03).relations == RT{{"studenten", "s1"}, {"studenten", "s2"}, {"hoeren", "h1"}, {"hoeren", "h2"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql04).relations == RT{{"vorlesungen", "v"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql05).relations == RT{{"vorlesungen", "v"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql09).relations == RT{{"vorlesungen", "titel"}});
   }

   SECTION("Projection attributes are correctly read", "[sql][projections]") {
      Database db;
      db.createUniDb("tmp.db");
      auto& tbl = db.createTable("test_table");
      tbl.addAttribute("test_attribute", Attribute::Type::Int, false);

      using RT = decltype(JoinQuery::projection);
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql01).projection == RT{});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql02).projection == RT{{"v", "titel"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql03).projection == RT{});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql04).projection == RT{{"v", "vorlnr"}, {"v", "titel"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql05).projection == RT{{"v", "vorlnr"}, {"v", "titel"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql09).projection == RT{{"titel", "titel"}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, "select test_attribute from test_table a_b where a_b.test_attribute=7").projection == RT{{"a_b", "test_attribute"}});
   }

   SECTION("Selections are correctly read", "[sql][selections]") {
      Database db;
      db.createUniDb("tmp.db");
      using RT = decltype(JoinQuery::selections);
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql01).selections == RT{{{"s1", "name"}, Register::makeString("Schopenhauer")}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql02).selections == RT{{{"s", "semester"}, Register::makeInt(2)}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql03).selections == RT{{{"s1", "name"}, Register::makeString("Schopenhauer")}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql04).selections == RT{{{"v", "vorlnr"}, Register::makeInt(5041)}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql05).selections == RT{{{"v", "vorlnr"}, Register::makeInt(5041)}, {{"v", "vorlnr"}, Register::makeInt(5031)}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql09).selections == RT{{{"titel", "titel"}, Register::makeString("Ethik")}});
   }

   SECTION("Join conditions are correctly read", "[sql][joinConditions]") {
      Database db;
      db.createUniDb("tmp.db");
      using RT = decltype(JoinQuery::joinConditions);
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql01).joinConditions == RT{{{"s1", "matrnr"}, {"h1", "matrnr"}}, {{"s2", "matrnr"}, {"h2", "matrnr"}}, {{"h1", "vorlnr"}, {"h2", "vorlnr"}}, {{"s2", "matrnr"}, {"s1", "matrnr"}}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql02).joinConditions == RT{{{"s", "matrnr"}, {"h", "matrnr"}}, {{"v", "vorlnr"}, {"h", "vorlnr"}}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql03).joinConditions == RT{{{"s1", "matrnr"}, {"h1", "matrnr"}}, {{"s2", "matrnr"}, {"h2", "matrnr"}}, {{"h1", "vorlnr"}, {"h2", "vorlnr"}}});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql04).joinConditions == RT{});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql05).joinConditions == RT{});
      CHECK(JoinQuery::parseAndAnalyseJoinQuery(db, sql09).joinConditions == RT{});
   }

   SECTION("Semantic errors are checked for", "[sql][syntax]") {
      Database db;
      db.createUniDb("tmp.db");
      CHECK_THROWS(JoinQuery::parseAndAnalyseJoinQuery(db, sql06));
      CHECK_THROWS(JoinQuery::parseAndAnalyseJoinQuery(db, sql07));
      CHECK_THROWS(JoinQuery::parseAndAnalyseJoinQuery(db, sql10));
   }
}
