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
void executeTree(tinydb::OperatorTree op, ostream& out) {
    if (!op.root) {
        out << "EMPTY" << endl;
        return;
    }
    op.root->printTree(out);
    out << endl;
    out << "--result" << endl;
    auto output = op.root->getOutput();
    tinydb::Printer p(move(op.root), move(output), out);
    p.open();
    while (p.next());
    p.close();
    out << "--end" << endl;
};

int main(int argc, char* argv[])
// Entry point
{
    string dbName = argv[1], query = argv[2];
    {
        tinydb::Database db;
        db.open(dbName);

        auto jq = tinydb::JoinQuery::parseAndAnalyseJoinQuery(db, query);

        //auto op = jq.buildCanonicalTree(db);
        auto qg = tinydb::QueryGraph::fromJoinQuery(db, jq);
        auto resTree = qg.runGOO(db).tree;

        executeTree(move(resTree), std::cout);

    }
   //return tinydb::integrationTesterMain(argc, argv);
}
//---------------------------------------------------------------------------
