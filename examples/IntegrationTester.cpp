#include "Attribute.hpp"
#include "Database.hpp"
#include "JoinQuery.hpp"
#include "Table.hpp"
#include "QueryGraph.hpp"
#include "operator/Printer.hpp"
#include "operator/Tablescan.hpp"
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
static void queryDb(Database& db, const string& query)
// Query the database
{
   JoinQuery jq;
   try {
      jq = JoinQuery::parseAndAnalyseJoinQuery(db, query);
      ofstream out("out.1.txt", ofstream::app | ofstream::binary);
      out << "ok" << endl;

      {
         auto vs = jq.relations;
         sort(vs.begin(), vs.end());
         out << vs.size() << " ";
         for (auto& r : vs)
            out << r.table << " " << r.binding << " ";
         out << endl;
      }
      {
         auto vs = jq.projection;
         sort(vs.begin(), vs.end());
         out << vs.size() << " ";
         for (auto& r : vs)
            out << r.binding << " " << r.attribute << " ";
         out << endl;
      }
      {
         auto vs = jq.joinConditions;
         for (auto& r : vs) {
            if (r.second < r.first)
               swap(r.first, r.second);
         }

         sort(vs.begin(), vs.end());
         out << vs.size() << " ";
         for (auto& r : vs) {
            out << r.first.binding << " " << r.first.attribute << " " << r.second.binding << " " << r.second.attribute << " ";
         }
         out << endl;
      }
      {
         auto vs = jq.selections;
         sort(vs.begin(), vs.end());
         out << vs.size() << " ";
         for (auto& r : vs)
            out << r.first.binding << " " << r.first.attribute << " " << r.second << " ";
         out << endl;
      }
   } catch (const std::exception& err) {
      ofstream out("out.1.txt", ofstream::app | ofstream::binary);
      out << "err: " << err.what() << endl;
   }

   auto executeTree = [&](OperatorTree op, ostream& out) {
      if (!op.root) {
         out << "EMPTY" << endl;
         return;
      }
      op.root->printTree(out);
      out << endl;
      out << "--result" << endl;
      auto output = op.root->getOutput();
      Printer p(move(op.root), move(output), out);
      p.open();
      while (p.next());
      p.close();
      out << "--end" << endl;
   };

   try {
      ofstream out("out.2.txt", ofstream::app | ofstream::binary);
      auto op = jq.buildCanonicalTree(db);
      out << "ok" << endl;
      executeTree(move(op), out);
   } catch (const std::exception& err) {
      ofstream out("out.2.txt", ofstream::app | ofstream::binary);
      out << "err: " << err.what() << endl;
   }

   QueryGraph qg;
   try {
      ofstream out("out.3.txt", ofstream::app | ofstream::binary);
      qg = QueryGraph::fromJoinQuery(db, jq);
      out << "ok" << endl;

      auto vs = jq.relations;
      sort(vs.begin(), vs.end());

      for (const auto& v1 : jq.relations)
         out << fixed << setprecision(2) << qg.getCardinality(v1.binding) << " ";
      out << endl;
      for (const auto& v1 : jq.relations) {
         for (const auto& v2 : jq.relations)
            out << fixed << setprecision(2) << qg.getSelectivity(v1.binding, v2.binding) << " ";
         out << endl;
      }
   } catch (const std::exception& err) {
      ofstream out("out.3.txt", ofstream::app | ofstream::binary);
      out << "err: " << err.what() << endl;
   }

   try {
      ofstream out("out.4.txt", ofstream::app | ofstream::binary);
      auto res = qg.runGOO(db);
      out << "ok" << endl;
      out << "cost: " << fixed << setprecision(2) << res.cost << endl;
      executeTree(move(res.tree), out);
   } catch (const std::exception& err) {
      ofstream out("out.4.txt", ofstream::app | ofstream::binary);
      out << "err: " << err.what() << endl;
   }

   try {
      ofstream out("out.5.txt", ofstream::app | ofstream::binary);
      auto res = qg.runDP(db);
      out << "ok" << endl;
      out << "cost: " << fixed << setprecision(2) << res.cost << endl;
      executeTree(move(res.tree), out);
   } catch (const std::exception& err) {
      ofstream out("out.5.txt", ofstream::app | ofstream::binary);
      out << "err: " << err.what() << endl;
   }

   db.close();
}
//---------------------------------------------------------------------------
static void runQuery(Database& db, istream& testFile)
// Run tests in file
{
   string query, myLine, otherLine;
   getline(testFile, query);

   queryDb(db, query);
}
//---------------------------------------------------------------------------
static void runTests(Database& db, istream& testFile)
// Run tests in file
{
   {
      ofstream o1("out.1.txt");
      ofstream o3("out.2.txt");
      ofstream o4("out.3.txt");
      ofstream o5("out.4.txt");
      ofstream o7("out.5.txt");
   }
   while (testFile)
      runQuery(db, testFile);
}
//---------------------------------------------------------------------------
static void showHelp(const char* argv0)
// Display a short help
{
   cout << "usage: " << argv0 << " [db] [test file]" << endl;
}
//---------------------------------------------------------------------------
static int integrationTesterMain(int argc, char* argv[])
// Entry point
{
   if (argc < 3) {
      showHelp(argv[0]);
      return 1;
   }

   cerr << "Hello world" << endl;

   /// TODO: Enter your matriculation numbers here!
   vector<string> matrNr{ "03741982", "03770446" };

   for (auto& m : matrNr)
      cout << m << " ";
   cout << endl;

   cerr << "Hi again world" << endl;

   string dbName = argv[1], testFile = argv[2];
   {
      Database db;
      db.open(dbName);
      ifstream tf(testFile);
      if (!tf.is_open()) {
         cerr << "Could not open test file" << endl;
         return 1;
      }
      runTests(db, tf);
   }

   cerr << "Goodbye world" << endl;
   return 0;
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
int main(int argc, char* argv[])
// Entry point
{
   return tinydb::integrationTesterMain(argc, argv);
}
//---------------------------------------------------------------------------
