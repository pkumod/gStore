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
  this->TrieBuffer = new char[8388608];
  this->treeMap = map<string, int>();
  this->treeMap.insert(make_pair("<http://www.", 0));
  this->s2int = map<string, int>();
  this->prefixnum = 0;
  this->int2s = new string[32768];
  this->going = true;
}

void
Trie::SetStorePath(string _path)
{
  this->store_path = _path;
}

Trie::~Trie()
{
  saveStringPrefix();
  this->treeMap.clear();
  this->s2int.clear();
  if (going == true)
    delete[] this->int2s;
  delete[] this->TrieBuffer;
}
void
Trie::Addstring(string& _str)
{
  map<string, int>::iterator it;
  string tmp;
  int start = 0;
  int end = -1;
  if (_str.substr(0, 12) == string("<http://www.")) {
    start = 12;
    (this->treeMap.find(string("<http://www."))->second)++;
  }
  for (int i = start; i < _str.length(); i++) {
    if (_str[i] == '.' || _str[i] == '/' || _str[i] == '<' || _str[i] == '>' || _str[i] == '#' || _str[i] == '\n') {
      end = i - 1;
      if (start != i) {
        /*tmp = _str.substr(start, end - start + 1);*/
        tmp = _str.substr(start, end - start + 2);
        if (tmp.size() <= 4) {
          start = i + 1;
          continue;
        }
        it = this->treeMap.find(tmp);
        if (it == this->treeMap.end())
          this->treeMap.insert(make_pair(tmp, 1));
        else
          (it->second)++;
      }
      start = i + 1;
    }
  }
  if (start != _str.length()) {
    tmp = _str.substr(start, _str.length() - start);
    if (tmp.size() == 1)
      return;
    it = this->treeMap.find(tmp);
    if (it == this->treeMap.end())
      this->treeMap.insert(make_pair(tmp, 1));
    else
      (it->second)++;
  }
}

void
Trie::BuildPrefix()
{
  this->FindPrefixSet();
  return;
}

void
Trie::FindPrefixSet()
{
  multimap<long long, string> t;
  map<string, int>::iterator it1;
  long long tq;
  long long totalsize = 0;
  long long total_pattern_size = 0;
  for (it1 = this->treeMap.begin(); it1 != this->treeMap.end(); it1++) {
    if (it1->first.length() > this->PREFIX_UPBOUND)
      continue;
    tq = it1->second * (it1->first.length() - 2);
    totalsize += it1->second * (it1->first.length());
    total_pattern_size += 2 * it1->second;
    t.insert(make_pair(tq, it1->first));
  }

  multimap<long long, string>::iterator it2;
  long long save_size = 0;
  it2 = t.end();
  it2--;

  for (int i = 0; i < 32768 && (t.size() - i > 0); i++, it2--) {
    if (it2->first <= 0)
      break;
    save_size += it2->first / (it2->second.length() - 2) * (it2->second.length());
    int2s[i] = it2->second;
    this->s2int.insert(make_pair(it2->second, i));
  }

  if (totalsize == 0 || (save_size - total_pattern_size) * 1.0 / totalsize < 0.3) {
    cout << "Compressed Ratio: " << (save_size - total_pattern_size) * 1.0 / totalsize << " , Decide not to use Trie" << endl;
    going = false;
    prefixnum = 0;
    s2int.clear();
    delete[] int2s;
    int2s = NULL;
  } else {
    cout << "Compressed Ratio: " << (save_size - total_pattern_size) * 1.0 / totalsize << " , Decide to use Trie" << endl;
    going = true;
    prefixnum = s2int.size();
  }

  this->treeMap.clear();
  return;
}

void
Trie::append(string& ret, string _str)
{
  short substr_len;
  int t_start;
  map<string, int>::iterator it;
  it = this->s2int.find(_str);
  if (it == this->s2int.end()) //not find
  {
    substr_len = -_str.length();
    t_start = ret.size();
    /* append 2 char , the size of short
		[-size][str] */
    ret.append("00");
    ret[t_start] = (char)((substr_len >> 8) & 0xFF);
    ret[t_start + 1] = (char)(substr_len & 0xFF);
    ret.append(_str);
  } else //find  [order]
  {
    t_start = ret.size();
    ret.append("00");
    int order = it->second;
    ret[t_start] = (char)((order >> 8) & 0xFF);
    ret[t_start + 1] = (char)(order & 0xFF);
  }
}

