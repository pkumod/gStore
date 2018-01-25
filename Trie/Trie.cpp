/*=============================================================================
 * Filename: Trie.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last modified: 2018-01-15
 * Description: Implementing functions in Trie.h
 ==============================================================================*/
#include "Trie.h"

using namespace std;

Trie::Trie()
{
	root = new TrieNode;
	curID = 0;
	dictionary.clear();
}

/* choose first SAMPLE_UPBOUND triples in _rdf_file as a sample, builind Trie
 */
Trie::Trie(const string _rdf_file, string _store_path)
{
	cout << "begin building Trie..." << endl;

	long t1 = Util::get_cur_time();

	dictionary.clear();

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		cout << "Trie::Trie: Fail to open : " << _rdf_file << endl;
		root = NULL;
		return;
	}
	
	store_path = _store_path;
	root = new TrieNode;
	curID = 0;
	TripleWithObjType* triple_array = new TripleWithObjType[Trie::SAMPLE_UPBOUND];
	RDFParser _parser(_fin);
	int total_parse_triple_num = 0;

	while (total_parse_triple_num < Trie::SAMPLE_UPBOUND)
	{
		int parse_triple_num = 0;

		_parser.parseFileSample(triple_array, parse_triple_num, SAMPLE_UPBOUND);

		if (parse_triple_num == 0)
		{
			break;
		}
		
		for (int i = 0; i < parse_triple_num; i++)
		{
			string _sub = triple_array[i].getSubject();
			root->addString(_sub, -1);

			string _pre = triple_array[i].getPredicate();
			root->addString(_pre, -1);

			string _obj = triple_array[i].getObject();
			if (triple_array[i].isObjEntity())
			{
				root->addString(_obj, -1);
			}
			// ignore object if it's literal
		}
		
		total_parse_triple_num += parse_triple_num;
	}

	_fin.close();
	delete [] triple_array;
	triple_array = NULL;

	long t2 = Util::get_cur_time();
	cout << "Finish building trie, cost " << t2 - t1 << "ms." << endl;
}

/* return if the trie is built successfully */
bool
Trie::isInitialized()
{
	return this->root != NULL;
}

/* Write down dictionary */
bool
Trie::WriteDown()
{
	ofstream dictionary_out(store_path.c_str());
	if (!dictionary_out)
	{
		cout << "Trie::WriteDown: Fail to open " << store_path << endl;
		return false;
	}

	WriteDownNode(root, dictionary_out, "/");

	dictionary_out.close();

	return true;
}

/* Write Down node whose count is larger than LOWBOUND, recursively */
void
Trie::WriteDownNode(TrieNode *_node, ofstream& _fout, string _str)
{
	string curString = _node->getString();
	int curCount;

	if ((curCount = _node->getCount()) > Trie::LOWBOUND)
	{
		_node->ID = curID++;

		_fout << _node->ID << ' ' << _str + curString;
		
		/*if (curString == "") // root node
		{
			_fout << "/";
		} */
		_fout << endl;
	}

	TrieNode *pointer;
	
	if ((pointer = _node->getRbro()) != NULL)
	{
		WriteDownNode(pointer, _fout, _str);
	}

	/* if it has no child or the count of itself is lower than LOWBOUND, 
 	 * no need to visit its children
 	 */
	if ((pointer = _node->getLchd()) != NULL && curCount > Trie::LOWBOUND)
	{
		WriteDownNode(pointer, _fout, _str + curString);
	}	

	return;
} 

/* Return compressed Triple */
TripleWithObjType 
Trie::Compress(const TripleWithObjType &_in_triple, int MODE)
{
	int lowbound = (MODE == BUILDMODE) ? Trie::LOWBOUND : 0;
	
	string _in_sub = _in_triple.getSubject();
	string _sub = root->compress(_in_sub, lowbound);

	string _in_pre = _in_triple.getPredicate();
	string _pre = root->compress(_in_pre, lowbound);

	string _in_obj = _in_triple.getObject();
	string _obj;

	if (_in_triple.isObjEntity())
	{
		_obj = root->compress(_in_obj, lowbound);
	}
	else /* object is literal, no compression */
	{
		_obj = "-1/" + _in_obj;
	}

	TripleWithObjType _out_triple = _in_triple;
	
	_out_triple.setSubject(_sub);
	_out_triple.setPredicate(_pre);
	_out_triple.setObject(_obj);

	return _out_triple;
}

