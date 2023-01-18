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

int main(int argc, char* argv[]){
    //string dbName = argv[1], query = argv[2];

    string query = "select * from hoeren h, vorlesungen v, studenten s, pruefen pr, professoren prof where h.vorlnr = v.vorlnr and h.matrnr = s.matrnr and pr.matrnr = s.matrnr and pr.vorlnr = v.vorlnr and prof.persnr = pr.persnr";
    tinydb::Database db;
    db.createUniDb("temp");

    auto jq = tinydb::JoinQuery::parseAndAnalyseJoinQuery(db, query);
    auto queryGraph = tinydb::QueryGraph::fromJoinQuery(db, jq);
    for(int i = 0; i < 1000; i++){
        auto tree = queryGraph.runQuickPick(db);
    }
}
//---------------------------------------------------------------------------
