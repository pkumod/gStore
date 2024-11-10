#pragma once
#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;

struct ReasonSparql
{
  /* data */
  string insert_sparql;
  string delete_sparql;
  string check_sparql;
  string error_message;
  int issuccess = 0;
};
struct ReasonOperationResult
{
  int issuccess = 0;
  string error_message;
};

class ReasonHelper
{
private:
  /* data */
public:
  ReasonHelper();
  ~ReasonHelper();
  static vector<string> getReasonRuleList(const string &db_path);
  static ReasonOperationResult getReasonInfo(const string &rulename, const string &db_path);
  static ReasonOperationResult removeReasonRule(const string &rulename, const string &db_path);
  static ReasonOperationResult saveReasonRuleInfo(nlohmann::json &ruleInfo, const string &db_path);
  static ReasonSparql compileReasonRule(const string &rulename, const string &db_path);
  static ReasonSparql executeReasonRule(const string &rulename, const string &db_path);
  static string updateReasonRuleInfo(nlohmann::json &ruleInfo, const string &db_path);
  static string updateReasonRuleStatus(const string &rulename, const string &status, const string &db_path);
  static ReasonSparql disableReasonRule(const string &rulename, const string &db_path);
  static string updateReasonRuleEffectNum(const string &rulename, const string &db_path, int effectNum, string checkResultMsg);
  static ReasonSparql getCheckSparql(const string &rulename, const string &db_path);
};
