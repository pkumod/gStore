#pragma once

#include "GlobalTypedef.h"

using namespace std;
namespace gutil {

    class StringUtil 
    {
    public:
        StringUtil();
        ~StringUtil();
    public:
        static std::string trimLeft(const std::string& str);
        static std::string trimRight(const std::string& str);
        static std::string trim(const std::string& str);
        static int compare(const char* _str1, unsigned long _len1, const char* _str2, unsigned long _len2);
        static bool start_with(const std::string& str, const std::string& prefix);
        static bool contains(const string& _parent, const string& _child);
	    static void append(string& str, const char suffix);
        static std::string url_encode(const std::string& str);
        static std::string url_decode(const std::string& str);
        static std::string clear_linebreak(const std::string& str);
        static std::string replace_all(const std::string& str, const std::string oldtext, const std::string newtext);
        static void lower_case(std::string& str);
        static void upper_case(std::string& str);
        static std::string join(const std::vector<std::string>& vec, const std::string& delimiter);
    };
}