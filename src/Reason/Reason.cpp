#include "Reason.h"

using namespace std;

ReasonHelper::ReasonHelper()
{
}

ReasonHelper::~ReasonHelper()
{
}

ReasonOperationResult ReasonHelper::saveReasonRuleInfo(nlohmann::json &ruleInfo, const string &db_path)
{
  // 将字符串写入文件中
  ReasonOperationResult result;
  string rulename = JsonUtil::jsonParam(ruleInfo, "rulename");
  string filepath = db_path + "/reason_rule_files/";
  string rulefilepath = filepath + rulename + ".json";
  // cout<<"rulefilepath:"<<rulefilepath<<endl;
  if (Util::file_exist(rulefilepath))
  {
    result.issuccess = 0;
    result.error_message = "The rule file has been exist, please remove it before!";
    return result;
  }

  if (Util::dir_exist(filepath) == false)
  {
    Util::create_dir(filepath);
  }
  std::ofstream file(rulefilepath);
  file << ruleInfo.dump();
  file.close();
  result.issuccess = 1;
  result.error_message = "Save Successfully! the file path is " + rulefilepath;
  return result;
}

vector<string> ReasonHelper::getReasonRuleList(const string &db_path)
{
  // 将字符串写入文件中

  vector<string> resultlist;

  string filepath = db_path + "/reason_rule_files/";
  if (Util::dir_exist(filepath) == false)
  {
    Util::create_dir(filepath);
  }
  // cout<<"filepath:"<<filepath<<endl;
  vector<string> files = Util::GetFiles(filepath.c_str(), "json");

  int size = files.size();

  for (int i = 0; i < files.size(); i++)
  {
    string rulename = files[i];
    // cout<<"file:"<<rulename<<endl;
    string rulefilepath = filepath + rulename;

    if (Util::file_exist(rulefilepath))
    {
      // cout<<"file:"<<rulefilepath<<endl;
      std::ifstream ifs(rulefilepath);
      if (ifs.is_open() == false)
      {
        SLOG_ERROR("open the file is failed: " + rulefilepath);
        continue;
      }
      std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
      resultlist.push_back(jsonStr);
      ifs.close();
    }
  }
  return resultlist;
}

/// @brief make the rule enable
/// @param ruleinfo
/// @return
ReasonSparql ReasonHelper::compileReasonRule(const string &rulename, const string &db_path)
{
  ReasonSparql results;
  string rulefilepath = db_path + "/reason_rule_files/" + rulename + ".json";
  // string searchsparql = "";
  // string updatesparql = "";
  string insert_sparql = "";
  string delete_sparql = "";
  string check_sparql = "";

  if (Util::file_exist(rulefilepath) == false)
  {
    results.error_message = "the reason file is not exist";
    results.issuccess = 0;
    return results;
  }
  else
  {
    std::ifstream ifs(rulefilepath);
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    if (nlohmann::json::accept(jsonStr) == false)
    {
      results.error_message = "the reason file is not fit the json format";
      results.issuccess = 0;
      return results;
    }
    nlohmann::json doc = nlohmann::json::parse(jsonStr);
    if (doc.contains("conditions") == false)
    {
      results.error_message = "the reason file has not the conditons information";
      results.issuccess = 0;
      return results;
    }
    nlohmann::json conditions = doc["conditions"];
    string wheresparql = "";
    int conditions_length = conditions.size();
    int logic = JsonUtil::jsonParam(doc, "logic", 0);
    for (int i = 0; i < conditions_length; i++)
    {
      string subwhere = "";
      nlohmann::json condition = conditions[i];
      if (condition.contains("patterns"))
      {
        nlohmann::json patterns = condition["patterns"];
        for (int j = 0; j < patterns.size(); j++)
        {
          nlohmann::json pattern = patterns[j];
          string _s, _p, _o;
          pattern["subject"].get_to(_s);
          pattern["predicate"].get_to(_p);
          pattern["object"].get_to(_o);
          subwhere = subwhere + " " + _s + " " + _p + " " + _o + ".";
        }
      }

      string subfilter = "";
      if (condition.contains("filters"))
      {
        nlohmann::json filters = condition["filters"];
        for (int j = 0; j < filters.size(); j++)
        {
          string filter;
          filters[j].get_to(filter);
          if (j > 0)
            subfilter = subfilter + " & " + filter + " ";
          else
            subfilter = subfilter + "  " + filter + " ";
        }
        if (filters.size() > 0)
        {
          subfilter = " filter( " + subfilter + ").";
        }
      }
      subwhere = "{" + subwhere + subfilter + "}";

      if (conditions_length > 1 && i < conditions_length - 1)
      {
        if (logic == 0)
        {
          subwhere = subwhere + " union ";
        }
      }
      wheresparql = wheresparql + subwhere;
    }
    if (doc.contains("return") == false)
    {
      results.error_message = "the reason file has not the return information";
      results.issuccess = 0;
      return results;
    }
    nlohmann::json returnInfo = doc["return"];
    string source = JsonUtil::jsonParam(returnInfo, "source");

    string label = JsonUtil::jsonParam(returnInfo, "label");
    string value = JsonUtil::jsonParam(returnInfo, "value");
    int type = JsonUtil::jsonParam(returnInfo, "type", 0);

    if (type == 0)
    {
      // property
      // searchsparql = "select " + source + " where " + wheresparql;
      // updatesparql = " <?> <Rule:" + target + "> " + value + ". ";
      if (conditions_length > 1)
      {
        insert_sparql = "insert {" + source + " <Rule:" + label + "> " + value + ". } where { " + wheresparql + " }";
      }
      else
      {
        insert_sparql = "insert {" + source + " <Rule:" + label + "> " + value + ". } where " + wheresparql;
      }
      delete_sparql = "delete where {?x <Rule:" + label + "> " + value + ".}";
    }
    else
    {
      // relationship
      // searchsparql = "select " + source + " " + target + " where " + wheresparql;
      // updatesparql = " <?1> <Rule:" + label + "> <?2>.";
      string target = JsonUtil::jsonParam(returnInfo, "target");
      if (conditions_length > 1)
      {
        insert_sparql = "insert { " + source + " <Rule:" + label + "> " + target + ". } where { " + wheresparql + " }";
      }
      else
      {
        insert_sparql = "insert { " + source + " <Rule:" + label + "> " + target + ". } where " + wheresparql;
      }

      delete_sparql = "delete where {?x <Rule:" + label + "> ?y.}";
    }
    check_sparql = "select (count(*) as ?result) where { " + wheresparql + " }";

    doc["status"] = "已编译";
    doc["insert_sparql"] = insert_sparql;
    doc["delete_sparql"] = delete_sparql;
    doc["check_sparql"] = check_sparql;

    results.insert_sparql = insert_sparql;
    results.delete_sparql = delete_sparql;
    results.check_sparql = check_sparql;

    results.issuccess = 1;
    SLOG_CORE("insert_sparql:" << insert_sparql);
    SLOG_CORE("delete_sparql:" << delete_sparql);
    SLOG_CORE(" check_sparql:" << check_sparql);

    std::ofstream file(rulefilepath);
    file << doc.dump();
    file.close();
  }
  return results;
}

