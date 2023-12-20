/*=============================================================================
# Filename: gpara.cpp
# Author: zhezhang
# Mail: zhezhang99@gmail.com
# Last Modified: 2019-08-06 10:17
# Description: written by zhezhang, modify parameters of every database.
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;
using namespace rapidjson;

int main(int argc, char * argv[]){
	Util util;

	if(argc < 4)  
	{
		cout << "the usage of gparameter: " << endl;
		cout << "bin/gparameter your_database_name -parameter value " << endl << endl << endl;
		cout << "parameter supported and value example: " << endl;
		cout << "is_backup: true/false" << endl;
		cout << "backup_interval: 6(hours)" << endl;
		cout << "backup_timer: 6(hours) PS: This parameter is only owned by system, so your_database_name must be system!" << endl;
		return 0;
	}

	cout << "gparameter..." << endl;
	{
		cout << "argc: " << argc << "\t";
		cout << "DB_name:" << argv[1] << "\t";
		cout << "parameter: " << argv[2] << "\t";
		cout << "value: " << argv[3] << "\t";
		cout << endl;
	}

	string db_name = argv[1];
	string parameter = argv[2];
	string value = argv[3];

	int isbuilt;
	if (Util::file_exist(db_name + ".db"))
		isbuilt = 1;
	else
		isbuilt = 0;

	if(!isbuilt){
		cout << "parameter modification failed" << endl;
		cout << "database " + db_name + "is not built yet! please check your database name" << endl;
		return 0;
	}
	if(parameter == "backup_timer" && db_name != "system"){
		cout << "parameter modification failed" << endl;
		cout << "backup_timer only belongs to system!" << endl;
		return 0;
	}
	else if(parameter == "is_backup" && (value != "true" && value != "false")){
		cout << "parameter modification failed" << endl;
		cout << "is_backup must be equal to true or flase!" << endl;
		return 0;
	}
	else if((parameter == "backup_timer" || parameter == "backup_interval") 
		&& !Util::is_number(value)){
		cout << "parameter modification failed" << endl;
		cout << "backup_timer or backup_interval must be assigned to a number" << endl;
		return 0;
	}
	int ret = Util::update_backuplog(db_name, parameter, value);
	if(ret == 0){
		cout << "parameter modification success" << endl;
		return 0;
	}
	else if(ret == 1){
		cout << "parameter modification failed" << endl;
		return 0;
	}
	return 0;
}