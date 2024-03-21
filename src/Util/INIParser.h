#ifndef INI_PARSER_H
#define INI_PARSER_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <map>
using namespace std;
class ININode
{
public:
    ININode(string root, string key, string value)
    {
        this->root = root;
        this->key = key;
        this->value = value;
    }
    string root;
    string key;
    string value;
};

class SubNode
{
public:
    void InsertElement(string key, string value)
    {
        sub_node.insert(pair<string, string>(key, value));
    }
    map<string, string> sub_node;
};

class INIParser
{
public:
    int ReadINI(string path);
    string GetValue(string root, string key);
    vector<ININode>::size_type GetSize() { return map_ini.size(); }
    vector<ININode>::size_type SetValue(string root, string key, string value);
    int WriteINI(string path);
    void Clear() { map_ini.clear(); }
private:
    map<string, SubNode> map_ini;
};
#endif // INI_PARSER_H
