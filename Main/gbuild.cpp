/*
 * 
 * @Author: Bookug Lobert suxunbin liwenjie
 * @Date: 2021-08-20 10:29:41
 * @LastEditTime: 2023-02-14 14:10:28
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: The build database tool 
 * @FilePath: /gstore/Main/gbuild.cpp
 */


#include "../Database/Database.h"
#include "../Util/CompressFileUtil.h"
//#include "../Util/Slog.h"

using namespace std;

//[0]./gbuild [1]data_folder_path  [2]rdf_file_path
int
main(int argc, char * argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;
	string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	int _suffix_len = _db_suffix.length();
//#endif
	//Log.init("slog.properties");
	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		//Log.Error("Invalid arguments! Input \"bin/gbuild -h\" for help.");
		cout<<"Invalid arguments! Input \"bin/gbuild -h\" for help."<<endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Build Tools(gbuild)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gbuild -db [dbname] -f [filename] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-f,--file,\t\tthe file path for building." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			//cout << "the command is not complete." << endl;
			cout<<"Invalid arguments! Input \"bin/gbuild -h\" for help."<<endl;
			return 0;
		}
	}
	else
	{
		string db_name = Util::getArgValue(argc, argv, "db", "database");
		
		if (db_name.empty())
		{
			cout<<"You must input the database name for building database!"<<endl;
			return -1;
		}
		int len = db_name.length();
		if (len < _suffix_len || (len >= _suffix_len && db_name.substr(len - _suffix_len, _suffix_len) == _db_suffix))
		{
			cout << "your database can not end with " + _db_suffix + " or less than "<< _suffix_len <<" characters! Input \"bin/gbuild -h\" for help." << endl;
			return -1;
		}

		//check if the db_name is system
		if (db_name == "system")
		{
			cout<<"Your database's name can not be system."<<endl;
			return -1;
		}

		string _rdf = Util::getArgValue(argc, argv, "f", "file");
		bool is_zip = false;
		std::string unz_dir_path;
		std::vector<std::string> zip_files;
		//check if the db_path is the path of system.nt
		if (_rdf == Util::system_path)
		{
			cout<<"You have no rights to access system files"<<endl;
			return -1;
		}
		if (_rdf.empty())
		{
			std::cout<<"will build an empty database"<<std::endl;
			//cout<<"You must input the rdf file path for building database!"<<endl;
			//return -1;
		}

		string _db_path = _db_home + "/" + db_name + _db_suffix;
		cout << "database path: " << _db_path << endl;
		if(Util::file_exist(_db_path)) // if the database has been built
		{
			cout<<"the database " << db_name <<" has been built. please use bin/gdrop to remove it at first."<<endl;
			return -1;
		}
		else
		{
			std::string::size_type pos1 = _rdf.find_last_of("/");
			if (pos1 == std::string::npos)
				pos1 = 0;
			else
				pos1++;
			std::string zfile = _rdf.substr(pos1, -1);
			std::string::size_type pos2 = zfile.find_last_of(".");
			if (pos2 != std::string::npos && zfile.substr(pos2 + 1, -1)=="zip")
				is_zip = true;
			if (is_zip)
			{
				unz_dir_path = _rdf + "_" + Util::getTimeString2();
				std::cout<<"unz_dir_path:"<<unz_dir_path<<std::endl;
				mkdir(unz_dir_path.c_str(), 0775);
				CompressUtil::UnCompressZip unzip(_rdf, unz_dir_path);
				if (unzip.unCompress() != CompressUtil::UnZipOK)
				{
					Util::remove_path(unz_dir_path);
					cout<<"zip file uncompress faild "<<endl;
					return -1;
				}
				else
				{
					_rdf = unzip.getMaxFilePath();
					unzip.getFileList(zip_files, _rdf);
				}
			}
			long tv_begin = Util::get_cur_time();
			Database _db(db_name);
			bool flag = true;
			if (_rdf.empty()) 
				flag = _db.BuildEmptyDB();
			else
				flag = _db.build(_rdf);
			if (flag)
			{
				cout<<"Build Database Successfully!"<<endl;
				ofstream f;
				f.open(_db_path + "/success.txt");
				f.close();
			}
			else //if fails, drop database and return
			{
				cout<<"Build Database Failed!"<<endl;
				Util::remove_path(_db_path);
				return 0;
			}
			if (!Util::dir_exist(_db_home + "/system" + _db_suffix))
				return 0;
			//system("clock");
			cout<<"Save the database info to system database...."<<endl;
			Database system_db("system");
			system_db.load();
			// add database information to system.db
			string time = Util::get_date_time();
			string sparql = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." + "<" + db_name + "> <built_by> <root>."
				+ "<" + db_name + "> <built_time> \"" + time + "\".}";
			#ifdef DEBUG
			cout<<"insert sparql:"<<sparql<<endl;
			#endif
			ResultSet _rs;
			FILE* ofp = stdout;
			string msg;
			int ret = system_db.query(sparql, _rs, ofp);
			if (ret >= 0)
				msg = "update num " + Util::int2string(ret);
			else //update error
				msg = "update failed.";
			Util::add_backuplog(db_name);
			cout<<"Saving database info: " + msg<<endl;
			long tv_end = Util::get_cur_time();
			//stringstream ss;
			cout<< "Build RDF database "<<db_name<<" successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
			string error_log = _db_path + "/parse_error.log";
			// exclude Info line
			size_t parse_error_num = Util::count_lines(error_log);
			if (parse_error_num > 1)
			{
				cout<< "RDF parse error num "<< parse_error_num - 1 << endl;
				cout<< "See log file for details " << error_log << endl;
			}
			if (is_zip)
			{
				unsigned success_num = 0;
				unsigned total_num = 0;
				unsigned parse_error_num = 0 ;
				Database _db(db_name);
				_db.load();
				total_num = Util::count_lines(error_log);
				for (string rdf_file : zip_files)
				{
					cout << "begin insert data from " << rdf_file << endl;
					success_num += _db.batch_insert(rdf_file, false, nullptr);
				}
				// exclude Info line
				parse_error_num = Util::count_lines(error_log) - total_num - zip_files.size();
				tv_end = Util::get_cur_time();
				cout << "after inserted triples num "<< success_num <<",failed num " << parse_error_num <<",used " << (tv_end - tv_begin) << " ms" << endl;
				if (parse_error_num > 0)
				{
					cout<< "See parse error log file for details " << error_log << endl;
				}
				_db.save();
				Util::remove_path(unz_dir_path);
				cout<< "Build batch insert RDF database "<<db_name<<" successfully!"<<endl;
			}
			
			return 0;
		}
	}
}

