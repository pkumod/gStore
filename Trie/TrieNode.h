/*============================================================================
 * Filename: TrieNode.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@Pku.edu.cn
 * Last modified 2018-01-15
 * Description: 
 * Node of Trie
 * ===========================================================================*/
#include <cstdlib>
#include <string>

using namespace std;

class TrieNode {
  TrieNode* lchd;   // pointer to the most left child
  TrieNode* rbro;   // pointer to the nearest right brother
  TrieNode* lbro;   // pointer to the nearest left brother
  TrieNode* father; // pointer to its father
  string str;
  int count;

  public:
  int ID; // ID of the node, which is also used for compressing
  TrieNode();
  TrieNode(const string& str);
  void addCount();
  void setLchd(const TrieNode* _lchd);
  void setRbro(const TrieNode* _rbro);
  void setLbro(const TrieNode* _lbro);
  void setFather(const TrieNode* _father);
  void setString(const string& _str);
  int getCount();
  TrieNode* getLchd();
  TrieNode* getRbro();
  TrieNode* getLbro();
  TrieNode* getFather();
  string getString();
  void addString(string& _str, int _ID);
  string compress(string& _str, const int& lowbound);
  string split(string& _str);
};
