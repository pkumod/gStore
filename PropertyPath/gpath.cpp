/*=============================================================================
# Filename:		gpath.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified:	2017-05-22 23:27
# Description: 
=============================================================================*/

//BETTER: run queries in batch, so as to reduce duplicate computing
//

#include "../Util/Util.h"
#include "../Database/Database.h"
#include "../KVstore/KVstore.h"

using namespace std;

//BETTER: remove all leading spaces, tabs or \r\n
string trim(const string& str, char c = ' ')
{
    string::size_type pos = str.find_first_not_of(c);
    if (pos == string::npos)
    {
        return str;
    }
    string::size_type pos2 = str.find_last_not_of(c);
    if (pos2 != string::npos)
    {
        return str.substr(pos, pos2 - pos + 1);
    }
    return str.substr(pos);
}

int split(const string& str, vector<string>& ret_, string sep = ",")
{
    if (str.empty())
    {
        return 0;
    }

    string tmp;
    string::size_type pos_begin = str.find_first_not_of(sep);
    string::size_type comma_pos = 0;

    while (pos_begin != string::npos)
    {
        comma_pos = str.find(sep, pos_begin);
        if (comma_pos != string::npos)
        {
            tmp = str.substr(pos_begin, comma_pos - pos_begin);
            pos_begin = comma_pos + sep.length();
        }
        else
        {
            tmp = str.substr(pos_begin);
            pos_begin = comma_pos;
        }

        if (!tmp.empty())
        {
            ret_.push_back(tmp);
            tmp.clear();
        }
    }

    return 0;
}

string replace(const string& str, const string& src, const string& dest)
{
    string ret;

    string::size_type pos_begin = 0;
    string::size_type pos       = str.find(src);
    while (pos != string::npos)
    {
        cout <<"replacexxx:" << pos_begin <<" " << pos <<"\n";
        ret.append(str.data() + pos_begin, pos - pos_begin);
        ret += dest;
        pos_begin = pos + 1;
        pos       = str.find(src, pos_begin);
    }
    if (pos_begin < str.length())
    {
        ret.append(str.begin() + pos_begin, str.end());
    }
    return ret;
}

//NOTICE: we don't consider recombination path* here, like (P1|P2)* or (P1*/P2)*
//^path, path(n) and path(n,m) are currently not considered
//WARN: the direction is from left to right, no edge with different directions!(so all are entity except that the last one cna be literal)
//(P1/P2)* and P1*/P2*are considered here
//NOTICE: we assume that ';' can be used to divide src and dest and pres
typedef struct PreUnitType
{
	//not dividable
	vector<string> pset;
	// |, *, ? (^path, path(n), path(n,m))
	char op;  
}PreUnit;
typedef list< vector<unsigned> > ResultType;
typedef ResultType::iterator ResultIterator;

#define ONE_EDGE INVALID
#define NO_EDGE INVALID-1
#define DEPTH 20
#define LIMIT_DEPTH 1

KVstore* kvstore;

//check if pres are all right
void show(vector<PreUnit>& _pres)
{
}

