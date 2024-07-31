/*
 * @Author: liwenjie
 * @Date: 2021-08-22 20:14:02
 * @LastEditTime: 2022-11-22 20:58:42
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: batch insert data tool
 * @FilePath: /gstore/Main/gadd.cpp
 */

#include "../Database/Database.h"
#include "../Util/Util.h"
#include "../Util/CompressFileUtil.h"
//#include "../Util/Slog.h"
#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
	// chdir(dirname(argv[0]));
	//#ifdef DEBUG
	//*! [important] you should init the Util class.
	Util util;
	//#endif
	// Log.init("slog.properties");

	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		// Log.Error("Invalid arguments! Input \"bin/gadd -h\" for help.");
		cout << "Invalid arguments! Input \"bin/gadd -h\" for help." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Batch Insert Data Tools(gadd)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gadd -db [dbname] -f [filename] " << endl;
			cout << "      \tbin/gadd -db [dbname] -dir [dirname] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-f,--file,\t\tthe file path for inserting data." << endl;
			cout << "\t-dir,--directory,\t\tthe directory path for inserting datas." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			cout << "Invalid arguments! Input \"bin/gadd -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		string _db_home = util.getConfigureValue("db_home");
		string _db_suffix = util.getConfigureValue("db_suffix");
		int _len_suffix = _db_suffix.length();
		string db_folder = Util::getArgValue(argc, argv, "db", "database");
		if (db_folder.empty())
		{
			cout << "the database name is empty! Input \"bin/gadd -h\" for help." << endl;
			return 0;
		}
		int len = db_folder.length();

		if (len > _len_suffix && db_folder.substr(len - _len_suffix, _len_suffix) == _db_suffix)
		{
			cout << "your database can not end with " + _db_suffix + "! Input \"bin/gadd -h\" for help." << endl;
			return 0;
		}
		//check the db_name is system
		if (db_folder == "system")
		{
			cout<<"The database name can not be system."<<endl;
			return 0;
		}
		string filename = Util::getArgValue(argc, argv, "f", "file");
		string dirname = Util::getArgValue(argc, argv, "dir", "directory");
		if (filename.empty() && dirname.empty())
		{
			cout << "the add data file is empty! Input \"bin/gadd -h\" for help." << endl;
			return 0;
		}
		bool is_zip = false;
		if (Util::fileSuffix(filename) == "zip")
			is_zip = true;
		std::string unz_dir_path;
		std::vector<std::string> zip_files;
		if (is_zip)
		{
			unz_dir_path = filename + "_" + Util::getTimeString2();
			CompressUtil::UnCompressZip unzip(filename, unz_dir_path);
			mkdir(unz_dir_path.c_str(), 0775);
			if (unzip.unCompress() != CompressUtil::UnZipOK)
			{
				Util::remove_path(unz_dir_path);
				cout<<"zip file uncompress faild "<<endl;
				return -1;
			}
			else
			{
				unzip.getFileList(zip_files, "");
			}
		}
		Database system_db("system");
		system_db.load();

		string sparql = "ASK WHERE{<" + db_folder + "> <database_status> \"already_built\".}";
		ResultSet ask_rs;
		FILE *ask_ofp = nullptr;
		system_db.query(sparql, ask_rs, ask_ofp);
		if (ask_rs.answer[0][0] == "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>")
		{
			cout << "The database does not exist." << endl;
			return 0;
		}
		Database _db(db_folder);
		_db.load();
		cout << "finish loading" << endl;

		// Log.Info("finish loading.");
		//_db.insert(argv[2]);
		//_db.remove(argv[2]);
		long tv_begin = Util::get_cur_time();
		unsigned success_num = 0;
		unsigned total_num = 0;
		unsigned parse_error_num = 0 ;
		string error_log = _db_home + "/" + db_folder + _db_suffix + "/parse_error.log";
		if (filename.empty() == false)
		{
			if (!is_zip)
			{
				total_num = Util::count_lines(error_log);
				success_num = _db.batch_insert(filename, false, nullptr);
				// exclude Info line
				parse_error_num = Util::count_lines(error_log) - total_num - 1;
			}
			else
			{
				total_num = Util::count_lines(error_log);
				for (string rdf_file : zip_files)
				{
					cout << "begin insert data from " << rdf_file << endl;
					success_num += _db.batch_insert(rdf_file, false, nullptr);
				}
				// exclude Info line
				parse_error_num = Util::count_lines(error_log) - total_num - zip_files.size();
				Util::remove_path(unz_dir_path);
			}
		} 
		else if (dirname.empty() == false)
		{
			vector<string> files;
			if (dirname[dirname.length() - 1] != '/')
			{
				dirname.push_back('/');
			}
			Util::dir_files(dirname, "", files);
			total_num = Util::count_lines(error_log);
			for (string rdf_file : files)
			{
				cout << "begin insert data from " << dirname << rdf_file << endl;
				success_num += _db.batch_insert(dirname + rdf_file, false, nullptr);
			}
			// exclude Info line
			parse_error_num = Util::count_lines(error_log) - total_num - files.size();
		}
		long tv_end = Util::get_cur_time();
		// cout << "finish insert data" << endl;
		cout << "after inserted triples num "<< success_num <<",failed num " << parse_error_num <<",used " << (tv_end - tv_begin) << " ms" << endl;
		if (parse_error_num > 0)
		{
			cout<< "See parse error log file for details " << error_log << endl;
		}
		_db.save();

		/*stringstream ss;
		ss << "after insert,used " << (tv_end - tv_begin) << " ms";
		Log.Info(ss.str().c_str());*/
		/*delete _db;
		long tv_end1 = Util::get_cur_time();
		ss.str("");
		ss << "persistence on disk" << (tv_end1 - tv_end) << " ms";
		Log.Info(ss.str());*/
	}

	return 0;
}
