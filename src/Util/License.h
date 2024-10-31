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
#include "../Api/NlohmanJson.hpp"
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
        desc = "";
    }
    time_t time_to_stamp(std::string time)
    {
        struct tm *tm = (struct tm *)malloc(sizeof(struct tm));
        strptime(time.c_str(), "%Y-%m-%d %H:%M:%S", tm);
        time_t stamp = mktime(tm);
        return stamp;
    }
    bool validDate()
    {
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
        time_t _timer = time(0);
        struct tm *tm_now = localtime(&_timer);
        tm_now->tm_hour = 0;
        tm_now->tm_min = 0;
        tm_now->tm_sec = 0;
        time_t currentTime = mktime(tm_now);

        time_t resStart = time_to_stamp(startdate + " 00:00:00");
        time_t resEnd = time_to_stamp(enddate + " 23:59:59");
        if (currentTime >= resStart && currentTime <= resEnd)
        {
            desc = "The license has been successfully verified and is valid from " + startdate + " to " + enddate;
            isvalid = true;
        }
        else
        {
            desc = "The license has expired, from " + startdate + " to " + enddate;
            isvalid = false;
        }
        return isvalid;
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LicenseInfo, isvalid, product, version, cpu, mac, startdate, enddate, company, type, desc);

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
};