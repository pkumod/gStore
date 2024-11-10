/*
 * @Author: wangjian
 * @Date: 2021-12-20 16:35:18
 * @LastEditTime: 2023-02-09 13:38:24
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: api util
 * @FilePath: /gstore/GRPC/grpcUtil.h
 */
#pragma once

#include "../Database/Database.h"
#include "../Database/Txn_manager.h"
#include "../Util/Util.h"
#include "../Util/FileUtil.h"
#include "../Util/ResourceUtil.h"
#include "../Util/ThreadUtil.h"
#include "../Util/IPWhiteList.h"
#include "../Util/IPBlackList.h"
#include "../Util/CompressFileUtil.h"
#include "../Util/License.h"
#include "APIUserUtil.h"
#include "APIDatabaseUtil.h"
#include "APILogQueryUtil.h"

using namespace std;

class APIUtil
{
private:
    Util util;
    int32_t thread_pool_num;
    
    int32_t max_output_size;
    int32_t max_database_num;
    int32_t max_user_num;
    string query_log_mode;
    string query_log_path;
    string access_log_mode;
    string access_log_path;
    string query_result_path;
    std::vector<std::string> upload_allow_extensions;
    std::vector<std::string> upload_allow_compress_packages;

    // std::map<std::string, shared_ptr<Database>> databases;
    std::map<std::string, shared_ptr<DBUserInfo>> users;
    std::map<std::string, shared_ptr<DatabaseInfo>> already_build;
    std::map<std::string, shared_ptr<IpInfo>> ips;
    std::map<std::string, shared_ptr<Txn_manager>> txn_managers;
    std::map<std::string, txn_id_t> running_txn;

    shared_ptr<Database> system_database;
    pthread_rwlock_t users_map_lock;
    // pthread_rwlock_t databases_map_lock;
    pthread_rwlock_t already_build_map_lock;
    pthread_rwlock_t txn_m_lock;
    pthread_rwlock_t ips_map_lock;
    pthread_rwlock_t system_db_lock;
    string system_username;
    string system_password;
    string system_password_path;
    int connection_num = 0;
    int blackList = 0;
    int whiteList = 0;
    string ipBlackFile;
    string ipWhiteFile;
    std::unique_ptr<IPWhiteList> ipWhiteList;
    std::unique_ptr<IPBlackList> ipBlackList;

    pthread_rwlock_t query_log_lock;
    pthread_rwlock_t access_log_lock;
    pthread_rwlock_t transactionlog_lock;

    //license info
    struct LicenseInfo license_info;

