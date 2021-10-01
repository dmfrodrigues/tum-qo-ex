#include "Config.hpp"
#include "Database.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/Printer.hpp"
#include "operator/Projection.hpp"
#include "operator/Selection.hpp"
#include "operator/Tablescan.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
namespace {
//---------------------------------------------------------------------------
struct OperationResult {
   unique_ptr<Operator> result;
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
static OperationResult getJoin(Database& db) {
   Table& professoren = db.getTable("professoren");
   Table& vorlesungen = db.getTable("vorlesungen");

   auto scanProfessoren = make_unique<Tablescan>(professoren);
   auto scanVorlesungen = make_unique<Tablescan>(vorlesungen);

   const Register* name = scanProfessoren->getOutput("name");
   const Register* persnr = scanProfessoren->getOutput("persnr");
   const Register* titel = scanVorlesungen->getOutput("titel");
   const Register* gelesenvon = scanVorlesungen->getOutput("gelesenvon");

   auto cp = make_unique<CrossProduct>(move(scanProfessoren), move(scanVorlesungen));
   auto select = make_unique<Selection>(move(cp), persnr, gelesenvon);
   auto project = make_unique<Projection>(move(select), vector<const Register*>{name, titel});

   return OperationResult{move(project)};
}
//---------------------------------------------------------------------------
static int joinMain() {
   //select p.name, v.titel from professoren p, vorlesungen v where p.persnr = v.gelesenvon
   Database db;
   db.createUniDb("tmp.db");

   auto result = getJoin(db);

   Printer out(move(result.result), cout);

   out.open();
   while (out.next())
      ;
   out.close();
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
int main()
// Entry point
{
   return tinydb::joinMain();
}
//---------------------------------------------------------------------------
