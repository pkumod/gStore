#include "Reason.h"
using namespace rapidjson;
using namespace std;



 Util util;


ReasonHelper::ReasonHelper(/* args */)
{
   
}

ReasonHelper::~ReasonHelper()
{
}

ReasonOperationResult ReasonHelper::saveReasonRuleInfo(Value& ruleInfo,string db_name,string db_home,string db_suffix)
{
    // 将字符串写入文件中
    ReasonOperationResult result;
    string rulename = ruleInfo["rulename"].GetString();
    string filepath = db_home + db_name + db_suffix + "/reason_rule_files/";
    string rulefilepath = filepath + rulename + ".json";
    // cout<<"rulefilepath:"<<rulefilepath<<endl;
    if (Util::file_exist(rulefilepath))
    {
      // cout<< "the rule name is duplicated"<<endl;
      // Util::remove_file(rulefilepath);
      result.issuccess = 0;
      result.error_message = "The rule file has been exist, please remove it before!";
      return result;
    }

    // cout<<"rulename:"<<rulename<<endl;

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    ruleInfo.Accept(writer);

    // cout<<"filepath:"<<filepath<<endl;
    if (Util::dir_exist(filepath) == false)
    {
      Util::create_dir(filepath);
    }

    std::ofstream file(rulefilepath);
    file << buffer.GetString();
    file.close();
    result.issuccess = 1;
    result.error_message = "Save Successfully! the file path is " + rulefilepath;
    // cout<<"success:"<<result.error_message<<endl;
    return result;
}