    bool ip_check(const string& ip);
    bool ip_error_num_check(const string& ip);
    void init_params();
    bool update_sys_db(const string& query);
    bool refresh_sys_db();
    bool mv_or_cp(const string& src, const string& dsc, bool is_mv);
    /**
     * @function: update_privilege
     * @brief: update privilege of user
     * @param: userinfo: user info
     * @param: type: privilege type
     * @param: db_name: database name
     * @param: op: operation type 0=erase 1=add -1=clear all
     * @return: true if success, false if fail
     */
    bool update_privilege(std::shared_ptr<DBUserInfo>& userinfo, const string& type, const string& db_name, int16_t op);
    bool get_file_lines(vector<string> &lines, string &logFile, int &page_no, int &page_size, int &total_size, int &total_page, pthread_rwlock_t *rw_lock);
public:
    APIUtil();
    ~APIUtil();
    int initialize();
    void refresh_conf();
    bool trywrlock_already_build_map();
    bool unlock_already_build_map();
    bool init_databaseinfo(const std::string& db_name, const std::string creator, const std::string build_time, const DatabaseStatus status);
    bool update_database_status(const std::string& db_name, const DatabaseStatus status);
    bool remove_databaseinfo(const std::string& db_name, std::string msg);
    bool backup_databaseinfo(const std::string& db_name, const bool& compress, std::string& backup_path, std::string& msg);
    bool restore_databaseinfo(const std::string& username, const std::string& db_name, std::string& backup_path, std::string& msg);
    bool rename_databaseinfo(const std::string& db_name, const std::string& new_db_name, std::string& msg);
    bool get_databaseinfo(const std::string& db_name, shared_ptr<DatabaseInfo> &dbInfo);
    void get_databaseinfos(const std::string& username, vector<shared_ptr<DatabaseInfo>> &array);
    bool erase_databaseinfo(const std::string& username);
    bool wrlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo);
    bool trywrlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo);
    bool trywrlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo, const time_t& timeout_s);
    bool rdlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo);
    bool unlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo);

    bool get_txn_manager(const std::string& db_name, shared_ptr<Txn_manager> &txn_manager);
    bool insert_txn_manager(const std::string& db_name, shared_ptr<DatabaseInfo> &dbinfo);
    bool remove_txn_manager(const std::string& db_name, bool checkpoint);
    bool check_txn_id(const string& tid_s, txn_id_t& tid);
    bool get_txn_begin_time(shared_ptr<Txn_manager>& txn_m, txn_id_t& tid, std::string& begin_time);
    bool begin_process(const std::string& db_name, int level, std::string username, txn_id_t& tid);
    bool commit_process(shared_ptr<Txn_manager>& txn_m, txn_id_t& tid, std::string& msg);
    bool rollback_process(shared_ptr<Txn_manager>& txn_m, txn_id_t& tid, std::string& msg);
    bool aborted_process(shared_ptr<Txn_manager>& txn_m, txn_id_t& tid, std::string& msg);

    bool check_db_loaded(const std::string& db_name);
    bool check_db_built(const std::string& db_name);
    bool check_db_count();

    bool check_indentity(const std::string& username,const std::string& password,const std::string& encryption, std::string& msg);
    bool check_server_indentity(const std::string& password, std::string& msg);
    bool check_param_value(const string& paramname, const string& value, std::string& msg);
    bool check_user_exist(const std::string& username);
    bool check_user_count();
    bool add_privilege(const std::string& username, const vector<string>& types, const std::string& db_name);
    bool del_privilege(const std::string& username, const vector<string>& types, const std::string& db_name);
    bool check_privilege(const std::string& username, const std::string& type, const std::string& db_name);
    bool init_privilege(const std::string& username, const std::string& db_name);
    bool copy_privilege(const std::string& src_db_name, const std::string& dst_db_name);
    bool clear_privilege(const string& username);
    bool ask_sys_db(const std::string& sparql);
    bool query_sys_db(const std::string& sparql, ResultSet& _rs);

    //used by drop
    bool user_add(const string& username, const string& password);
    bool user_delete(const string& username);
    bool user_pwd_alert(const string& username, const string& password);
    void get_user_info(vector<shared_ptr<struct DBUserInfo>> &_users);

    string ip_enabled_type();
    void ip_list(const string& type, std::vector<std::string>& ip_list);
    bool ip_save(string ip_type, vector<string> ipVector);
    bool check_access_ip(const string& ip, int check_level, std::string& msg);
    void update_access_ip_error_num(const string& ip);
    void reset_access_ip_error_num(const string& ip);
    
    // for access log
    void get_access_log_files(std::vector<std::string> &file_list);
    void get_access_log(const string &date, int &page_no, int &page_size, shared_ptr<struct DBAccessLogs> logPtr);
    void write_access_log(const string &operation, const string &remoteIP, const int statusCode, const string &statusMsg, const string &optId = "");
    void update_access_log(int statusCode, string statusMsg, string opt_id, int state, int num, int failnum, string backupfilepath = "");
    bool getAccessLogByOptId(string opt_id, struct DBAccessLogInfo& log);

    // for query log
    void get_query_log_files(std::vector<std::string> &file_list);
    void get_query_log(const string &date, int &page_no, int &page_size, shared_ptr<struct DBQueryLogs> logPtr);
    void write_query_log(DBQueryLogInfo* log);

    // for transaction log
    void init_transactionlog();
	int add_transactionlog(std::string db_name, std::string user, std::string TID,  std::string begin_time, std::string status = "RUNNING",  std::string end_time = "INF");
	int update_transactionlog(std::string db_name, std::string status, std::string end_time);
	void get_transactionlog(int &page_no, int &page_size, shared_ptr<struct TransactionLogs> logPtr);

    // for license
    void init_license();
    bool check_license(std::string& msg);
    bool import_license(const string& license_file, std::string& msg);
    bool remove_license(std::string& msg);
    LicenseInfo& get_license();

    // for data get
    string get_query_result_path();
    int get_thread_pool_num();
    int get_max_output_size();
    int get_connection_num();
    void increase_connection_num();
    size_t get_upload_max_body_size();
    bool check_upload_allow_extensions(const string& suffix);
    bool check_upload_allow_compress_packages(const string& suffix);
};