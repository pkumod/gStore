/*=============================================================================
# Filename: gbackup.cpp
# Author: zhe zhang
# Mail: zhezhang99@gmail.com
# Last Modified: 2019.7.22
# Description: backup a database by copy its source files to a backup location
=============================================================================*/

//backup
//TODO:database_path is designed to support the customized database path
//the defalut database_path is ./database_name.db

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;
#define SYSTEM_PATH "data/system/system.nt"
#define DEFALUT_BACKUP_PATH "backups"

int copy(string src_path, string dest_path)
{
  string sys_cmd;
  if (!boost::filesystem::exists(src_path)) {
    //check the source path
    cout << "Source Path Error, Please Check It Again!" << endl;
    return 1;
  }
  if (!boost::filesystem::exists(dest_path)) {
    //check the destnation path
    sys_cmd = "mkdir -p ./" + dest_path;
    system(sys_cmd.c_str());
  }

  sys_cmd = "cp -r " + src_path + " ./" + dest_path;
  system(sys_cmd.c_str());
  return 0; // success
}

int
main(int argc, char* argv[])
{
  Util util;
  string db_name, backup_path;
  if (argc < 2) {
    //output help info here
    cout << "the usage of gbackup: " << endl;
    cout << "./bin/gbackup your_database_name your_backup_path(optional) " << endl;
    cout << "defalut backup_path = ./backups" << endl;
    cout << "the path should not include your database's name!" << endl;
    return 0;
  }
  db_name = string(argv[1]);
  if (argc > 2)
    backup_path = string(argv[2]);

  cout << "gbackup..." << endl;
  {
    cout << "argc: " << argc << "\t";
    cout << "DB_name:" << db_name << "\t";
    cout << "Backup_path: " << backup_path << "\t";
    cout << endl;
  }

  int len = db_name.length();
  if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db") {
    cout << "your database name can not end with .db" << endl;
    return -1;
  }

  if (db_name == "system") {
    cout << "Your database's name can not be system." << endl;
    return -1;
  }

  //TODO: We need two column in system.db :
  //build_path and backup_path;
  //we can get the build_path as database_path should write the backup_path when we finish the backup
  //query database_name build_path
  //insert database_name backup_path

  if (backup_path == "")
    backup_path = DEFALUT_BACKUP_PATH;
  if (backup_path == "." || backup_path == "./") {
    cout << "Backup Path Can not be root, Backup Failed!" << endl;
    return 0;
  }
  if (backup_path[0] == '/')
    backup_path = '.' + backup_path;
  if (backup_path[backup_path.length() - 1] == '/')
    backup_path = backup_path.substr(0, backup_path.length() - 1);

  backup_path = "./" + backup_path;
  string db_path = db_name + ".db";

  //copy
  if (backup_path == "")
    backup_path = DEFALUT_BACKUP_PATH;
  int ret = copy(db_path, backup_path);
  if (ret == 1) {
    cout << "Database Name Error, Backup Failed!" << endl;
  } else {
    string time = Util::get_date_time();
    string timestamp = Util::get_timestamp();
    backup_path = backup_path + "/" + db_path;
    string _backup_path = backup_path + "_" + timestamp;
    string sys_cmd = "mv " + backup_path + " " + _backup_path;
    system(sys_cmd.c_str());
    cout << "Time:" + time << endl;
    cout << "DB:" + db_name + " Backup done!" << endl;
  }
  return 0;
}
