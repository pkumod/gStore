/*===========================================================================
* Filename: Trie.h
* Author: Zongyue Qin
* Mail: qinzongyue@pku.edu.cn
* Last modified	2018-01-15
* Description:
* This data structure is used to extract common prefixes from a given dataset,
* and compress/uncompress the dataset according to those prefixes.
==============================================================================*/
#include "TrieNode.h"
#include "../Util/Triple.h"
#include "../Util/Util.h"
#include "../Parser/RDFParser.h"
#include <vector>
#include <map>
using namespace std;

class Trie {
  static const int PREFIX_UPBOUND = 10000;
  bool going;
  string store_path;
  vector<string> dictionary;
  map<string, int> treeMap;
  map<string, int> s2int;
  string* int2s;
  char* TrieBuffer;
  int prefixnum;

  public:
  static const int BUILDMODE = 0;
  static const int QUERYMODE = 1;

  Trie();
  ~Trie();

  void Addstring(string& s);
  string Compress(string& _str);
  void BuildPrefix();
  bool loadStringPrefix();
  bool saveStringPrefix();
  void FindPrefixSet();
  void append(string& ret, string _str);
  void Uncompress(const char* _str, const int len, string& target_string);
  inline void Uncompress(const char* _str, const int len, string& target_string, char* buffer)
  {
    if (going == false) {
      target_string = string(_str, len);
      return;
    }
    int loc = 0;  // for the Compressed
    int uloc = 0; //for the UnCompressed
    short t, t1, t2, tminus;
    while (loc < len) {
      t1 = ((((short)_str[loc]) & 0xFF) << 8) & 0xFF00;
      t2 = (((short)_str[loc + 1]) & 0xFF);
      t = t1 | t2;
      if (t < 0) //[-len][str]
      {
        loc += 2;
        tminus = -t;
        memcpy(buffer + uloc, _str + loc, tminus);
        uloc += tminus;
        loc += tminus;
      } else //[order]
      {
        loc += 2;
        if (t >= this->prefixnum) {
          cerr << "t=" << t << "  prefix=" << prefixnum << endl;
          cerr << "err" << endl;
          target_string = "";
          return;
        }
        int size = this->int2s[t].size();
        memcpy(buffer + uloc, this->int2s[t].c_str(), size);
        uloc += size;
      }
    }
    target_string.assign(buffer, uloc);
    return;
  }
  void SetStorePath(string _path);
  string Uncompress(const string& _str, const int len);
};