void convert(string _query, unsigned& src, unsigned& dest, vector<PreUnit>& _pres)
{
	string s = trim(_query, '\n');
	s = trim(s, '\r');
	s = trim(s, ' ');
	//NOTICE: we assume that no space/tab at the begining or end
	vector<string> divid;
	split(s, divid, ";");
	int size = divid.size();
	string str1 = divid[0];
	src = kvstore->getIDByEntity(str1);
	//get the dest, which may be a literal
	string str2 = divid[size-1];
	//cout<<"debug: "<<str2.length()<<endl;
	//cout<<(int)(str2[str2.length()-1])<<endl;
	if(Util::isEntity(str2))
	{
		//cout<<"is entity!"<<endl;
		dest = kvstore->getIDByEntity(str2);
		//cout<<kvstore->getIDByEntity("<http://dbpedia.org/resource/Piano>")<<endl;
	}
	else
	{
		dest = kvstore->getIDByLiteral(str2);
	}
	//cout<<"debug: "<<src<<" "<<dest<<endl;

	//divide all pres
	for(int i = 1; i < size-1; ++i)
	{
		string tmp = divid[i];
		int tnum = tmp.size();
		PreUnit pu;
		//Case: P+ or (path)+
		//BETTER: P+=PP*=P*P (in which order?)
		if(tmp[tnum-1] == '+')
		{
			string unit = tmp.substr(0, tnum-1);
			pu.pset.push_back(unit);
			//BETTER: this may be divided more precisily
			pu.op = ' ';   //means empty
			_pres.push_back(pu);
			PreUnit pu2;
			pu2.op = '*';
			pu2.pset.push_back(unit);
			_pres.push_back(pu2);
		}
		//Case: P* or (path)*
		else if(tmp[tnum-1] == '*')
		{
			string unit = tmp.substr(0, tnum-1);
			pu.pset.push_back(unit);
			//BETTER: this may be divided more precisily
			pu.op = '*';   //means empty
			_pres.push_back(pu);
		}
		//Case: P? or (path)?
		//NOTICE: in fact, P?=(P|null)
		else if(tmp[tnum-1] == '?')
		{
			string unit = tmp.substr(0, tnum-1);
			pu.pset.push_back(unit);
			//BETTER: this may be divided more precisily
			pu.op = '?';   //means empty
			_pres.push_back(pu);
		}
		//Case: (P1|P2|P3)
		else if(tmp[0] == '(' && tmp[tnum-1] == ')')
		{
			string unit = tmp.substr(1, tnum-2);
			split(unit, pu.pset, "|");
			pu.op = '|';
			_pres.push_back(pu);
		}
		//Case: constant Pre
		else
		{
			pu.pset.push_back(tmp);
			pu.op = ' ';
			_pres.push_back(pu);
		}
	}
}

void project(ResultType& _rs, bool _idx, vector<unsigned>& _cand)
{
	//project a column and remove duplicates
	if(_idx)
	{
		//to project the first column
		for(ResultIterator it = _rs.begin(); it != _rs.end(); ++it)
		{
			_cand.push_back((*it)[0]);
		}
	}
	else
	{
		//to project the last column
		for(ResultIterator it = _rs.begin(); it != _rs.end(); ++it)
		{
			int cnt = (*it).size() - 1;
			_cand.push_back((*it)[cnt]);
		}
	}
	//sort and remove duplicates
	sort(_cand.begin(), _cand.end());
	Util::removeDuplicate(_cand);
}

