#pragma once

#include "../Util/Util.h"
#include "PFNEntity.h"

using namespace std;
using namespace gutil;

#define PFN_HEADER "#include \"../../../src/Query/Algorithm/PathQueryHandler.h\"\n\nusing namespace std;\n\n"


class PFNUtil
{
private:
    pthread_rwlock_t pfn_data_lock;
    std::string pfn_base_path;
    std::string pfn_cpp_path;
    std::string pfn_lib_path;
    std::string fun_cppcheck(const std::string username, struct PFNInfo *fun_info);
    std::string fun_build_source_data(struct PFNInfo * fun_info, bool has_header);
    void fun_write_json_file(const std::string& username, struct PFNInfo *fun_info, std::string operation);
    void fun_parse_from_name(const std::string& username, const std::string& fun_name, struct PFNInfo *fun_info);
    vector<string> get_files(const char *src_dir, const std::string& prefix);
public:
    PFNUtil();
    ~PFNUtil();
    void fun_query(const std::string &fun_name, const string &fun_status, const string &username, std::shared_ptr<PFNInfos> pfn_infos);
    void fun_create(const std::string &username, struct PFNInfo *pfn_info);
    void fun_update(const std::string &username, struct PFNInfo *pfn_info);
    void fun_delete(const std::string &username, struct PFNInfo *pfn_info);
    string fun_build(const std::string &username, const std::string fun_name);
    void fun_review(const std::string &username, struct PFNInfo *pfn_info);
};
