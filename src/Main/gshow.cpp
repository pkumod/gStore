/*=============================================================================
# Filename: gshow.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2019-07-25 17:00
# Description: used to show all the databases that have already been built
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

struct DBInfo
{
public:
    string db_name;
    string creator;
    string built_time;
    DBInfo()
    {
    }
    DBInfo(string _db_name)
    {
        db_name = _db_name;
    }
    ~DBInfo()
    {
    }
};

int main(int argc, char *argv[])
{
    Util util;
    string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	string system_db_name = "system";
	string system_db_path = _db_home + "/" + system_db_name + _db_suffix;
    if (Util::dir_exist(system_db_path) == false)
    {
        cout << "The system database is not exist,please use bin/ginit to rebuild the system database at first!" << endl;
        return 0;
    }

    Database system_db(system_db_name);
    system_db.load();
    string sparql = "select ?s where{?s <database_status> \"already_built\".}";
    ResultSet _rs;
    FILE *ofp = nullptr;
    system_db.query(sparql, _rs, ofp);
    DBInfo *databases = new DBInfo[_rs.ansNum + 1];
    databases[0].db_name = system_db_name;
    for (unsigned i = 0; i < _rs.ansNum; i++)
    {
        databases[i + 1].db_name = Util::clear_angle_brackets(_rs.answer[i][0]);
        sparql = "select ?x ?y where{" + _rs.answer[i][0] + " <built_by> ?x. " + _rs.answer[i][0] + " <built_time> ?y.}";
        ResultSet _rs1;
        system_db.query(sparql, _rs1, ofp);
        if (_rs1.ansNum > 0)
        {
            databases[i + 1].creator = Util::clear_angle_brackets(_rs1.answer[0][0]);
            databases[i + 1].built_time = Util::replace_all(_rs1.answer[0][1], "\"", "");                
        }
    }
    sparql = "select ?x where{<system> <built_by> ?x.}";
    ResultSet _rs2;
    system_db.query(sparql, _rs2, ofp);
    if (_rs2.ansNum > 0)
    {
        databases[0].creator = Util::clear_angle_brackets(_rs2.answer[0][0]);
    }
    std::vector<std::string> headers = {"database", "creator", "built_time"};
    std::vector<std::vector<std::string>> rows;
    for (size_t i = 0; i < _rs.ansNum + 1; i++)
    {
        rows.push_back({databases[i].db_name, databases[i].creator, databases[i].built_time});
    }
    Util::printConsole(headers, rows);
    return 0;
}