//NOTICE: if src or dest is variable, use INVALID instead 
//We do not store predicate in final paths, this is ok!
bool query(Database& _db, ResultType& _rs, unsigned _src, unsigned _dest, vector<PreUnit>& _pres)
{
	int size = _pres.size();
	if(size == 0)
	{
		bool flag = (_src == _dest);
		if(flag)
		{
			_rs.push_back(vector<unsigned>(1, NO_EDGE));
		}
		return flag;
	}
	//Case: one triple and no variables
	if(size == 1 && _pres[0].op == ' ' && _src != INVALID && _dest != INVALID)
	{
		TYPE_PREDICATE_ID preid = kvstore->getIDByPredicate(_pres[0].pset[0]);
		unsigned* list = NULL;
		unsigned list_len = INVALID;
		kvstore->getobjIDlistBysubIDpreID(_src, preid, list, list_len, true);
		unsigned ret = Util::bsearch_int_uporder(_dest, list, list_len);
		//_pres.clear();
		if(ret == INVALID)
		{
			//cout<<"unreachable: "<<_src<<" "<<preid<<" "<<_dest<<endl;
			return  false;
		}
		else
		{
			_rs.push_back(vector<unsigned>(1, ONE_EDGE));
			//cout<<"reachable: "<<_src<<" "<<preid<<" "<<_dest<<endl;
			return true;
		}
	}

	//use gstore to answer queries, set NULL, convert to unsigned
	string str = "select ";
	ResultSet ts;
	string str1, str2;

	if(_src == INVALID)
	{
		str1 = "?s";
		str = str + str1 + " ";
	}
	else
	{
		str1 = kvstore->getEntityByID(_src);
	}

	for(int i = 1; i < size; ++i)
	{
		str = str + "?x" + Util::int2string(i) + " ";
	}
	if(_dest == INVALID)
	{
		str2 = "?t";
		str = str + str2+ " ";
	}
	else
	{
		if(Util::is_entity_ele(_dest))
		{
			str2 = kvstore->getEntityByID(_dest);
		}
		else
		{
			str2 = kvstore->getLiteralByID(_dest);
		}
	}

	str += "where { ";
	int num = 1;
	string prev, next = str2;
	if(size > 1)
	{
		next = "?x" + Util::int2string(num);
	}
	string triple = str1 + " " + _pres[0].pset[0] + " " + next + " . ";
	str += triple;

	for(int i = 1; i < size-1; ++i)
	{
		prev = next;
		num++;
		next = "?x" + Util::int2string(num);
		triple = prev + " " + _pres[i].pset[0] + " " + next + " . ";
		str += triple;
	}

	if(size > 1)
	{
		triple = next + " " + _pres[size-1].pset[0] + " " + str2 + " . ";
		str += triple;
	}
	str += "} ";
	cout<<"query string: "<<str<<endl;
	
	_db.query(str, ts, NULL);
	//transform all strings to IDs
	int rowNum = ts.ansNum;
	cout<<"query(): "<<rowNum<<endl;
	if(rowNum == 0)
	{
		return false;
	}
	int colNum = ts.select_var_num;
	string** ansp = ts.answer;
	for(int i = 0 ; i < rowNum; ++i)
	{
		vector<unsigned> vt;
		for(int j = 0; j < colNum; ++j)
		{
			string ss = ansp[i][j];
			TYPE_ENTITY_LITERAL_ID id;
			if(Util::isEntity(ss))
			{
				id = kvstore->getIDByEntity(ss);
			}
			else
			{
				id = kvstore->getIDByLiteral(ss);
			}
			vt.push_back(id);
		}
		_rs.push_back(vt);
	}

	//_pres.clear();
	return true;
}

void find(vector<PreUnit>& _pres, vector<int>& _begin_vec, vector<int>& _end_vec)
{
	int maxl = -1;
	//find the longest sequence(which only contains constants)
	int i = 0, j;
	int size = _pres.size();
	while(i < size)
	{
		while(i < size && _pres[i].op != ' ')
		{
			i++;
		}
		j = i;
		while(j < size && _pres[j].op == ' ')
		{
			j++;
		}
		int len = j - i;
		//BETTER: choose according to pre2num?
		if(len > 0)
		{
			_begin_vec.push_back(i);
			_end_vec.push_back(j-1);
		}
		//if(len > maxl)
		//{
			//maxl = len;
			//_begin = i;
			//_end = j - 1;
		//}
		i = j;
	}
}

void merge(ResultType& _rs, ResultType* _curRS, int _num)
{
	for(int i = 0; i < _num; ++i)
	{
		ResultType& tmpRS = _curRS[i];
		for(ResultIterator it = tmpRS.begin(); it != tmpRS.end(); ++it)
		{
			_rs.push_back(*it);
		}
		tmpRS.clear();
	}
}

