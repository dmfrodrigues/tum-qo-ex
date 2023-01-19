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
    string dbPath = argv[1];
    string query = "select l.l_orderkey from lineitem l, orders o, customer c, nation n, supplier s, partsupp ps , part p, region r where l.l_orderkey = o. o_orderkey and o.o_custkey = c.c_custkey and c.c_nationkey = n. n_nationkey and l.l_suppkey = s.s_suppkey and s.s_nationkey = n.n_nationkey and n.n_regionkey = r.r_regionkey and l.l_partkey = ps.ps_partkey and l.l_suppkey = ps.ps_suppkey and l.l_partkey = p.p_partkey";
    
    tinydb::Database db;
    db.open(dbPath);

    auto jq = tinydb::JoinQuery::parseAndAnalyseJoinQuery(db, query);
    auto queryGraph = tinydb::QueryGraph::fromJoinQuery(db, jq);

    for(int i = 0; i < 1000; i++){
        auto tree = queryGraph.runQuickPick(db);
    }
}
//---------------------------------------------------------------------------
