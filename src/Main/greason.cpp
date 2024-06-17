#include "../Util/Util.h"
#include "../Reason/Reason.h"
using namespace std;
int main(int argc, char *argv[])
{
    /* code */
    Util util;
    ReasonHelper reasonhelper;
    // Log.init("slog.properties");
    string _db_home = util.getConfigureValue("db_home");
    string _db_suffix = util.getConfigureValue("db_suffix");
    int _suffix_len = _db_suffix.length();
    // string _db_name = "system";

    string _rdf = Util::system_path;
    long tv_begin = Util::get_cur_time();

    string action = Util::getArgValue(argc, argv, "a", "action");
    string db_namestr = Util::getArgValue(argc, argv, "db", "database");
    cout<<"action:"<<action<<",db:"<<db_namestr<<endl;
    if (db_namestr.empty())
    {
        cout << "You need to input the database name that you want to init. Input \"bin/ginit -h\" for help." << endl;
        return -1;
    }
    else
    {
        string _db_path = _db_home  + db_namestr + _db_suffix;
        cout<<"_db_path:"<<_db_path<<endl;
        if (Util::dir_exist(_db_path) == false)
        {
            cout << "The database [" << db_namestr << "] is not exist,please use bin/gbuild to build the  database at first!" << endl;
            return -1;
        }
    }
    if (action == "save")
    {

        string rulefilepath = Util::getArgValue(argc, argv, "f", "file");
        //    string rulefilepath="../data/reason_test/reason.json";
        std::ifstream ifs(rulefilepath);
        std::string jsonStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        Document doc;
        doc.SetObject();
        doc.Parse(jsonStr.c_str());
        // cout<<jsonStr<<endl;

        if (doc.HasMember("ruleinfo"))
        {

            //  Value
            //  rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            Value ruleInfo = doc["ruleinfo"].GetObject();

            ReasonOperationResult result=reasonhelper.saveReasonRuleInfo(ruleInfo, db_namestr,_db_home,_db_suffix);
            cout<<"operation result:"<<result.error_message<<endl;

            //    reasonhelper.getReasonRuleList(db_namestr);
        }
    }
    else if (action == "show")
    {
        string db_path=_db_home+db_namestr+_db_suffix;
        vector<string> list = reasonhelper.getReasonRuleList(db_path);
        // 创建一个StringBuffer来存储格式化的JSON
        for(int i=0;i<list.size();i++)
        {
            cout<<list[i]<<endl;
        }
    }
    else if(action=="compile")
    {
        string reasonname = Util::getArgValue(argc, argv, "n", "name");
        reasonhelper.compileReasonRule(reasonname,db_namestr,_db_home,_db_suffix);
        reasonhelper.updateReasonRuleStatus(reasonname,db_namestr,"已编译",_db_home,_db_suffix);
    }
    else if(action=="execute")
    {
        string reasonname = Util::getArgValue(argc, argv, "n", "name");
        ReasonSparql result = reasonhelper.executeReasonRule(reasonname, db_namestr,_db_home,_db_suffix);
        if (result.issuccess == 1)
        {
            try
            {
                /* code */
                Database _db(db_namestr);
                _db.load();
                ResultSet ask_rs;
                FILE *ask_ofp = NULL;
                string sparql = result.insert_sparql;
                _db.query(sparql, ask_rs, ask_ofp);
                long rs_ansNum = max((long)ask_rs.ansNum - ask_rs.output_offset, 0L);
                cout << "ans:" << rs_ansNum << endl;
                _db.save();
                reasonhelper.updateReasonRuleStatus(reasonname, db_namestr, "已执行",_db_home,_db_suffix);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
                reasonhelper.updateReasonRuleStatus(reasonname, db_namestr, "已编译",_db_home,_db_suffix);
            }
            catch (...)
            {
                reasonhelper.updateReasonRuleStatus(reasonname, db_namestr, "已编译",_db_home,_db_suffix);
            }
        }
        else
        {
            cout << result.error_message << endl;
        }

    //    ask_rs.prettyPrint();
   
    //   _db.save();
      
    }
    else if(action=="updateStatus")
    {
          string status = Util::getArgValue(argc, argv, "s", "status");
          string reasonname = Util::getArgValue(argc, argv, "n", "name");
          string result=reasonhelper.updateReasonRuleStatus(reasonname,db_namestr,status,_db_home,_db_suffix);
          if(result.empty())
          {
            cout<<"Update Status for Reason File Successfully!"<<endl;
          }
          else{
             cout<<"Update Status for Reason File Failure!"<<result<<endl;
          }
    }
    else if(action=="disable")
    {
         string reasonname = Util::getArgValue(argc, argv, "n", "name");
        ReasonSparql result = reasonhelper.disableReasonRule(reasonname, db_namestr,_db_home,_db_suffix);
        if (result.issuccess == 1)
        {
            try
            {
                /* code */
                Database _db(db_namestr);
                _db.load();
                ResultSet ask_rs;
                FILE *ask_ofp = NULL;
                string sparql = result.delete_sparql;
                _db.query(sparql, ask_rs, ask_ofp);
                long rs_ansNum = max((long)ask_rs.ansNum - ask_rs.output_offset, 0L);
                cout << "ans:" << rs_ansNum << endl;
                _db.save();
                reasonhelper.updateReasonRuleStatus(reasonname, db_namestr, "已失效",_db_home,_db_suffix);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
                reasonhelper.updateReasonRuleStatus(reasonname, db_namestr, "已编译",_db_home,_db_suffix);
            }
            catch (...)
            {
                reasonhelper.updateReasonRuleStatus(reasonname, db_namestr, "已编译",_db_home,_db_suffix);
            }
        }
        else
        {
            cout << result.error_message << endl;
        }
    }
    return 0;
}