ReasonSparql ReasonHelper::executeReasonRule(const string &rulename, const string &db_path)
{
  ReasonSparql results;
  string rulefilepath = db_path + "/reason_rule_files/" + rulename + ".json";
  SLOG_CORE("rulefilepath:" << rulefilepath);

  if (Util::file_exist(rulefilepath) == false)
  {
    results.error_message = "the reason file is not exist";
    results.issuccess = 0;
    return results;
  }
  else
  {
    std::ifstream ifs(rulefilepath);
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    if (nlohmann::json::accept(jsonStr) == false)
    {
      results.error_message = "the reason file is not fit the json format";
      results.issuccess = 0;
      return results;
    }

    string sparql = "";
    nlohmann::json doc = nlohmann::json::parse(jsonStr);
    if (doc.contains("insert_sparql"))
    {
      doc.at("insert_sparql").get_to(sparql);
      SLOG_CORE("start loading the database......");
      results.insert_sparql = sparql;
      results.issuccess = 1;
      return results;
    }
    else
    {
      results.error_message = "The sparql of the reason rule is not exist! ";
      results.issuccess = 0;
      return results;
    }
  }
  return results;
}

string ReasonHelper::updateReasonRuleInfo(nlohmann::json &ruleInfo, const string &db_path)
{
  string result = "";
  string rulename = JsonUtil::jsonParam(ruleInfo, "rulename");
  string filepath = db_path + "/reason_rule_files/";
  string rulefilepath = filepath + rulename + ".json";

  if (Util::dir_exist(rulefilepath) == false)
  {
    Util::create_dir(filepath);
  }
  std::ofstream file(rulefilepath);
  file << ruleInfo.dump();
  file.close();
  return "";
}

string ReasonHelper::updateReasonRuleStatus(const string &rulename, const string &status, const string &db_path)
{
  string result = "";
  string rulefilepath = db_path + "/reason_rule_files/" + rulename + ".json";

  if (Util::dir_exist(db_path) == false)
  {
    result = "the database directory is not exists";
    return result;
  }
  if (Util::file_exist(rulefilepath) == false)
  {
    result = "the reason file is not exists";
    return result;
  }
  std::ifstream ifs(rulefilepath);
  std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  if (nlohmann::json::accept(jsonStr) == false)
  {
    result = "the reason file is not fit the json format";
    return result;
  }
  nlohmann::json doc = nlohmann::json::parse(jsonStr);
  doc["status"] = status;
  std::ofstream file(rulefilepath);
  file << doc.dump();
  file.close();
  return "";
}

