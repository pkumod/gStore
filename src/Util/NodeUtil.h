#pragma once

#include "GlobalTypedef.h"

using namespace std;
namespace gutil {

    /**
     * @brief A utility class for handling nodes.
     * 
     */
    class NodeUtil {
    public:
        NodeUtil();
        ~NodeUtil();
    public:
        static std::string node2string(const char* _raw_str);
        static std::string clear_angle_brackets(const string& _node_str);
        static std::string clear_angle_brackets_and_prefix(const string& _node_str);
    };
}