vector<string> ReasonHelper::getReasonRuleList(string db_path)
{
    // 将字符串写入文件中
   
    vector<string> resultlist;
    
   
    string filepath =db_path+"/reason_rule_files/";
    if (Util::dir_exist(filepath) == false)
    {
        Util::create_dir(filepath);
    }
    // cout<<"filepath:"<<filepath<<endl;
    vector<string> files=Util::GetFiles(filepath.c_str(),"json");
  
    int size=files.size();
   
    Value list(kArrayType);
    for(int i=0;i<files.size();i++)
    {
        string rulename=files[i];
        // cout<<"file:"<<rulename<<endl;
        string rulefilepath = filepath + rulename;

        if(Util::file_exist(rulefilepath))
        {
            // cout<<"file:"<<rulefilepath<<endl;
            std::ifstream ifs(rulefilepath);
             if(ifs.is_open()==false)
             {
                cout<<"open the file is failed"<<endl;
             }
            //std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
          
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
ReasonSparql ReasonHelper::compileReasonRule(string rulename, string db_name,string db_home,string db_suffix)
{
  ReasonSparql results;
  string _db_path = db_home + db_name + db_suffix;
  string rulefilepath = _db_path + "/reason_rule_files/" + rulename + ".json";
  // string searchsparql = "";
  // string updatesparql = "";
  string insert_sparql="";
  string delete_sparql="";

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
    Document doc;
    doc.SetObject();
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
      results.error_message = "the reason file is not fit the json format";
      results.issuccess = 0;
      return results;
    }
    if (doc.HasMember("conditions") == false)
    {
      results.error_message = "the reason file has not the conditons information";
      results.issuccess = 0;
      return results;
    }
    Value conditions = doc["conditions"].GetArray();
    string wheresparql = "";
    int conditions_length = conditions.Size();
    for (int i = 0; i < conditions.Size(); i++)
    {
      string subwhere = "";
      Value condition = conditions[i].GetObject();
      if (condition.HasMember("condition") == false)
      {
        continue;
      }
      Value conditioninfos = condition["condition"].GetObject();
      int logic = condition["logic"].GetInt();
      if (conditioninfos.HasMember("patterns"))
      {
        Value patterns = conditioninfos["patterns"].GetArray();
        for (int j = 0; j < patterns.Size(); j++)
        {
          Value pattern = patterns[j].GetObject();
          subwhere = subwhere + " " + pattern["subject"].GetString() + " " + pattern["predicate"].GetString() + " " + pattern["object"].GetString() + ".";
        }
        if (patterns.Size() > 0)
        {
          subwhere = "{ " + subwhere + " }";
        }
      }

      string subfilter = "";
      if (conditioninfos.HasMember("filters"))
      {
        Value filters = conditioninfos["filters"].GetArray();
        for (int j = 0; j < filters.Size(); j++)
        {
          string filter = filters[j].GetString();
          if (j > 0)
            subfilter = subfilter + " & " + filter + " ";
          else
            subfilter = subfilter + "  " + filter + " ";
        }
        if (filters.Size() > 0)
        {
          subfilter = " filter( " + subfilter + ")";
        }
      }

      if (conditions_length > 1)
      {
        if (logic == 1)
        {
          subwhere = subwhere + " union ";
        }
      }
      wheresparql = wheresparql + subwhere;
    }
    if(doc.HasMember("return")==false)
    {
       results.error_message = "the reason file has not the return information";
      results.issuccess = 0;
      return results;
    }
    Value returnInfo = doc["return"].GetObject();
    string source = returnInfo["source"].GetString();
 
    string label = "";
    string value = "";
    if (returnInfo.HasMember("label"))
    {
      label = returnInfo["label"].GetString();
    }
    if (returnInfo.HasMember("value"))
    {
      value = returnInfo["value"].GetString();
    }
    int type = doc["type"].GetInt();
  
    if (type == 0)
    {
      // property
      // searchsparql = "select " + source + " where " + wheresparql;
      // updatesparql = " <?> <Rule:" + target + "> " + value + ". ";
      insert_sparql="insert {"+source+" <Rule:" + label + "> " + value + ". } where "+wheresparql;
      delete_sparql="delete where {?x <Rule:" + label + "> " + value +".}";
    }
    else
    {
      // relationship
      // searchsparql = "select " + source + " " + target + " where " + wheresparql;
      // updatesparql = " <?1> <Rule:" + label + "> <?2>.";
      string target = Util::getStringFromJSON(doc,"target");
      insert_sparql="insert { "+source+" <Rule:" + label + "> "+target+". } where "+wheresparql;
      delete_sparql="delete where {?x <Rule:" + label + "> ?y.}";
    }
    Document::AllocatorType &allocator = doc.GetAllocator();
   
    doc.SetObject();
    doc.Parse(jsonStr.c_str());
    // doc.AddMember("conditions",conditions,allocator);
    // doc.AddMember("return",returnInfo,allocator);
    doc["status"].SetString("已编译",allocator);
    if(doc.HasMember("insert_sparql"))
    {
       doc["insert_sparql"].SetString(StringRef(insert_sparql.c_str()),allocator);
    }
    else
    {
         doc.AddMember("insert_sparql", StringRef(insert_sparql.c_str()), allocator);
    }
    if(doc.HasMember("delete_sparql"))
    {
      doc["delete_sparql"].SetString(StringRef(delete_sparql.c_str()),allocator);
    }
    else{
      doc.AddMember("delete_sparql",StringRef(delete_sparql.c_str()),allocator);
    }
   
    //doc.AddMember("updatesparql", StringRef(updatesparql.c_str()), allocator);
    results.insert_sparql = insert_sparql;
    results.delete_sparql=delete_sparql;

    results.issuccess = 1;
    cout << "insert_sparql:" << insert_sparql << endl;
    cout << "delete_sparql:" << delete_sparql << endl;
    
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::ofstream file(rulefilepath);
    file << buffer.GetString();
    file.close();
  }
  return results;
}

ReasonSparql ReasonHelper::executeReasonRule(string rulename, string db_name,string db_home,string db_suffix)
{
  ReasonSparql results;
  string _db_path = db_home + db_name + db_suffix;
  string rulefilepath = _db_path + "/reason_rule_files/" + rulename + ".json";
  cout<<"rulefilepath:"<<rulefilepath<<endl;
  string searchsparql = "";
  string updatesparql = "";

  if (Util::file_exist(rulefilepath)==false)
  {
    results.error_message = "the reason file is not exist";
    results.issuccess=0;
    return results;
  }
  else
  {
    std::ifstream ifs(rulefilepath);
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    Document doc;
    doc.SetObject();
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
      results.error_message = "the reason file is not fit the json format";
      results.issuccess=0;
      return results;
    }
    
    string sparql = "";

    if (doc.HasMember("insert_sparql"))
    {
      sparql = doc["insert_sparql"].GetString();
      cout << "start loading the database......" << endl;
      results.insert_sparql=sparql;
      results.issuccess=1;
      return results;
      // ResultSet ask_rs;
      // FILE *ask_ofp = stdout;
      // _db.query(sparql, ask_rs, ask_ofp);
    }
    else
    {
      results.error_message = "The  sparql  of the reason rule is not exist! ";
      results.issuccess=0;
      return results;
    }
  }
  return results;
}

