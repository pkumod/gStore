#include "NodeUtil.h"

namespace gs
{

    NodeUtil::NodeUtil()
    {
    }

    NodeUtil::~NodeUtil()
    {
    }

    std::string NodeUtil::node2string(const char* _raw_str)
    {
        string _output;
        unsigned _first_quote = 0;
        unsigned _last_quote = 0;
        bool _has_quote = false;
        for (unsigned i = 0; _raw_str[i] != '\0'; i++) {
            if (_raw_str[i] == '\"') {
                if (!_has_quote) {
                    _first_quote = i;
                    _last_quote = i;
                    _has_quote = true;
                }
                else {
                    _last_quote = i;
                }
            }
        }
        if (_first_quote==_last_quote) {
            _output += _raw_str;
            return _output;
        }
        for (unsigned i = 0; i <= _first_quote; i++) {
            _output += _raw_str[i];
        }
        for (unsigned i = _first_quote + 1; i < _last_quote; i++) {
            switch (_raw_str[i]) {
            case '\n':
                _output += "\\n";
                break;
            case '\r':
                _output += "\\r";
                break;
            case '\t':
                _output += "\\t";
                break;
            case '\"':
                _output += "\\\"";
                break;
            case '\\':
                _output += "\\\\";
                break;
            default:
                _output += _raw_str[i];
            }
        }
        for (unsigned i = _last_quote; _raw_str[i] != 0; i++) {
            _output += _raw_str[i];
        }
        return _output;
    }

    std::string NodeUtil::clear_angle_brackets(const string& _node_str)
    {
        string result = _node_str;
        if (!result.empty() && result[0] == '<') {
            result.erase(result.begin());
        }
        if (!result.empty() && result[result.size() - 1] == '>') {
            result.erase(result.end() - 1);
        }
        return result;
    }

    std::string NodeUtil::clear_angle_brackets_and_prefix(const string& _node_str)
    {
        string result = _node_str;
        if (!result.empty() && result[0] == '<') {
            result.erase(result.begin());
        }
        if (!result.empty() && result[result.size() - 1] == '>') {
            result.erase(result.end() - 1);
        }
        if (!result.empty() && result.find('http') != string::npos) {
            size_t start = result.find("http");
            if (result.find("#") != string::npos)
            {
                size_t end = result.find_last_of("#");
                result.erase(start, end - start + 1);
            } else if (result.find("/") != string::npos) {
                size_t end = result.find_last_of("/");
                result.erase(start, end - start + 1);
            }
        }
        return result;
    }
}