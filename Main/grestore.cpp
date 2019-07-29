/*=============================================================================
# Filename: grestore.cpp
# Author: zhe zhang
# Mail: zhezhang99@gmail.com
# Last Modified: 2019.7.22
# Description: restore a database by copy its backup files to root location
=============================================================================*/




#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;
#define DEFALUT_BUILD_PATH "."
#define DEFALUT_BACKUP_PATH "./backups"

int copy(string src_path, string dest_path)
{
	if(!boost::filesystem::exists(src_path)){
		//check the source path
		cout << "Source Path Error, Please Check It Again!" << endl;
		return 1;
	}
	if(!boost::filesystem::exists(dest_path)){
		//check the destnation path 
		cout << "Destnation Path Error, Please Check It Again!" << endl;
		return -1;
	}
	string sys_cmd;
	sys_cmd = "cp -r " + src_path + ' ' +  dest_path ;
	system(sys_cmd.c_str());
    return 0;// success 
}

int
main(int argc, char * argv[])
{
	Util util;
	string db_name, backup_path, db_path;

	if(argc < 2)  
	{
		//output help info here
		cout << "the usage of grestore: " << endl;
		cout << "./bin/restore your_database_name your_backup_path(optional) your_database_path(optional)" << endl;
		cout << "defalut backup_path = ./backups" << endl;
		cout << "defalut database_path  = ./ " << endl;
		cout << "the path should not include your database's name!" << endl;
		return 0;
	}

	db_name = string(argv[1]);
	if(argc > 2)
		backup_path = string(argv[2]);
	if(argc > 3)
		db_path = string(argv[3]);

	cout << "gbackup..." << endl;
	{
		cout << "argc: " << argc << "\t";
		cout << "DB_name:" << db_name << "\t";
		cout << "Build_path: " << db_path << "\t";
		cout << "Backup_path: " << backup_path << "\t";
		cout << endl;
	}

	
	int len = db_name.length();
	if(db_name.length() > 3 && db_name.substr(len-3, 3) == ".db")
	{
		cout<<"your database name can not end with .db"<<endl;
		return -1;
	}

	if (db_name == "system")
	{
		cout<< "Your database's name can not be system."<<endl;
		return -1;
	}

//TODO1: We need a column in system.db : 
//last restore time;
//TODO2: 
//We can restore a deleted database
//if we insert the new database's info into the system.db 
//then we restore the files and we rebuild it 

	//copy

	if(backup_path == "")
		backup_path = DEFALUT_BACKUP_PATH;
	backup_path = backup_path + "/" + db_name + ".db";
	if(db_path == "") db_path = DEFALUT_BUILD_PATH;

	//delete
	string _db_path = db_path + "/" + db_name + ".db";
	string sys_cmd = "rm -rf " + _db_path;
	system(sys_cmd.c_str());

	int ret = copy(backup_path, db_path);
	if(ret == 1){
		cout << "Backup Path Error, Restore Failed!" << endl;
	}else if(ret == -1){
		cout << "Database Path Error, Restore Failed!" << endl;
	}else{
		//TODO update the in system.db
		string time = Util::get_date_time();
		cout << "Time:" + time << endl;
		cout << "DB:" + db_name + " Restore done!" << endl;
	}
	return 0;
}