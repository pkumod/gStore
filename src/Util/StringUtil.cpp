#include "StringUtil.h"

namespace gutil
{
    StringUtil::StringUtil() {};
    StringUtil::~StringUtil() {};

    std::string StringUtil::trimLeft(const std::string &str)
    {
        std::string::size_type first = str.find_first_not_of(" \t\n\r\f\v");
        if (first == std::string::npos)
        {
            return "";
        }
        return str.substr(first);
    }

    std::string StringUtil::trimRight(const std::string &str)
    {
        std::string::size_type last = str.find_last_not_of(" \t\n\r\f\v");
        if (last == std::string::npos)
        {
            return "";
        }
        return str.substr(0, (last + 1));
    }

    std::string StringUtil::trim(const std::string &str)
    {
        std::string::size_type first = str.find_first_not_of(" \t\n\r\f\v");
        std::string::size_type last = str.find_last_not_of(" \t\n\r\f\v");

        if (first == std::string::npos || last == std::string::npos)
        {
            return "";
        }
        return str.substr(first, (last - first + 1));
    }

    int StringUtil::compare(const char *_str1, unsigned long _len1, const char *_str2, unsigned long _len2)
    {
        int ifswap = 1; // 1 indicate: not swapped
        if (_len1 > _len2)
        {
            const char *str = _str1;
            _str1 = _str2;
            _str2 = str;
            unsigned long len = _len1;
            _len1 = _len2;
            _len2 = len;
            ifswap = -1;
        }
        unsigned long i;
        // DEBUG: if char can be negative, which cause problem when comparing(128+)
        //
        // NOTICE:little-endian-storage, when string buffer poniter is changed to
        // unsigned long long*, the first char is the lowest byte!
        /*
        unsigned long long *p1 = (unsigned long long*)_str1, *p2 = (unsigned long long*)_str2;
        unsigned limit = _len1/8;
        for(i = 0; i < limit; ++i, ++p1, ++p2)
        {
        if((*p1 ^ *p2) == 0)	continue;
        else
        {
        if(*p1 < *p2)	return -1 * ifswap;
        else			return 1 * ifswap;
        }
        }
        for(i = 8 * limit; i < _len1; ++i)
        {
        if(_str1[i] < _str2[i])	return -1 * ifswap;
        else if(_str1[i] > _str2[i])	return 1 * ifswap;
        else continue;
        }
        if(i == _len2)	return 0;
        else	return -1 * ifswap;
        */
        for (i = 0; i < _len1; ++i)
        { // ASCII: 0~127 but c: 0~255(-1) all transfered to unsigned char when comparing
            if ((unsigned char)_str1[i] < (unsigned char)_str2[i])
                return -1 * ifswap;
            else if ((unsigned char)_str1[i] > (unsigned char)_str2[i])
                return 1 * ifswap;
        }
        if (i == _len2)
            return 0;
        else
            return -1 * ifswap;
    }

    bool StringUtil::start_with(const std::string& str, const std::string& prefix)
    {
        return str.rfind(prefix, 0) == 0;
    }

    bool StringUtil::contains(const string& _parent, const string& _child)
    {
        return _parent.find(_child) != string::npos;
    }

    void StringUtil::append(string& str, const char suffix)
    {
        if (str[str.length()-1] != suffix)
        {
            str.push_back(suffix);
        }
    }

    std::string StringUtil::url_encode(const std::string& str)
    {
        std::string strTemp = "";
        size_t length = str.length();
        unsigned char x;
        for (size_t i = 0; i < length; i++)
        {
            if (isalnum((unsigned char)str[i]) ||
                (str[i] == '-') ||
                (str[i] == '_') ||
                (str[i] == '.') ||
                (str[i] == '~'))
                strTemp += str[i];
            else if (str[i] == ' ')
                strTemp += "+";
            else
            {
                strTemp += '%';
                x = (unsigned char)str[i] >> 4;
                strTemp += x > 9 ? x + 55 : x + 48;
                x = (unsigned char)str[i] % 16;
                strTemp += x > 9 ? x + 55 : x + 48;
            }
        }
        return strTemp;
    }

    std::string StringUtil::url_decode(const std::string& str)
    {
        std::string strTemp = "";
        size_t length = str.length();
        unsigned char x;
        for (size_t i = 0; i < length; i++)
        {
            if (str[i] == '+')
                strTemp += ' ';
            else if (str[i] == '%')
            {
                assert(i + 2 < length);
                x = (unsigned char)str[++i];
                unsigned char high;
                if (x >= 'A' && x <= 'Z')
                    high = x - 'A' + 10;
                else if (x >= 'a' && x <= 'z')
                    high = x - 'a' + 10;
                else if (x >= '0' && x <= '9')
                    high = x - '0';
                else
                    assert(0);

                x = (unsigned char)str[++i];
                unsigned char low = 0;
                if (x >= 'A' && x <= 'Z')
                    low = x - 'A' + 10;
                else if (x >= 'a' && x <= 'z')
                    low = x - 'a' + 10;
                else if (x >= '0' && x <= '9')
                    low = x - '0';

                strTemp += high * 16 + low;
            }
            else
                strTemp += str[i];
        }
        return strTemp;
    }

    std::string StringUtil::clear_linebreak(const std::string& str)
    {
        string result = str;
        result.erase(remove(result.begin(), result.end(), '\n'), result.end());
        result.erase(remove(result.begin(), result.end(), '\r'), result.end());
        result.erase(remove(result.begin(), result.end(), '\t'), result.end());
        result.erase(remove(result.begin(), result.end(), '\v'), result.end());
        result.erase(remove(result.begin(), result.end(), '\f'), result.end());
        return result;
    }

    std::string StringUtil::replace_all(const std::string& str, const std::string oldtext, const std::string newtext)
    {
        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(oldtext, pos)) != std::string::npos) {
            result.replace(pos, oldtext.length(), newtext);
            pos += newtext.length();
        }
        return result;
    }

     void StringUtil::lower_case(std::string& str)
     {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
     }

     void StringUtil::upper_case(std::string& str)
     {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
     }
}