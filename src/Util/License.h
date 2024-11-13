#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/buffer.h>
#include <map>
#include <regex>
#include <stdexcept>
#include <algorithm>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include "nlohmann/json.hpp"
#include "Slog.h"

using namespace std;
using namespace nlohmann;

struct LicenseInfo
{
public:
    bool isvalid;
    std::string product;
    std::string version;
    std::string cpu;
    std::string mac;
    std::string startdate;
    std::string enddate;
    std::string company;
    std::string type;
    std::string desc;
    std::string content;
    void reset()
    {
        isvalid = false;
        product = "";
        version = "";
        cpu = "";
        mac = "";
        startdate = "";
        enddate = "";
        company = "";
        type = "";
        desc = "Please import the license first";
        content= "";
    }
    time_t time_to_stamp(const std::string& time)
    {
        struct tm timeStruct = {0};
        const char* format = "%Y-%m-%d %H:%M:%S";
        strptime(time.c_str(), format, &timeStruct);
        time_t stamp = mktime(&timeStruct);
        return stamp;
    }
    bool validDate()
    {
        if (!isvalid)
        {
            return isvalid;
        }
        if(startdate.empty())
        {
            desc = "the license is not complete!";
            isvalid = false;
            return isvalid;
        }
        if(enddate.empty())
        {
            desc = "the license is not complete!";
            isvalid = false;
            return isvalid;
        }
        std::time_t now = std::time(0);
        struct tm *tm_now = std::localtime(&now);
        time_t currentTime = mktime(tm_now);
        string st = startdate + " 00:00:00";
        string et = enddate + " 23:59:59";
        time_t resStart = time_to_stamp(st);
        time_t resEnd = time_to_stamp(et);
        SLOG_CORE("now:" + std::to_string(currentTime) + " start:" + std::to_string(resStart) + " end:" + std::to_string(resEnd));
        if (currentTime >= resStart && currentTime <= resEnd)
        {
            desc = "The license is valid, from " + startdate + " to " + enddate;
            isvalid = true;
        }
        else
        {
            desc = "The license has expired, from " + startdate + " to " + enddate;
            isvalid = false;
        }
        return isvalid;
    }
    void fromJSON(nlohmann::json &j)
    {
        j.at("product").get_to(product);
        j.at("version").get_to(version);
        j.at("company").get_to(company);
        j.at("startdate").get_to(startdate);
        j.at("enddate").get_to(enddate);
        if (j.contains("isvalid"))
        {
            j.at("isvalid").get_to(isvalid);
        }
        if (j.contains("cpu"))
        {
            j.at("cpu").get_to(cpu);
        }
        if (j.contains("mac"))
        {
            j.at("mac").get_to(mac);
        }
        if (j.contains("type"))
        {
            j.at("type").get_to(type);
        }
        if (j.contains("desc"))
        {
            j.at("desc").get_to(desc);
        }
        if (j.contains("content"))
        {
            j.at("content").get_to(content);
        }
    }
};

inline void to_json(nlohmann::json& nlohmann_json_j, const LicenseInfo& nlohmann_json_t) 
{ 
    nlohmann_json_j["isvalid"] = nlohmann_json_t.isvalid; 
    nlohmann_json_j["product"] = nlohmann_json_t.product; 
    nlohmann_json_j["version"] = nlohmann_json_t.version; 
    nlohmann_json_j["cpu"] = nlohmann_json_t.cpu; 
    nlohmann_json_j["mac"] = nlohmann_json_t.mac; 
    nlohmann_json_j["startdate"] = nlohmann_json_t.startdate; 
    nlohmann_json_j["enddate"] = nlohmann_json_t.enddate; 
    nlohmann_json_j["company"] = nlohmann_json_t.company; 
    nlohmann_json_j["type"] = nlohmann_json_t.type; 
    nlohmann_json_j["desc"] = nlohmann_json_t.desc; 
};

struct LicenseFileContext 
{
    std::string content;
    std::string notify_msg;
};

class LicenseHelper{
private:
    const std::string pub_key = "-----BEGIN PUBLIC KEY-----\
    \nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA5hiAM5dwFokCmHqyIiFs\
    \n59bF4BttmFGANVXBfZoAyfKfEabWuF9FEkXwdwo641DjDX83z/P9C+2+4xV1CnTj\
    \nR6RKbir4IlvIEjPSe1MQrknzrsF0FyYlLwG8KFJDgE5ZAKdqVRYKRp8BZVu8N3/y\
    \nV4HDXQoEwWlw/wn2S5GtRfYZSUhKS5SXfjMxkuA/ZEQ2M4gWCmfkcVH4J+ohVJat\
    \nLKxFQUscYe4/mHCqJe7f8s/0nahq3wGs9j+LcbV6LTGSeY+iz8GrFlV/tjNiCmHV\
    \nI815RrFsap1uKXEZSonbP8wplzTtwjPkJhLOUF1KjBnBcn8Mdwq/CqU+xRjIl1j1\
    \nQwIDAQAB\n-----END PUBLIC KEY-----\n";
private:
    bool file_exist(const char* _path);
    std::string rsa_pub_decrypt(const std::string &cipherText);
    std::vector<char> hexToBytes(const std::string &hex);
    std::string removeSpaces(const std::string &str);
    std::string base64Encode(const std::vector<char> &data);
    std::string base64Decode(const std::string &encodedData);
    std::string loadBinaryFile(const std::string &filename);
    std::string loadHexFile(const std::string &filename);
    bool getCPUID(std::string &cpuid);
    std::string getMacAddress();
    bool getMacAddress(std::string &mac);
public:
    LicenseHelper();
    ~LicenseHelper();
    bool validLicense(struct LicenseInfo &licenseInfo, const std::string licenseFile);
    bool validLicense(struct LicenseInfo &licenseInfo, const char* licenseContent);
};