string ReasonHelper::updateReasonRuleEffectNum(const string &rulename, const string &db_path, int effectNum, string checkResultMsg)
{
  string result = "";
  string rulefilepath = db_path + "/reason_rule_files/" + rulename + ".json";

  if (Util::dir_exist(db_path) == false)
  {
    result = "the database directory is not exists";
    return result;
  }
  if (Util::file_exist(rulefilepath) == false)
  {
    result = "the reason file is not exists";
    return result;
  }
  std::ifstream ifs(rulefilepath);
  std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  if (nlohmann::json::accept(jsonStr) == false)
  {
    result = "the reason file is not fit the json format";
    return result;
  }
  nlohmann::json doc = nlohmann::json::parse(jsonStr);
  doc["effectNum"] = effectNum;
  doc["checkResult"] = checkResultMsg;
  std::ofstream file(rulefilepath);
  file << doc.dump();
  file.close();
  return "";
}

ReasonSparql ReasonHelper::disableReasonRule(const string &rulename, const string &db_path)
{
  ReasonSparql results;
  string rulefilepath = db_path + "/reason_rule_files/" + rulename + ".json";
  SLOG_CORE("rulefilepath:" << rulefilepath);

  if (Util::file_exist(rulefilepath) == false)
  {
    results.error_message = "the reason file is not exist";
    results.issuccess = 0;
    return results;
  }
  else
  {
    std::ifstream ifs(rulefilepath);
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    if (nlohmann::json::accept(jsonStr) == false)
    {
      results.error_message = "the reason file is not fit the json format";
      results.issuccess = 0;
      return results;
    }
    string sparql = "";
    nlohmann::json doc = nlohmann::json::parse(jsonStr);

    if (doc.contains("delete_sparql"))
    {
      doc.at("delete_sparql").swap(sparql);
      results.delete_sparql = sparql;
      results.issuccess = 1;
      return results;
    }
    else
    {
      results.error_message = "The sparql of the reason rule is not exist! ";
      results.issuccess = 0;
      return results;
    }
  }
  return results;
}

ReasonOperationResult ReasonHelper::getReasonInfo(const string &rulename, const string &db_path)
{
  ReasonOperationResult result;
  string rulefilepath = db_path + "/reason_rule_files/" + rulename + ".json";

  if (Util::dir_exist(db_path) == false)
  {
    result.error_message = "the database directory is not exists";
    result.issuccess = 0;
    return result;
  }
  if (Util::file_exist(rulefilepath) == false)
  {
    result.error_message = "the reason file is not exists";
    result.issuccess = 0;
    return result;
  }
  std::ifstream ifs(rulefilepath);
  std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  result.issuccess = 1;
  result.error_message = jsonStr;
  return result;
}

ReasonOperationResult ReasonHelper::removeReasonRule(const string &rulename, const string &db_path)
{
  ReasonOperationResult result;

  string rulefilepath = db_path + "/reason_rule_files/" + rulename + ".json";

  if (Util::dir_exist(db_path) == false)
  {
    result.error_message = "the database directory is not exists";
    result.issuccess = 0;
    return result;
  }
  if (Util::file_exist(rulefilepath) == false)
  {
    result.error_message = "the reason file is not exists";
    result.issuccess = 0;
    return result;
  }
  Util::remove_file(rulefilepath);
  result.issuccess = 1;
  result.error_message = "the reason file has been remove successfully! file path:" + rulefilepath;
  return result;
}

ReasonSparql ReasonHelper::getCheckSparql(const string &rulename, const string &db_path)
{
  ReasonSparql results;
  string rulefilepath = db_path + "/reason_rule_files/" + rulename + ".json";
  SLOG_CORE("rulefilepath:" << rulefilepath);

  if (Util::file_exist(rulefilepath) == false)
  {
    results.error_message = "the reason file is not exist";
    results.issuccess = 0;
    return results;
  }
  else
  {
    std::ifstream ifs(rulefilepath);
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    if (nlohmann::json::accept(jsonStr) == false)
    {
      results.error_message = "the reason file is not fit the json format";
      results.issuccess = 0;
      return results;
    }

    string sparql = "";
    nlohmann::json doc = nlohmann::json::parse(jsonStr);
    if (doc.contains("check_sparql"))
    {
      doc.at("check_sparql").get_to(sparql);
      SLOG_CORE("start loading the database......");
      results.check_sparql = sparql;
      results.issuccess = 1;
      return results;
    }
    else
    {
      results.error_message = "The sparql of the reason rule is not exist! ";
      results.issuccess = 0;
      return results;
    }
  }
  return results;
}