string
Trie::Compress(string& _str)
{
  if (going == false)
    return _str;
  string ret;
  map<string, int>::iterator it;
  vector<string> vstring;
  vector<int> vint;
  int start = 0;
  int end = -1;
  if (_str.substr(0, 12) == string("<http://www.")) {
    vstring.push_back(string("<http://www."));
    vint.push_back(0);
    start = 12;
  }
  for (int i = start; i < _str.length(); i++) {
    if (_str[i] == '.' || _str[i] == '/' || _str[i] == '<' || _str[i] == '>' || _str[i] == '#' || _str[i] == '\n') {
      end = i - 1;
      if (start != i) {
        vstring.push_back(_str.substr(start, end - start + 2));
        it = this->s2int.find(_str.substr(start, end - start + 2));
        if (it == this->s2int.end())
          vint.push_back(-(end - start + 2));
        else
          vint.push_back(it->second);
      } else {
        vstring.push_back(_str.substr(i, 1));
        vint.push_back(-1);
      }
      start = i + 1;
    }
  }
  if (start != _str.length()) {
    vstring.push_back(_str.substr(start, _str.length() - start));
    it = this->s2int.find(_str.substr(start, _str.length() - start));
    if (it == this->s2int.end())
      vint.push_back(-(_str.length() - start));
    else
      vint.push_back(it->second);
  }
  int j;
  string tmp;
  for (int i = 0; i < vstring.size(); i++) {
    if (vint[i] >= 0)
      append(ret, vstring[i]);
    else {
      tmp = vstring[i];
      j = i + 1;
      while (j < vstring.size() && vint[j] < 0) {
        tmp += vstring[j];
        j++;
      }
      append(ret, tmp);
      i = j - 1;
    }
  }
  return ret;
}

/* save String Prefix to file, return true if succeed */
bool
Trie::saveStringPrefix()
{
  FILE* StringPrefix = fopen(this->store_path.c_str(), "w");

  if (StringPrefix == NULL) {
    cout << "Save String Prefixes ERROR" << endl;
    return false;
  }

  int nPrefix = s2int.size();

  char int2c[this->PREFIX_UPBOUND];
  sprintf(int2c, "%d", nPrefix);
  fputs(int2c, StringPrefix);
  fputc('\n', StringPrefix);

  if (nPrefix == 0) {
    fclose(StringPrefix);
    return true;
  }

  map<string, int>::iterator iter;

  for (iter = s2int.begin(); iter != s2int.end(); ++iter) {
    fputs(iter->first.c_str(), StringPrefix);
    fputc('\n', StringPrefix);
    sprintf(int2c, "%d", iter->second);
    fputs(int2c, StringPrefix);
    fputc('\n', StringPrefix);
  }
  /*
	sprintf(int2c, "%d", prefixnum);
	fputs(int2c, StringPrefix);
	fputc('\n', StringPrefix);

	for (int i = 0; i < prefixnum; ++i)
	{
		fputs(int2s[i].c_str(), StringPrefix);
		fputc('\n', StringPrefix);
	}
	*/
  fclose(StringPrefix);
  return true;
}

/* load String Prefix file, return true if succeed */
bool
Trie::loadStringPrefix()
{
  cout << "load Stirng Prefix" << endl;
  FILE* StringPrefix = fopen(this->store_path.c_str(), "r");
  if (StringPrefix == NULL) {
    cout << "Open String Prefix File ERROR" << endl;
    return false;
  }
  char tmp[this->PREFIX_UPBOUND + 1];

  fgets(tmp, this->PREFIX_UPBOUND, StringPrefix);
  int nPrefix = atoi(tmp);

  this->prefixnum = nPrefix;
  int2s = new string[prefixnum + 1];

  if (nPrefix == 0) {
    going = false;
    fclose(StringPrefix);
    return true;
  }

  for (int i = 0; i < nPrefix; ++i) {
    fgets(tmp, this->PREFIX_UPBOUND + 1, StringPrefix);
    string tmpPrefix = tmp;
    tmpPrefix = tmpPrefix.substr(0, (int)tmpPrefix.length() - 1);
    fgets(tmp, this->PREFIX_UPBOUND + 1, StringPrefix);
    int tmpprefixnum;
    if (tmp[0] == '\n') {
      tmpPrefix += "\n";
      fgets(tmp, this->PREFIX_UPBOUND + 1, StringPrefix);
      tmpprefixnum = atoi(tmp);
    } else {
      tmpprefixnum = atoi(tmp);
    }
    int2s[tmpprefixnum] = tmpPrefix;
    s2int.insert(pair<string, int>(tmpPrefix, tmpprefixnum));
  }

  /*
	fgets(tmp, PREFIX_UPBOUND, StringPrefix);
	this->prefixnum = atoi(tmp);
	int2s = new string[ prefixnum + 1];
	for (int i = 0; i < prefixnum; ++i)
	{
		fgets(tmp, PREFIX_UPBOUND, StringPrefix);
		if (tmp[0] == '\n')
		{
			i--;
			int2s[i] += "\n";
			continue;
		}
		int2s[i] = tmp;
		int2s[i] = int2s[i].substr(0, (int)int2s[i].length() - 1);
	}*/
  cout << "load String Prefix end" << endl;
  fclose(StringPrefix);
  return true;
}

void
Trie::Uncompress(const char* _str, const int len, string& target_string)
{
  this->Uncompress(_str, len, target_string, this->TrieBuffer);
  return;
}

string
Trie::Uncompress(const string& _str, const int len)
{
  string _ret;
  this->Uncompress(_str.c_str(), len, _ret);
  return _ret;
}
