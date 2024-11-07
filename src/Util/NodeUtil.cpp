#include "NodeUtil.h"

namespace gutil
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
        std::string _output;
        size_t len = _node_str.size();
        if (_node_str[0] == '<' && _node_str[len-1] == '>')
        {
            _output = _node_str.substr(1, len-2);
        } 
        else if (_node_str[0] == '<') 
        {
            _output = _node_str.substr(1, len-1);
        }
        else if (_node_str[len-1] == '>') 
        {
            _output = _node_str.substr(0, len-2);
        }
        return _output;
    }
}