#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

int main(int argc, char * argv[])
{
	if(argc > 1)
	{
		if(boost::filesystem::exists("system.db"))
			return 0;
	}
	Util util;
	string _db_path = "system";
	string _rdf = "data/system/system.nt";
	Database _db(_db_path);
	bool flag = _db.build(_rdf);
	if (flag)
	{
		cout << "import RDF file to database done." << endl;
		ofstream f;
		f.open("./"+ _db_path +".db/success.txt");
		f.close();
	}
	else
	{
		cout << "import RDF file to database failed." << endl;
		string cmd = "rm -r " + _db_path + ".db";
		system(cmd.c_str());
	}
	return 0;
}
