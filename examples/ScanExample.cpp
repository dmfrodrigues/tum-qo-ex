#include "Config.hpp"
#include "Database.hpp"
#include "operator/Tablescan.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
using namespace tinydb;
//---------------------------------------------------------------------------
int main() {
   Database db;
   db.createUniDb("tmp.db");
   Table& studenten = db.getTable("studenten");

   Tablescan scan(studenten);
   const Register* name = scan.getOutput("name");

   scan.open();
   while (scan.next())
      cout << name->getString() << endl;
   scan.close();
}
//---------------------------------------------------------------------------
