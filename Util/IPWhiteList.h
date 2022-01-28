/*
 * @Author: your name
 * @Date: 2021-12-10 20:45:51
 * @LastEditTime: 2021-12-14 15:56:14
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gstore/Util/IPWhiteList.h
 */
/*=============================================================================
# Filename: IPWhiteList.h
# Author: Jing Li
# Mail: li_jing@pku.edu.cn
# Last Modified: 2020-12-08 16:02
# Description: IP white list
=============================================================================*/


#include "Util.h"

class IPWhiteList{
public:
    IPWhiteList();
    ~IPWhiteList();
    std::set<std::string> ipList;
    //std::regex ipPattern("(1\\d{1,2}|2[0-4]\\d|25[0-5]|\\d{1,2})\\." + "(1\\d{1,2}|2[0-4]\\d|25[0-5]|\\d{1,2})\\."
     //                + "(1\\d{1,2}|2[0-4]\\d|25[0-5]|\\d{1,2})\\." + "(1\\d{1,2}|2[0-4]\\d|25[0-5]|\\d{1,2})")
    void Init();
    void Load(std::string file);
    void ReadIPFromFile(std::string file);
    bool Check(std::string ip);
    void InsertIPToFile(std::string file,std::string ip,std::string reason);
    //bool AddIP(std::string ip);
    void UpdateIPToFile(std::string file, vector<std::string>& ips, std::string reason);

};