string ReasonHelper::updateReasonRuleInfo(string rulename,string db_name,Document ruleinfo,string db_home,string db_suffix)
{
   string result="";
  string filepath="./" + db_name + ".db/reason_rule_files/";
   string rulefilepath=filepath+rulename+".json";
   
    StringBuffer buffer;
    if (Util::dir_exist(rulefilepath) == false)
    {
        Util::create_dir(filepath);
    }
       Writer<StringBuffer> writer(buffer);
       ruleinfo.Accept(writer);
       std::ofstream file(rulefilepath);
       file << buffer.GetString();
       file.close();
   return "";
}

string ReasonHelper::updateReasonRuleStatus(string rulename,string db_name,string status,string db_home,string db_suffix)
{
   string result="";
  string _db_path = db_home + db_name + db_suffix;
  string rulefilepath2 = _db_path + "/reason_rule_files/" + rulename + ".json";
   
    StringBuffer buffer;
    if(Util::dir_exist(_db_path)==false)
    {
      result="the database directory is not exists";
      return result;
    }
    if (Util::file_exist(rulefilepath2) == false)
    {
       result="the reason file is not exists";
      return result;
    }
    std::ifstream ifs(rulefilepath2);
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    Document doc;
    doc.SetObject();
    doc.Parse(jsonStr.c_str());
    ifs.close();
    Document::AllocatorType &allocator = doc.GetAllocator();
    if (doc.HasMember("status"))
    {
       doc["status"].SetString(status.c_str(),allocator);
      
    }
    else{
      doc.AddMember("status",StringRef(status.c_str()),allocator);
    }
     Writer<StringBuffer> writer(buffer);
     doc.Accept(writer);
     std::ofstream file(rulefilepath2);
       file << buffer.GetString();
       file.close();
    return "";
}

ReasonSparql ReasonHelper::disableReasonRule(string rulename,string db_name,string db_home,string db_suffix)
{
  ReasonSparql results;
  string _db_path = db_home + db_name + db_suffix;
  string rulefilepath = _db_path + "/reason_rule_files/" + rulename + ".json";
  cout<<"rulefilepath:"<<rulefilepath<<endl;
  string searchsparql = "";
  string updatesparql = "";

  if (Util::file_exist(rulefilepath)==false)
  {
    results.error_message = "the reason file is not exist";
    results.issuccess=0;
    return results;
  }
  else
  {
    std::ifstream ifs(rulefilepath);
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    Document doc;
    doc.SetObject();
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
      results.error_message = "the reason file is not fit the json format";
      results.issuccess=0;
      return results;
    }
    
    string sparql = "";

    if (doc.HasMember("delete_sparql"))
    {
      sparql = doc["delete_sparql"].GetString();
      results.delete_sparql=sparql;
      results.issuccess=1;
      // cout << "start loading the database......" << endl;
      return results;
      // ResultSet ask_rs;
      // FILE *ask_ofp = stdout;
      // _db.query(sparql, ask_rs, ask_ofp);
    }
    else
    {
      results.error_message = "The  sparql  of the reason rule is not exist! ";
      results.issuccess=0;
      return results;
    }
  }
  return results;
}

ReasonOperationResult ReasonHelper::getReasonInfo(string rulename,string db_name,string db_home,string db_suffix)
{
  ReasonOperationResult result;
  
  string _db_path = db_home + db_name + db_suffix;
  string rulefilepath2 = _db_path + "/reason_rule_files/" + rulename + ".json";
   
    StringBuffer buffer;
    if(Util::dir_exist(_db_path)==false)
    {
      result.error_message="the database directory is not exists";
      result.issuccess=0;
      return result;
    }
    if (Util::file_exist(rulefilepath2) == false)
    {
       result.error_message="the reason file is not exists";
       result.issuccess=0;
      return result;
    }
    std::ifstream ifs(rulefilepath2);
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    result.issuccess=1;
    result.error_message=jsonStr;
    return result;
}

ReasonOperationResult ReasonHelper::removeReasonRule(string rulename,string db_name,string db_home,string db_suffix)
{
  ReasonOperationResult result;
  
  string _db_path = db_home + db_name + db_suffix;
  string rulefilepath2 = _db_path + "/reason_rule_files/" + rulename + ".json";
   
    StringBuffer buffer;
    if(Util::dir_exist(_db_path)==false)
    {
      result.error_message="the database directory is not exists";
      result.issuccess=0;
      return result;
    }
    if (Util::file_exist(rulefilepath2) == false)
    {
       result.error_message="the reason file is not exists";
       result.issuccess=0;
      return result;
    }
    Util::remove_file(rulefilepath2);
    result.issuccess=1;
    result.error_message="the reason file has been remove successfully! file path:"+rulefilepath2;
    return result;
}