bool compute(Database& _db, ResultType& _rs, unsigned _src, unsigned _dest, vector<PreUnit>& _pres, int _depth=DEPTH)
{
#ifdef LIMIT_DEPTH
	if(_depth <= 0)
	{
		return false;
	}
#endif
	//cout<<"compute: "<<_src<<" "<<_dest<<endl;
	int size = _pres.size();
	if(size == 0)
	{
		bool flag = (_src == _dest);
		if(flag)
		{
			_rs.push_back(vector<unsigned>(1, NO_EDGE));
			//cout<<"matched: "<<_src<<" "<<_dest<<endl;
		}
		else
		{
			//cout<<"not matched: "<<_src<<" "<<_dest<<endl;
		}
		return flag;
	}
	//Case: all constants, one query
	vector<int> begin_vec, end_vec;
	find(_pres, begin_vec, end_vec);
	int num = begin_vec.size();
	int len = -1, maxi = -1, begin = 0, end = -1;
	for(int i = 0; i < num; ++i)
	{
		int t = end_vec[i] - begin_vec[i] + 1;
		if(t > len)
		{
			len = t;
			maxi = i;
		}
	}
	//begin = begin_vec[maxi];  end = end_vec[maxi];
	//cout<<"check: "<<begin<<" "<<end<<" "<<len<<" "<<_pres.size()<<endl;
	//cout<<"check kvstore: "<<kvstore->getIDByPredicate("<Friend>")<<endl;
	//cout<<"check kvstore: "<<kvstore->getIDByEntity("<Bob>")<<endl;
	if (len == size)
	{
		bool flag = query(_db, _rs, _src, _dest, _pres);
		//_pres.clear();
		return flag;
	}

	//Case: exist a sequence of constant predicates
	if(len > 0)
	{
		//compute all intervals first, then join them
		ResultType* curRS = new ResultType[num];
		vector<PreUnit>* constant_pres = new vector<PreUnit>[num];
		for(int i = 0; i < num; ++i)
		{
			begin = begin_vec[i]; end = end_vec[i];
			for(int j = begin; j <= end; ++j)
			{
				constant_pres[i].push_back(_pres[j]);
			}
		}

		unsigned constant_src = INVALID;
		unsigned constant_dest = INVALID;
		int other_num = num - 1;
		if(begin_vec[0] > 0)
		{
			other_num++;
		}
		else
		{
			constant_src = _src;
		}
		if(end_vec[num-1] < size-1)
		{
			other_num++;
		}
		else
		{
			constant_dest = _dest;
		}
		vector<PreUnit>* other_pres = new vector<PreUnit>[other_num];
		int cnt = 0;
		if(constant_src == INVALID)
		{
			for(int i = 0; i < begin_vec[0]; ++i)
			{
				other_pres[cnt].push_back(_pres[i]);
			}
			cnt++;
		}
		for(int i = 0; i < num-1; ++i)
		{
			begin = end_vec[i]+1; end = begin_vec[i+1]-1;
			for(int j = begin; j <= end; ++j)
			{
				other_pres[cnt].push_back(_pres[j]);
			}
			cnt++;
		}
		if(constant_dest == INVALID)
		{
			for(int i = end_vec[num-1]+1; i < size; ++i)
			{
				other_pres[cnt].push_back(_pres[i]);
			}
		}

		//DEBUG!
		//_pres.clear();
		bool flag;
		for(int i = 0; i < num; ++i)
		{
			unsigned t1 = (i==0)?constant_src:INVALID;
			unsigned t2 = (i==num-1)?constant_dest:INVALID;
			bool ret = query(_db, curRS[i], t1, t2, constant_pres[i]);
			//cout<<"check: finish the "<<i<<"th query"<<endl;
			if(i == 0)
			{
				flag = ret;
			}
		}

		//Case: one sequence, return result directly
		if(constant_src != INVALID && constant_dest != INVALID && num == 1)
		{
			_rs = curRS[0];
			delete[] constant_pres;
			delete[] other_pres;
			delete[] curRS;
			return flag;
		}

		//BETTER: the join order is important
		//divide and conquer, recursively
		//BETTER?: use other pres to filter the candidates first?

		//BETTER: project, hash and link
		cnt = 0;
		//deal with left
		if(constant_src == INVALID)
		{
			//project(curRS[0], true, xxx);
			for(ResultIterator it = curRS[0].begin(); it != curRS[0].end(); ++it)
			{
				unsigned id = (*it)[0];
				ResultType tmprs;
				compute(_db, tmprs, _src, id, other_pres[cnt], _depth-1);
				for(ResultIterator it2 = tmprs.begin(); it2 != tmprs.end(); ++it2)
				{
					unsigned t2 = (*it2)[0];
					//cout<<"check: "<<t2<<endl;
					//if(t2 == ONE_EDGE)
					//{
						//cout<<"check: one edge exists"<<endl;
					//}
					//else if(t2 == NO_EDGE)
					//{
						//cout<<"check: no edge exists"<<endl;
					//}
					_rs.push_back(*(it2));
					//if(t2 != ONE_EDGE && t2 != NO_EDGE)
					//{
						//_rs.push_back(*(it2));
					//}
					//else
					//{
						//_rs.push_back(vector<unsigned>());
					//}
					vector<unsigned>& last = _rs.back();
					int tsize = (*it).size();
					int ss = 0;
					//int ss = (t2 == NO_EDGE)?1:0;
					//cout<<"check: "<<ss<<" "<<tsize<<endl;
					for(int i = ss; i < tsize; ++i)
					{
						//cout<<"ele: "<<(*it)[i]<<endl;
						last.push_back((*it)[i]);
					}
				}
				tmprs.clear();
			}
			cnt++;
		}
		else
		{
			_rs = curRS[0];
		}
		//cout<<"check: left delaed"<<endl;

		//deal with medium
		for(int i = 1; i < num; ++i)
		{
			ResultType swapRS;
			//enum each pair of _rs and curRS[i]
			for(ResultIterator it = _rs.begin(); it != _rs.end(); ++it)
			{
				unsigned id1 = (*it).back();
				for(ResultIterator it2 = curRS[i].begin(); it2 != curRS[i].end(); ++it2)
				{
					unsigned id2 = (*it2).front();
					ResultType tmprs;
					compute(_db, tmprs, id1, id2, other_pres[cnt], _depth-1);
					for(ResultIterator it3 = tmprs.begin(); it3 != tmprs.end(); ++it3)
					{
						unsigned t2 = (*it3)[0];
						swapRS.push_back(*(it));
						vector<unsigned>& last = swapRS.back();
						int tsize = (*it3).size();
						//if(t2 != NO_EDGE && t2 != ONE_EDGE)
						//{
							for(int j = 0; j < tsize; ++j)
							{
								last.push_back((*it3)[j]);
							}
						//}
						tsize = (*it2).size();
						int ss = 0;
						//int ss = (t2 == NO_EDGE)?1:0;
						for(int j = ss; j < tsize; ++j)
						{
							last.push_back((*it2)[j]);
						}
					}
					tmprs.clear();
				}
			}
			_rs = swapRS;
			//if(_rs.empty())
			//{
				//break;
			//}
			cnt++;
		}
		//cout<<"check: medium delaed"<<endl;
		//if(_rs.empty())
		//{
			//cout<<"check: the result is aleardy empty!"<<endl;
		//}

		//deal with right
		if(constant_dest == INVALID)
		{
			ResultType swapRS;
			//project(curRS[num-1], false, xxx);
			for(ResultIterator it = _rs.begin(); it != _rs.end(); ++it)
			{
				unsigned id = (*it).back();
				ResultType tmprs;
				compute(_db, tmprs, id, _dest, other_pres[cnt], _depth-1);
				for(ResultIterator it2 = tmprs.begin(); it2 != tmprs.end(); ++it2)
				{
					unsigned t2 = (*it2)[0];
					swapRS.push_back(*(it));
					vector<unsigned>& last = swapRS.back();
					//if(t2 == NO_EDGE)
					//{
						//last.erase(--last.end());
					//}
					int tsize = (*it2).size();
					//if(t2 != NO_EDGE && t2 != ONE_EDGE)
					//{
						for(int i = 0; i < tsize; ++i)
						{
							last.push_back((*it2)[i]);
						}
					//}
				}
				tmprs.clear();
			}
			_rs = swapRS;
		}
		//cout<<"check: right delaed"<<endl;

		delete[] constant_pres;
		//cout<<"check: constant pres"<<endl;
		delete[] other_pres;
		//cout<<"check: other pres"<<endl;
		delete[] curRS;
		//cout<<"check: current result"<<endl;
		//DEBUG: this may be error
		return true;
	}

	//Case: no constants!
	//BETTER: we can select between head and tail, or just deal with P? or (P1|P2) before P*
	int i;
	for(i = 0; i < size; ++i)
	{
		if(_pres[i].op != '*')
		{
			break;
		}
	}
	if(i != size)
	{
		ResultType* curRS = NULL;
		int pnum = -1;
		vector<PreUnit> tpu = _pres;
		tpu[i].op = ' ';
		if(_pres[i].op == '?')
		{
			pnum = 2;
			curRS = new ResultType[pnum];
			compute(_db, curRS[0], _src, _dest, tpu, _depth-1);
			//cout<<"to erase: "<<i<<" "<<tpu[i].pset[0]<<endl;
			tpu.erase(tpu.begin() + i);
			//cout<<"check: "<<tpu.size()<<endl;
			compute(_db, curRS[1], _src, _dest, tpu, _depth-1);
		}
		else if(_pres[i].op == '|')
		{
			pnum = _pres[i].pset.size();
			curRS = new ResultType[pnum];
			for(int j = 0; j < pnum; ++j)
			{
				tpu[i].pset = vector<string>(1, _pres[i].pset[j]);
				compute(_db, curRS[j], _src, _dest, tpu, _depth-1);
			}
		}

		merge(_rs, curRS, pnum);
		//cout<<"check rs: "<<_rs.size()<<endl;
		delete[] curRS;
		return true;
	}

	//TODO: P1*P2* or (P1P2)*
	if(_pres.size() > 1)
	{
		cout<<"ERROR: too mnay P*, not supported now!"<<endl;
		return false;
	}
	//BETTER: deal with P*, using DFS to avoid circle
	ResultType* curRS = new ResultType[2];
	vector<PreUnit> tpu = _pres, tpu2;
	tpu.push_back(tpu[0]); tpu[1].op = ' ';
	//DEBUG: we set the maxium depth to 20 here to avoid circles
	compute(_db, curRS[0], _src, _dest, tpu2, _depth-1);
	compute(_db, curRS[1], _src, _dest, tpu, _depth-1);
	merge(_rs, curRS, 2);
	delete[] curRS;

	return true;
}

