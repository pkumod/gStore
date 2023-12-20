/*=============================================================================
# Filename: IPBlackList.h
# Author: Jing Li
# Mail: li_jing@pku.edu.cn
# Last Modified: 2020-12-08 16:02
# Description: IP black list
=============================================================================*/

#include "Util.h"

class IPBlackList{
public:
    IPBlackList();
    ~IPBlackList();
    std::set<std::string> ipList;
    //std::regex ipPattern("(1\\d{1,2}|2[0-4]\\d|25[0-5]|\\d{1,2})\\." + "(1\\d{1,2}|2[0-4]\\d|25[0-5]|\\d{1,2})\\."
    //+ "(1\\d{1,2}|2[0-4]\\d|25[0-5]|\\d{1,2})\\." + "(1\\d{1,2}|2[0-4]\\d|25[0-5]|\\d{1,2})")
    void Init();
    void Load(std::string file);
    void ReadIPFromFile(std::string file);
    bool Check(std::string ip);
    //bool AddIP(std::string ip);
    void InsertIPToFile(std::string file,std::string ip,std::string reason);
    void UpdateIPToFile(std::string file, vector<std::string>& ips, std::string reason);
};