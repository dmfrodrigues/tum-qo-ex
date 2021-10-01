#include "Config.hpp"
#include "Database.hpp"
#include "operator/Selection.hpp"
#include "operator/Tablescan.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
using namespace tinydb;
//---------------------------------------------------------------------------
namespace {
//---------------------------------------------------------------------------
struct OperationResult {
   vector<unique_ptr<Register>> registerContainer;
   unique_ptr<Operator> result;
   const Register* name;
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
static OperationResult getSelect(Database& db) {
   OperationResult retval;
   Table& studenten = db.getTable("studenten");

   auto scan = make_unique<Tablescan>(studenten);
   retval.name = scan->getOutput("name");
   const Register* semester = scan->getOutput("semester");
   //Register two;
   auto& two = *retval.registerContainer.emplace_back(make_unique<Register>());
   two.setInt(2);
   retval.result = make_unique<Selection>(move(scan), semester, &two);
   return retval;
}
//---------------------------------------------------------------------------
int main()
//select s.name from studenten s where s.semester = 2
{
   Database db;
   db.createUniDb("tmp.db");

   auto result = getSelect(db);
   auto& select = *result.result;

   select.open();
   while (select.next())
      cout << result.name->getString() << endl;
   select.close();
}
//---------------------------------------------------------------------------