void output(ResultType& _rs, FILE* _fp = NULL)
{
	//transform ID to string and output paths
	int rowNum = _rs.size();
	//if(rowNum == 0)
	//{
		//fprintf(_fp, "[Empty]\n");
		//return;
	//}
	cout<<"path num: "<<rowNum<<endl;
	//fprintf(_fp, "path num: %d\n", rowNum);

	for(ResultIterator it = _rs.begin(); it != _rs.end(); ++it)
	{
		//NOTICE: each row may have different column num
		int colNum = (*it).size();
		for(int j = 0; j < colNum; ++j)
		{
			string str;
			TYPE_ENTITY_LITERAL_ID id = (*it)[j];
			//cout<<"ele: "<<id<<endl;
			if(id == ONE_EDGE)
			{
				str = "connected-with-one-edge\t";
			}
			else if(id == NO_EDGE)
			{
				str = "connected-with-no-edge\t";
			}
			else
			{
				if(Util::is_entity_ele(id))
				{
					str = kvstore->getEntityByID(id);
				}
				else
				{
					str = kvstore->getLiteralByID(id);
				}
			}

			fprintf(_fp, "%s\t", str.c_str());
		}
		fprintf(_fp, "\n");
	}
}

int
main(int argc, char* argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;
//#endif
	if(argc < 3)  //./gbuild
	{
		//output help info here
		cout << "the usage of gpath: " << endl;
		cout << "bin/gpath -s dataset_name" << endl;
		cout << "or use an existing database:"<<endl;
		cout << "bin/gpath -d database_name" << endl;
		cout <<"after the databse is built/loaded, you can input queries in the console"<<endl;
		return 0;
	}
	//system("clock");

	string opt = string(argv[1]);
	string name = string(argv[2]);
	Database db(name);
	if(opt == "-s")
	{
		bool flag = db.build(name);
		if (flag)
		{
			cout << "import RDF file to database done." << endl;
		}
		else
		{
			cout << "import RDF file to database failed." << endl;
		}
	}
	else if(opt == "-d")
	{
		db.load();
		cout << "finish loading" << endl;
	}
	else
	{
		cout<<"ERROR: invalid option!"<<endl;
		return -1;
	}

	//assign kvstore
	kvstore = db.getKVstore();
	//cout<<"kvstore check"<<endl;
	//cout<<kvstore->getEntityByID(0)<<endl;
	//cout<<kvstore->getIDByEntity("<ub:University>")<<endl;

	//query console - input or a file
	string query;
	//char resolved_path[PATH_MAX+1];
#ifdef READLINE_ON
	char *buf, prompt[] = "gsql>";
	//const int commands_num = 3;
	//char commands[][20] = {"help", "quit", "sparql"};
	printf("Type `help` for information of all commands\n");
	printf("Type `help command_t` for detail of command_t\n");
	rl_bind_key('\t', rl_complete);
	while (true)
	{
		buf = readline(prompt);
		if (buf == NULL)
			continue;
		else
			add_history(buf);
		if (strncmp(buf, "help", 4) == 0)
		{
			if (strcmp(buf, "help") == 0)
			{
				//print commands message
				printf("help - print commands message\n");
				printf("quit - quit the console normally\n");
				printf("sparql - load query from the second argument\n");
			}
			else
			{
				//BETTER: help for a given command
			}
			continue;
		}
		else if (strcmp(buf, "quit") == 0)
			break;
		else if (strncmp(buf, "sparql", 6) != 0)
		{
			printf("unknown commands\n");
			continue;
		}
		//BETTER: sparql + string, not only path
		string query_file;
		//BETTER:build a parser for this console
		bool ifredirect = false;

		char* rp = buf;
		int pos = strlen(buf) - 1;
		while (pos > -1)
		{
			if (*(rp + pos) == '>')
			{
				ifredirect = true;
				break;
			}
			pos--;
		}
		rp += pos;

		char* p = buf + strlen(buf) - 1;
		FILE* fp = stdout;      ///default to output on screen
		if (ifredirect)
		{
			char* tp = p;
			while (*tp == ' ' || *tp == '\t')
				tp--;
			*(tp + 1) = '\0';
			tp = rp + 1;
			while (*tp == ' ' || *tp == '\t')
				tp++;
			fp = fopen(tp, "w");	//NOTICE:not judge here!
			p = rp - 1;					//NOTICE: all separated with ' ' or '\t'
		}
		while (*p == ' ' || *p == '\t')	//set the end of path
			p--;
		*(p + 1) = '\0';
		p = buf + 6;
		while (*p == ' ' || *p == '\t')	//acquire the start of path
			p++;
		//there are also readlink and getcwd functions for help
		//http://linux.die.net/man/2/readlink
		//NOTICE:getcwd and realpath cannot acquire the real path of file
		//in the same directory and the program is executing when the
		//system starts running
		//NOTICE: use realpath(p, NULL) is ok, but need to free the memory
		char* q = realpath(p, NULL);	//QUERY:still not work for soft links
		if (q == NULL)
		{
			printf("invalid path!\n");
			free(q);
			free(buf);
			continue;
		}
		else
			printf("%s\n", q);
		//query = getQueryFromFile(p);
		query = Util::getQueryFromFile(q);
		if (query.empty())
		{
			free(q);
			//free(resolved_path);
			free(buf);
			if (ifredirect)
				fclose(fp);
			continue;
		}
		printf("query is:\n");
		printf("%s\n\n", query.c_str());

	//freopen("ans.txt", "w", stdout); 
		//property path handling: the query format is s regex_path t
		//NOTICE: we only find one path if reachable, or empty if not reachable
		unsigned src = INVALID, dest = INVALID;
		vector<PreUnit> pres;
		convert(query, src, dest, pres);
		string src_str = kvstore->getEntityByID(src);
		string dest_str;
		if(Util::is_entity_ele(dest))
		{
			dest_str = kvstore->getEntityByID(dest);
		}
		else
		{
			dest_str = kvstore->getLiteralByID(dest);
		}

		//BETTER: when query() returns, it should be IDs instead of Strings?
		ResultType rs;
	long tv_begin = Util::get_cur_time();
		compute(db, rs, src, dest, pres);
	long tv_end = Util::get_cur_time();
	cout << "this query uses " << (tv_end - tv_begin) << "ms." << endl;

		cout<<"the source is: "<<src_str<<endl;
		cout<<"the destination is: "<<dest_str<<endl;
		output(rs, fp);
	//fclose(stdout);

		//string msg;
		free(q);
		//free(resolved_path);
		free(buf);
		if (ifredirect)
			fclose(fp);
	}
#endif

	//system("clock");
	return 0;
}

