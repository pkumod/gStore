/*=============================================================================
# Filename: IPWhiteList.cpp
# Author: Jing Li
# Mail: li_jing@pku.edu.cn
# Last Modified: 2020-12-08 16:02
# Description: IP white list
=============================================================================*/

#include "IPWhiteList.h"
#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace std;

IPWhiteList::IPWhiteList(){
    this->Init();
}

IPWhiteList::~IPWhiteList(){

}

void IPWhiteList::Init(){
    ipList.erase(ipList.begin(), ipList.end());
}

void IPWhiteList::Load(std::string file){
    this->Init();
    this->ReadIPFromFile(file);
}

void IPWhiteList::ReadIPFromFile(std::string file){
    ifstream infile(file.c_str());
    string line;
    if (!infile.is_open())
    {
        cout << "open white list file failed." << endl;
        return;
    }

    while(getline(infile, line)) {
        //cout << line << endl;
        if (line.length() >= 7) {
            int pos = line.find("#");
            if(pos != -1)
                continue;
            this->ipList.insert(line);
        }
    }
    infile.close();
}

bool IPWhiteList::Check(std::string ip){
    if(this->ipList.empty())
        return false;
    else if(this->ipList.find(ip) != this->ipList.end())
        return true;
    for(std::set<string>::iterator it=this->ipList.begin(); it!=this->ipList.end(); ++it){
        // cout << "case for xxx.xxx.xxx.xxx-xxx.xxx.xxx.xxx" << endl;
        string test = *it;
        if(test.find("-") != -1){
            std::vector<std::string> fields;
            std::vector<std::string> start;
            std::vector<std::string> end;
            std::vector<std::string> test_ip;
            boost::split( fields, test, boost::is_any_of( "-" ) );
            boost::split( start, fields[0], boost::is_any_of( "\\." ) );
            boost::split( end, fields[1], boost::is_any_of( "\\." ) );
            boost::split( test_ip, ip, boost::is_any_of( "\\." ) );
            bool res = true;
            for(int i = 0; i < 4; i++){
                int s = std::stoi(start[i]);
                int e = std::stoi(end[i]);
                int candidate = std::stoi(test_ip[i]);

                if(!(s <= candidate && candidate <= e)){
                    res = false;
                    break;
                }
            }
            if(res)
                return true;
        }
    }
    return false;
}