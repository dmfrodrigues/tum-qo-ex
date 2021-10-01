#include "Config.hpp"
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "QueryGraph.hpp"
#include "Register.hpp"
#include "catch2/catch.hpp"
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace tinydb;

extern string sql02;
extern string sql04;
string sql04b = "select vorlnr, titel from vorlesungen v where v.sws = 4";
extern string sql05;
extern string sql17;
string sql18 = "select name, titel from vorlesungen v, professoren p where p.persnr=v.gelesenvon and p.rang='C4' and v.sws=3";
string sql19 = "select raum, note from pruefen p, assistenten a, professoren prof where a.boss=p.persnr and prof.persnr=a.boss and p.persnr=prof.persnr and note=2";

TEST_CASE("task3") {
   SECTION("SQL to graph", "[sql][querygraph]") {
      auto sqlToGraph = [](Database& db, string_view sql) {
         auto jq = JoinQuery::parseAndAnalyseJoinQuery(db, sql);
         return QueryGraph::fromJoinQuery(db, jq);
      };

      Database db;
      db.createUniDb("tmp.db");
      db.runStats();

      //select titel from vorlesungen v, studenten s, hoeren h where s.matrnr=h.matrnr and v.vorlnr = h.vorlnr and s.semester = 2
      auto graph = sqlToGraph(db, sql02);
      CHECK(graph.getCardinality("v") == Approx(10.0));
      CHECK(graph.getCardinality("s") == Approx(1.142857));
      CHECK(graph.getCardinality("h") == Approx(13.0));
      CHECK(graph.getCardinality("x") == Approx(-1.0));

      CHECK(graph.getSelectivity("v", "h") == Approx(0.1));
      CHECK(graph.getSelectivity("h", "v") == Approx(0.1));
      CHECK(graph.getSelectivity("v", "s") == Approx(1.0));
      CHECK(graph.getSelectivity("s", "v") == Approx(1.0));
      CHECK(graph.getSelectivity("s", "h") == Approx(0.125));
      CHECK(graph.getSelectivity("h", "s") == Approx(0.125));
      CHECK(graph.getSelectivity("x", "y") == Approx(1.0));

      //select vorlnr, titel from vorlesungen v where v.vorlnr = 5041
      graph = sqlToGraph(db, sql04);
      CHECK(graph.getCardinality("v") == Approx(1.0));

      //select vorlnr, titel from vorlesungen v where v.titel = 'Logik'
      graph = sqlToGraph(db, sql04b);
      CHECK(graph.getCardinality("v") == Approx(3.33333));

      //select vorlnr, titel from vorlesungen v where v.vorlnr = 5041 and v.vorlnr = 5031
      graph = sqlToGraph(db, sql05);
      CHECK(graph.getCardinality("v") == Approx(0.1));

      //select name, titel from vorlesungen v, studenten s, hoeren h where s.matrnr=h.matrnr and v.vorlnr = h.vorlnr
      graph = sqlToGraph(db, sql17);
      CHECK(graph.getCardinality("v") == Approx(10.0));
      CHECK(graph.getCardinality("s") == Approx(8.0));
      CHECK(graph.getCardinality("h") == Approx(13.0));

      CHECK(graph.getSelectivity("v", "h") == Approx(0.1));
      CHECK(graph.getSelectivity("h", "v") == Approx(0.1));
      CHECK(graph.getSelectivity("v", "s") == Approx(1.0));
      CHECK(graph.getSelectivity("s", "v") == Approx(1.0));
      CHECK(graph.getSelectivity("s", "h") == Approx(0.125));
      CHECK(graph.getSelectivity("h", "s") == Approx(0.125));

      //select name, titel from vorlesungen v, professoren p where p.persnr=v.gelesenvon and p.rang='C4' and v.sws=3
      graph = sqlToGraph(db, sql18);
      CHECK(graph.getCardinality("v") == Approx(3.33333));
      CHECK(graph.getCardinality("p") == Approx(3.5));

      CHECK(graph.getSelectivity("v", "p") == Approx(0.142857));
      CHECK(graph.getSelectivity("p", "v") == Approx(0.142857));

      //select raum, note from pruefen p, assistenten a, professoren prof where a.boss=p.persnr and prof.persnr=a.boss and p.persnr=prof.persnr and note=2
      graph = sqlToGraph(db, sql19);
      CHECK(graph.getCardinality("p") == Approx(1.5));
      CHECK(graph.getCardinality("a") == Approx(6.0));
      CHECK(graph.getCardinality("prof") == Approx(7.0));

      CHECK(graph.getSelectivity("p", "a") == Approx(0.25));
      CHECK(graph.getSelectivity("a", "p") == Approx(0.25));
      CHECK(graph.getSelectivity("p", "prof") == Approx(0.142857));
      CHECK(graph.getSelectivity("prof", "p") == Approx(0.142857));
      CHECK(graph.getSelectivity("a", "prof") == Approx(0.142857));
      CHECK(graph.getSelectivity("prof", "a") == Approx(0.142857));
   }
}