string
Trie::Compress(string _str)
{
	int lowbound = 0;

	if (Util::isLiteral(_str))
	{
		return "-1/" + _str;
	}

	if ((_str[0] >= '0' && _str[0] <= '9') || (_str[0] == '-' && 
	     _str[1] == '1'))	// already been compressed
		return _str;
	
	return root->compress(_str, lowbound);
}

/* Load dictionary to uncompress */
bool 
Trie::LoadDictionary()
{
	ifstream _fin(this->store_path.c_str());
	if (!_fin)
	{
		cout << "Trie::LoadDictionary: Fail to open " << store_path 
		     << endl;
		return false;
	}

	int dictionaryID, cnt = 0;
	string dictionaryEntry;
	dictionary.clear();

	while (_fin >> dictionaryID >> dictionaryEntry)
	{
		if (dictionaryID != cnt++)
		{
			cout << "DictionaryID mismatch: " << dictionaryID << endl;
			return false;
		}
		if (dictionaryEntry.length() == 1)	// root
		{
			dictionary.push_back("/");
		}
		else
		{
			dictionary.push_back(dictionaryEntry.substr(1, //2
						dictionaryEntry.length() - 1)); //2
		}
	}

	_fin.close();
	return true;
}

string
Trie::Uncompress(const char *_str, const int len)
{
	if (len == 0)
		return "";

	if ((_str[0] < '0' || _str[0] > '9') && 
	   !(_str[0] == '-' && _str[1] == '1'))	// _str is not compressed
	{
		string ret = _str;
		return ret;
	}

	//cout << "Uncompress: original str = " << _str << ' ' << len << endl;
	if (dictionary.empty())
	{
		if(!LoadDictionary())
		{
			exit(0);
		}
	}
	int dictionaryID;
	char *tmp_str = new char [len + 1];
	if (tmp_str == NULL)
	{
		cout << "Trie::Uncompress Error, failed new " << endl;
		exit(0);
	}
	sscanf(_str, "%d %s", &dictionaryID, tmp_str);
	string strPiece = tmp_str;
	delete [] tmp_str;

	string retval;

	if (dictionaryID < 0) /* _str is literal */
	{
		return strPiece.substr(1, strPiece.length() - 1);
	}		
	else
	{
		int strLen = strPiece.length();

		if (dictionaryID == 0)
		{
			return strPiece.substr(1, strLen - 1);
		}

		if (strLen > 1)
		{
			return dictionary[dictionaryID] + strPiece.substr(1, 
                               strLen - 1);
		}
		else
		{
			return dictionary[dictionaryID];
		}
	}

}

string
Trie::Uncompress(const string _str, const int len)
{
	return Uncompress(_str.data(), len);
}

/* addstring:
 * Insert a string into Trie
 */
void
Trie::addString(string _str, int _ID)
{
	root->addString(_str, _ID);
}

/* LoadTrie:
 * Load Trie into main memory
 */
bool
Trie::LoadTrie(string dictionary_path)
{
	this->Release();
	root = new TrieNode;	
	store_path = dictionary_path;
	
	ifstream fin(store_path.c_str());
	if (!fin)
	{
		cout << "Trie::LoadTrie: Fail to open " << store_path << endl;
		return false;
	}	
	
	int dictionaryID;
	string dictionaryEntry;

	while (fin >> dictionaryID >> dictionaryEntry)
	{
		if (dictionaryEntry == "/")	// corresponding to root node
		{
			dictionaryEntry = "";
		}
		else
		{
			dictionaryEntry = dictionaryEntry.substr(1, 
				  dictionaryEntry.length() - 1);
		}

		addString(dictionaryEntry, dictionaryID);
	}

	return true;
}

void
Trie::Release()
{
	dictionary.clear();
	Release(root);
}

void
Trie::Release(TrieNode *node)
{
	if (node == NULL) return;
	
	Release(node->getLchd());
	Release(node->getRbro());

	delete node;
}

Trie::~Trie()
{
	this->Release();
}
