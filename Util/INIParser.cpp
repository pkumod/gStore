#include "INIParser.h"

//remove all blank space
string & TrimString(string & str)
{
        string::size_type pos = 0;
        while (str.npos != (pos = str.find(" ")))   
            str = str.replace(pos, pos + 1, "");
        return str;
}
//read in INI file and parse it
int INIParser::ReadINI(string path)
{
        ifstream in_conf_file(path.c_str());  
        if (!in_conf_file) return 0;   
        string str_line = "";   
        string str_root = "";  
        vector<ININode> vec_ini;   
        while (getline(in_conf_file, str_line))           
        {           
                string::size_type left_pos = 0;            
                string::size_type right_pos = 0;        
                string::size_type equal_div_pos = 0;          
                string str_key = "";            
                string str_value = "";           
                if ((str_line.npos != (left_pos = str_line.find("["))) && (str_line.npos != (right_pos = str_line.find("]"))))                 
                {                   
                        //cout << str_line.substr(left_pos+1, right_pos-1) << endl;                      
                        str_root = str_line.substr(left_pos + 1, right_pos - 1);                  
                }                                     
                if (str_line.npos != (equal_div_pos = str_line.find("=")))                  
                {                    
                        str_key = str_line.substr(0, equal_div_pos);                  
                        str_value = str_line.substr(equal_div_pos + 1, str_line.size() - 1);                  
                        str_key = TrimString(str_key);                 
                        str_value = TrimString(str_value);                    
                        //cout << str_key << "=" << str_value << endl;                       
                }                        
                if ((!str_root.empty()) && (!str_key.empty()) && (!str_value.empty()))                    
                {                   
                        ININode ini_node(str_root, str_key, str_value);                  
                        vec_ini.push_back(ini_node);                  
                        //cout << vec_ini.size() << endl;                       
                }            
        }
        in_conf_file.close();   
        in_conf_file.clear();     
        //vector convert to map       
        map<string, string> map_tmp; 
        for (vector<ININode>::iterator itr = vec_ini.begin(); itr != vec_ini.end(); ++itr)           
        {            
                map_tmp.insert(pair<string, string>(itr->root, ""));           
        }      
        SubNode sn;
        for (map<string, string>::iterator itr = map_tmp.begin(); itr != map_tmp.end(); ++itr)         
        {            
                //cout << itr->first << endl;           
                for (vector<ININode>::iterator sub_itr = vec_ini.begin(); sub_itr != vec_ini.end(); ++sub_itr)                
                {                  
                        if (sub_itr->root == itr->first)                           
                        {                       
                                //cout << sub_itr->key << "=" << sub_itr->value << endl;                           
                                sn.InsertElement(sub_itr->key, sub_itr->value);                          
                        }                 
                }            
                map_ini.insert(pair<string, SubNode>(itr->first, sn));        
        }
         return 1;
   
}

//get value by root and key

string INIParser::GetValue(string root, string key)
{
        map<string, SubNode>::iterator itr = map_ini.find(root);
        map<string, string>::iterator sub_itr = itr->second.sub_node.find(key);
        if (sub_itr != itr->second.sub_node.end())
            return sub_itr->second;  
        return "";
}



//write ini file

int INIParser::WriteINI(string path)
{
        ofstream out_conf_file(path.c_str());
        if (!out_conf_file)          
            return -1;      
        //cout << map_ini.size() << endl;    
        for (map<string, SubNode>::iterator itr = map_ini.begin(); itr != map_ini.end(); ++itr)         
        {          
                //cout << itr->first << endl;                
                out_conf_file << "[" << itr->first << "]" << endl;         
                for (map<string, string>::iterator sub_itr = itr->second.sub_node.begin(); sub_itr != itr->second.sub_node.end(); ++sub_itr)               
                {                 
                        //cout << sub_itr->first << "=" << sub_itr->second << endl;                      
                        out_conf_file << sub_itr->first << "=" << sub_itr->second << endl;                    
                }          
        }       
       out_conf_file.close();  
        out_conf_file.clear();
       return 1;
}


//set value

vector<ININode>::size_type INIParser::SetValue(string root, string key, string value)
{
        map<string, SubNode>::iterator itr = map_ini.find(root);
        if (map_ini.end() != itr)  
            itr->second.sub_node.insert(pair<string, string>(key, value));
        else     
        {            
                SubNode sn;
                sn.InsertElement(key, value);      
                map_ini.insert(pair<string, SubNode>(root, sn));
        }
        return map_ini.size();
}
