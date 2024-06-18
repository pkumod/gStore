#pragma once
#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;
using namespace rapidjson;
struct ReasonSparql
{
  /* data */
  string insert_sparql;
  string delete_sparql;
  string error_message;
  int issuccess=0;
};
struct ReasonOperationResult
{
  int issuccess=0;
  string error_message;
};

class ReasonHelper
{
private:
    /* data */
public:
    ReasonHelper(/* args */);
    ~ReasonHelper();
    static void addReasonRule(Document rule);
    static vector<string> getReasonRuleList(string db_path);
    static void alterReasonRule(Document rule);
    static Document getReasonRuleById(string id);
    static ReasonOperationResult getReasonInfo(string name,string db_name,string db_home,string db_suffix);
    static ReasonOperationResult removeReasonRule(string rulename,string db_name,string db_home,string db_suffix);
    static string getSparqlForReasonRule(Document rule);
    static Document stringToRuleInfo(Value doc);
    static string RuleInfoToString(Document ruleinfo);
    static ReasonOperationResult saveReasonRuleInfo(Value& ruleInfo,string db_name,string db_home,string db_suffix);
    static ReasonSparql compileReasonRule(string rulename,string db_name,string db_home,string db_suffix);
    static ReasonSparql executeReasonRule(string rulename,string db_name,string db_home,string db_suffix);
    static string updateReasonRuleInfo(string rulename,string db_name,Document ruleinfo,string db_home,string db_suffix);
    static string updateReasonRuleStatus(string rulename,string db_name,string status,string db_home,string db_suffix);
    static ReasonSparql disableReasonRule(string rulename,string db_name,string db_home,string db_suffix);
};


