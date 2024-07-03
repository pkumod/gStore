/*=============================================================================
# Filename: ResultSet.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-24 22:01
# Description: implement functions in ResultSet.h
=============================================================================*/

#include "ResultSet.h"
#include <regex>

using namespace std;

ResultSet::ResultSet()
{
	this->select_var_num = 0;
	this->var_name = NULL;
	this->ansNum = 0;
	this->answer = NULL;
	this->stream = NULL;
	this->useStream = false;
	this->output_offset = 0;
	this->output_limit = -1;
}

ResultSet::~ResultSet()
{
	release();
}

ResultSet::ResultSet(int _v_num, const string* _v_names)
{
	this->select_var_num = _v_num;
	this->var_name = new string[this->select_var_num];
	for(int i = 0; i < this->select_var_num; i++)
	{
		this->var_name[i] = _v_names[i];
	}
	this->ansNum = 0;
	this->answer = NULL;
	this->stream = NULL;
	this->useStream = false;
	this->output_offset = 0;
	this->output_limit = -1;
}

void
ResultSet::release()
{
	if (this->var_name != nullptr)
	{
  		delete[] this->var_name;
		this->var_name = nullptr;
	}
	if (!this->useStream)
	{
		if (this->answer != nullptr)
		{
			for(unsigned i = 0; i < this->ansNum; i++)
			{
				if (this->answer[i] != nullptr)
				{
					delete[] this->answer[i];
					this->answer[i] = nullptr;
				}	
			}
			delete[] this->answer;
			this->answer = nullptr;
		}
	}
	else if(this->stream)
	{
		delete this->stream;    //maybe NULL
		this->stream = nullptr;
	}
}

void
ResultSet::setUseStream()
{
	this->useStream = true;
}

bool
ResultSet::checkUseStream()
{
	return this->useStream;
}

void 
ResultSet::setOutputOffsetLimit(int _output_offset, int _output_limit)
{
	this->output_offset = _output_offset;
	this->output_limit = _output_limit;
}

void
ResultSet::setVar(const vector<string> & _var_names)
{
	if (this->var_name != nullptr) {
		delete[] this->var_name;
	}
	this->select_var_num = _var_names.size();
	this->var_name = new string[this->select_var_num];
	for(int i = 0; i < this->select_var_num; i++)
	{
		this->var_name[i] = _var_names[i];
	}
}

void
ResultSet::setUsername(const std::string& _username)
{
	this->username = _username;
}
string
ResultSet::getUsername()
{
	return this->username;
}

//convert to TSV string
string 
ResultSet::to_str()
{
	long long ans_num = max((long long)this->ansNum - this->output_offset, 0LL);
	if (this->output_limit != -1)
		ans_num = min(ans_num, (long long)this->output_limit);
	if(ans_num == 0)
	{
		return "[empty result]\n";
	}

	stringstream _buf;

	for(int i = 0; i < this->true_select_var_num; i++)
	{
		if (i != 0)
			_buf << "\t";
		_buf << this->var_name[i];
	}
	_buf << "\n";

	if (this->useStream)
		this->resetStream();

	const Bstr* bp = NULL;
	for(long long i = (!this->useStream ? this->output_offset : 0LL); i < this->ansNum; i++)
	{
		if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
			break;

		if (this->useStream)
			bp = this->stream->read();

		if (i >= this->output_offset)
		{
			for(int j = 0; j < this->true_select_var_num; j++)
			{
				if (j != 0)
					_buf << "\t";
				if (!this->useStream)
					_buf << Util::node2string(this->answer[i][j].c_str());
				else
					_buf << Util::node2string(bp[j].getStr());
			}
			_buf << "\n";
		}
	}

	return _buf.str();
}

//convert to JSON string
/**
string
ResultSet::to_JSON()
{
	stringstream _buf;

	_buf << "{ \"head\": { \"link\": [], \"vars\": [";
	for (int i = 0; i < this->true_select_var_num; i++)
	{
		if (i != 0)
			_buf << ", ";
		_buf << "\"" + this->var_name[i].substr(1) + "\"";
	}
	_buf << "] }, \n";

	_buf << "\t\"results\": \n";
	_buf << "\t{\n";
	_buf << "\t\t\"bindings\": \n";
	_buf << "\t\t[\n";

	if (this->useStream)
		this->resetStream();

	const Bstr* bp = NULL;
	// match ^^<*> string
	regex dataTypePattern("\\^\\^<(\\S*?)[^>]*>.*?|<.*? />");
	smatch matchResult;
	string::const_iterator iterStart;
	string::const_iterator iterEnd;
	for(long long i = (!this->useStream ? this->output_offset : 0LL); i < this->ansNum; i++)
	{
		if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
		{
			  cout<<"the size is out than the output_limit"<<endl;
              break;
		}
			
		if (this->useStream)
		{
            bp = this->stream->read();
		}
			

		if (i >= this->output_offset)
		{
			if (i != this->output_offset)
				_buf << ",\n";

			_buf << "\t\t\t{ ";

			bool list_empty = true;
			for(int j = 0; j < this->true_select_var_num; j++)
			{
				string ans_type, ans_str;

				if (!this->useStream)
					ans_str = this->answer[i][j];
				else
					ans_str = string(bp[j].getStr());

				if (ans_str.length() == 0)
					continue;
                //ans_str=Util::replace_all(ans_str,"\n","");
				if (!list_empty)
					_buf << ",\t";
				if (ans_str[0] == '<')
				{
					ans_type = "uri";
					ans_str = ans_str.substr(1, ans_str.length() - 2);
					_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
					_buf << "\"type\": \"" + ans_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
				    list_empty=false;
				}
				else if (ans_str[0] == '"')
				{
					if (ans_str.find("\"^^<") == string::npos)
					{
						if (ans_str.find("\"@") != string::npos)
						{
							//for language string
							ans_type = "literal";
							string data_type = ans_str.substr(ans_str.rfind("@")+1, ans_str.length());
							ans_str = ans_str.substr(1, ans_str.rfind('"') - 1);
							_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
							_buf << "\"type\": \"" + ans_type + "\", \"lang\": \"" + data_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
							list_empty = false;
						}
						else
						{
							//no has type string
							ans_type = "literal";
							ans_str = ans_str.substr(1, ans_str.rfind('"') - 1);
							_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
							_buf << "\"type\": \"" + ans_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
							list_empty = false;
						}
					}
					else
					{
						string data_type = "";
						if (ans_str[ans_str.length() - 1] == '>')
						{
							ans_type = "typed-literal";
							int pos = ans_str.find("\"^^<");
							// string data_type = ans_str.substr(pos + 4, ans_str.length() - pos - 5);
							iterStart = ans_str.begin();
							iterEnd = ans_str.end();
							while (regex_search(iterStart, iterEnd, matchResult, dataTypePattern))
							{
								data_type = matchResult[0];
								break;
							}
							if (data_type.length() > 4)
							{
								// remove ^^<>
								data_type = data_type.substr(3, data_type.length()-4);
								if (data_type.find("^^<") != string::npos)
								{
									data_type = "http://www.w3.org/2001/XMLSchema#string-complete";
								}
							}
							else
							{
								data_type = "http://www.w3.org/2001/XMLSchema#string-complete";
							}
							ans_str = ans_str.substr(0, pos+1);
							_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
							_buf << "\"type\": \"" + ans_type + "\", \"datatype\": \"" + data_type + "\", \"value\": " + Util::node2string(ans_str.c_str()) + " }";
							list_empty = false;
						}
						else
						{
							// the entity value is not complete
							ans_type = "typed-literal";
							int pos = ans_str.find("\"^^<");
							data_type = "http://www.w3.org/2001/XMLSchema#string-not-complete";
							ans_str = ans_str.substr(0, pos+1);
							_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
							_buf << "\"type\": \"" + ans_type + "\", \"datatype\": \"" + data_type + "\", \"value\": " + Util::node2string(ans_str.c_str()) + " }";
							list_empty = false;
						}
					}
				}
				else if (ans_str[0] == '\'') 
				{
					if (ans_str.find("'^^<") == string::npos)
					{
						if (ans_str.find("'@") != string::npos)
						{
							//for language string
							ans_type = "literal";
							string data_type = ans_str.substr(ans_str.rfind("@")+1, ans_str.length());
							ans_str = ans_str.substr(1, ans_str.rfind('\'') - 1);
							_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
							_buf << "\"type\": \"" + ans_type + "\", \"lang\": \"" + data_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
							list_empty = false;
						}
						else
						{
							//no has type string
							ans_type = "literal";
							ans_str = ans_str.substr(1, ans_str.rfind('\'') - 1);
							_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
							_buf << "\"type\": \"" + ans_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
							list_empty = false;
						}
					}
					else
					{
						_buf<<"\"error\":{\"errorMsg:\":\"the information is not complete!\"}";
						list_empty=false;
					}
				}
				// else if (ans_str[0] == '"' && ans_str.find("\"^^<") == string::npos && ans_str[ans_str.length() - 1] != '>' )
				// {
				// 	ans_type = "literal";
				// 	ans_str = ans_str.substr(1, ans_str.rfind('"') - 1);
				// 	_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
				// 	_buf << "\"type\": \"" + ans_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
				//     list_empty=false;
				// }
				// else if (ans_str[0] == '"' && ans_str.find("\"^^<") != string::npos && ans_str[ans_str.length() - 1] == '>' )
				// {
				// 	ans_type = "typed-literal";
				// 	int pos = ans_str.find("\"^^<");
				// 	string data_type = ans_str.substr(pos + 4, ans_str.length() - pos - 5);
				// 	ans_str = ans_str.substr(1, pos - 1);
				// 	_buf << "\"" + this->var_name[j].substr(1) + "\": { ";
				// 	_buf << "\"type\": \"" + ans_type + "\", \"datatype\": \"" + data_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
                //     list_empty=false;
				// }
				
				else{
					//string ans_str_new=Util::replace_all(ans_str,"\"","“");
					_buf<<"\"error\":{\"errorMsg:\":\"the information is not complete!\"}";
					list_empty=false;
				}
				//list_empty = false;
			}
			_buf << "}";
		}
	}

	_buf << "\n\t\t]\n";
	_buf << "\t}\n";
	_buf << "}\n";

	return _buf.str();
}
**/

string
ResultSet::to_JSON()
{
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
	#if defined(DEBUG)
	Util::create_dirs("logs/result_set");
	ofstream outfile;
	string log_file_name = "logs/result_set/" + Util::getTimeString2() + "_" + Util::int2string(Util::getRandNum()) + ".txt";
	outfile.open(log_file_name);
	#endif
	rapidjson::Value head(rapidjson::kObjectType);
	rapidjson::Value link(rapidjson::kArrayType);
	rapidjson::Value vars(rapidjson::kArrayType);
	rapidjson::Value str_value(rapidjson::kStringType);
	head.AddMember("link", link, allocator);
	for (int i = 0; i < this->true_select_var_num; i++)
	{
		string var = this->var_name[i].substr(1);
		str_value.SetString(var.c_str(), var.size(), allocator);
		vars.PushBack(str_value.Move(), allocator);
	}
	head.AddMember("vars", vars, allocator);
	rapidjson::Value results(rapidjson::kObjectType);
	rapidjson::Value buildings(rapidjson::kArrayType);

	if (this->useStream)
		this->resetStream();

	const Bstr* bp = NULL;
	// match ^^<*> string
	regex dataTypePattern("\\^\\^<(\\S*?)[^>]*>.*?|<.*? />");
	smatch matchResult;
	string::const_iterator iterStart;
	string::const_iterator iterEnd;
	for(long long i = (!this->useStream ? this->output_offset : 0LL); i < this->ansNum; i++)
	{
		if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
		{
			  SLOG_CODE("the size is out than the output_limit");
              break;
		}
		if (this->useStream)
		{
            bp = this->stream->read();
		}
		if (i >= this->output_offset)
		{
			rapidjson::Value json_var(rapidjson::kObjectType);
			for(int j = 0; j < this->true_select_var_num; j++)
			{
				string ans_str;

				if (!this->useStream)
					ans_str = this->answer[i][j];
				else
					ans_str = string(bp[j].getStr());

				if (ans_str.length() == 0)
					continue;
				string ans_key = "", ans_type = "", data_type = "", lang = "";
                #if defined(DEBUG)
				outfile << ans_str + "\n";
				#endif
				ans_key = this->var_name[j].substr(1);
				if (ans_str[0] == '<')
				{
					ans_type = "uri";
					ans_str = Util::clear_angle_brackets(ans_str);
				}
				else if (ans_str[0] == '"')
				{
					if (ans_str.find("\"^^<") == string::npos)
					{
						if (ans_str.find("\"@") != string::npos)
						{
							//for language string
							ans_type = "literal";
							lang = ans_str.substr(ans_str.rfind("@")+1, ans_str.length());
							ans_str = ans_str.substr(1, ans_str.rfind('"') - 1);
						}
						else
						{
							//no has type string
							ans_type = "literal";
							ans_str = ans_str.substr(1, ans_str.rfind('"') - 1);
						}
					}
					else
					{
						
						if (ans_str[ans_str.length() - 1] == '>')
						{
							ans_type = "typed-literal";
							int pos = ans_str.find("\"^^<");
							// string data_type = ans_str.substr(pos + 4, ans_str.length() - pos - 5);
							iterStart = ans_str.begin();
							iterEnd = ans_str.end();
							while (regex_search(iterStart, iterEnd, matchResult, dataTypePattern))
							{
								data_type = matchResult[0];
								break;
							}
							if (data_type.length() > 4)
							{
								// remove ^^<>
								data_type = data_type.substr(3, data_type.length()-4);
								if (data_type.find("^^<") != string::npos)
								{
									data_type = "http://www.w3.org/2001/XMLSchema#string-complete";
								}
							}
							else
							{
								data_type = "http://www.w3.org/2001/XMLSchema#string-complete";
							}
							ans_str = ans_str.substr(1, pos-1);
						}
						else
						{
							// the entity value is not complete
							ans_type = "typed-literal";
							int pos = ans_str.find("\"^^<");
							data_type = "http://www.w3.org/2001/XMLSchema#string-not-complete";
							ans_str = ans_str.substr(1, pos-1);
						}
					}
				}
				else if (ans_str[0] == '\'') 
				{
					if (ans_str.find("'^^<") == string::npos)
					{
						if (ans_str.find("'@") != string::npos)
						{
							//for language string
							ans_type = "literal";
							lang = ans_str.substr(ans_str.rfind("@")+1, ans_str.length());
							ans_str = ans_str.substr(1, ans_str.rfind('\'') - 1);
						}
						else
						{
							//no has type string
							ans_type = "literal";
							ans_str = ans_str.substr(1, ans_str.rfind('\'') - 1);
						}
					}
					else
					{
						ans_type = "error";
					}
				}
				else
				{
					ans_type = "error";
				}
				rapidjson::Value json_item(rapidjson::kObjectType);

				str_value.SetString(ans_type.c_str(), ans_type.size(), allocator);
				json_item.AddMember("type", str_value.Move(), allocator);
				
				if (data_type != "")
				{
					str_value.SetString(data_type.c_str(), data_type.size(), allocator);
					json_item.AddMember("datatype", str_value.Move(), allocator);
				}

				str_value.SetString(ans_str.c_str(), ans_str.size(), allocator);
				json_item.AddMember("value", str_value.Move(), allocator);

				if (lang != "")
				{
					str_value.SetString(lang.c_str(), lang.size(), allocator);
					json_item.AddMember("lang", str_value.Move(), allocator);
				}
				
				json_var.AddMember(rapidjson::Value().SetString(ans_key.c_str(), ans_key.size(), allocator).Move(), json_item, allocator);
			}
			buildings.PushBack(json_var, allocator);
		}
	}

	results.AddMember("bindings", buildings, allocator);

	doc.AddMember("head", head, allocator);
	doc.AddMember("results", results, allocator);

	#if defined(DEBUG)
	rapidjson::StringBuffer prettyBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(prettyBuffer);
	doc.Accept(prettyWriter);
	outfile << prettyBuffer.GetString();
	outfile.close();
	#endif

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	doc.Accept(writer);

	return s.GetString();
}

void
ResultSet::output(FILE* _fp)
{
	if (this->useStream)
	{
		long long ans_num = max((long long)this->ansNum - this->output_offset, 0LL);
		if (this->output_limit != -1)
			ans_num = min(ans_num, (long long)this->output_limit);
		if(ans_num == 0)
		{
			fprintf(_fp, "[empty result]\n");
			return;
		}

		fprintf(_fp, "%s", this->var_name[0].c_str());
		for(int i = 1; i < this->true_select_var_num; i++)
		{
			fprintf(_fp, "\t%s", this->var_name[i].c_str());
		}
		fprintf(_fp, "\n");

		const Bstr* bp;
		for(long long i = 0; i < this->ansNum; i++)
		{
			if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
				break;

			bp = this->stream->read();
			if (i >= this->output_offset)
			{
				fprintf(_fp, "%s", Util::node2string(bp[0].getStr()).c_str());
				for(int j = 1; j < this->true_select_var_num; j++)
				{
					fprintf(_fp, "\t%s", Util::node2string(bp[j].getStr()).c_str());
				}
				fprintf(_fp, "\n");
			}
		}
	}
	else
	{
		long long ans_num = max((long long)this->ansNum - this->output_offset, 0LL);
		if (this->output_limit != -1)
			ans_num = min(ans_num, (long long)this->output_limit);
		if(ans_num == 0)
		{
			fprintf(_fp, "[empty result]\n");
			return;
		}

		fprintf(_fp, "%s", this->var_name[0].c_str());
		for(int i = 1; i < this->true_select_var_num; i++)
		{
			fprintf(_fp, "\t%s", this->var_name[i].c_str());
		}
		fprintf(_fp, "\n");
		
		for(long long i = (!this->useStream ? this->output_offset : 0LL); i < this->ansNum; i++)
		{
			if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
				break;

			if (i >= this->output_offset)
			{
				fprintf(_fp, "%s", Util::node2string(this->answer[i][0].c_str()).c_str());
				for(int j = 1; j < this->true_select_var_num; j++)
				{
					fprintf(_fp, "\t%s", Util::node2string(this->answer[i][j].c_str()).c_str());
				}
				// fprintf(_fp, ".\n");
				fprintf(_fp, "\n");
			}
		}
	}
}

void
ResultSet::prettyPrint()
{
	long long ans_num = max((long long)this->ansNum - this->output_offset, 0LL);
	if (this->output_limit != -1)
		ans_num = min(ans_num, (long long)this->output_limit);
	std::vector<std::string> headers;
	std::vector<std::vector<std::string>> rows;
	for(int i = 0; i < this->true_select_var_num; i++)
	{
		headers.emplace_back(this->var_name[i]);
	}
	if(ans_num == 0)
	{
		Util::printConsole(headers, rows);
		return;
	}
	if (this->useStream)
	{
		const Bstr* bp;
		for(long long i = 0; i < this->ansNum; i++)
		{
			if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
				break;

			bp = this->stream->read();
			if (i >= this->output_offset)
			{
				std::vector<std::string> row;
				for(int j = 0; j < this->true_select_var_num; j++)
				{
					row.emplace_back(Util::node2string(bp[j].getStr()));
				}
				rows.emplace_back(row);
			}
		}
		bp = NULL;
	}
	else
	{
		for(long long i = this->output_offset; i < this->ansNum; i++)
		{
			if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
				break;

			if (i >= this->output_offset)
			{
				std::vector<std::string> row;
				for(int j = 0; j < this->true_select_var_num; j++)
				{
					row.emplace_back(Util::node2string(this->answer[i][j].c_str()));
				}
				rows.emplace_back(row);
			}
		}
	}
	Util::printConsole(headers, rows);
}

void
ResultSet::openStream(vector<unsigned> &_keys, vector<bool> &_desc)
{
	if (this->useStream)
	{
#ifdef DEBUG_STREAM
		vector<int> debug_keys;
		vector<bool> debug_desc;
		for(int i = 0; i < this->select_var_num; ++i)
		{
			debug_keys.push_back(i);
			debug_desc.push_back(false);
		}
#endif
		if(this->stream != NULL)
		{
			delete this->stream;
			this->stream = NULL;
		}
#ifdef DEBUG_STREAM
		if(this->ansNum > 0)
			this->stream = new Stream(debug_keys, debug_desc, this->ansNum, this->select_var_num, true);
#else
		if(this->ansNum > 0)
			this->stream = new Stream(_keys, _desc, this->ansNum, this->select_var_num, _keys.size() > 0);
#endif  //DEBUG_STREAM
	}
}

void
ResultSet::resetStream()
{
	if (this->useStream)
	{
		//this->stream.reset();
		if(this->stream != NULL)
			this->stream->setEnd();
	}
}

void
ResultSet::writeToStream(string& _s)
{
	if (this->useStream)
	{
		if(this->stream != NULL)
			this->stream->write(_s.c_str(), _s.length());
	}
}

const Bstr*
ResultSet::getOneRecord()
{
	if (this->useStream)
	{
		if(this->stream == NULL)
		{
			fprintf(stderr, "ResultSet::getOneRecord(): no results now!\n");
			return NULL;
		}
		if(this->stream->isEnd())
		{
			fprintf(stderr, "ResultSet::getOneRecord(): read till end now!\n");
			return NULL;
		}
		//NOTICE:this is one record, and donot free the memory!
		//NOTICE:Bstr[] but only one element, used as Bstr*
		return this->stream->read();
	}
	else
	{
		return NULL;
	}
}

TempResult
ResultSet::to_tempresult()
{
	TempResult tmp;
    long long ans_num = max((long long)this->ansNum - this->output_offset, 0LL);
	if (this->output_limit != -1)
		ans_num = min(ans_num, (long long)this->output_limit);

	for(int i = 0; i < this->true_select_var_num; i++)
	{
        tmp.str_varset.addVar(this->var_name[i]);
        //cout<<"##VAR##"<<this->var_name[i]<<endl;
	}

	if (this->useStream)
		this->resetStream();

	const Bstr* bp = NULL;
	for(long long i = (!this->useStream ? this->output_offset : 0LL); i < this->ansNum; i++)
	{
		if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
			break;

		if (this->useStream)
			bp = this->stream->read();

		if (i >= this->output_offset)
		{
            //cout<<"##VALUE##";
            tmp.result.push_back(TempResult::ResultPair());
            TempResult::ResultPair& rp=*(tmp.result.end()-1);
			for(int j = 0; j < this->true_select_var_num; j++)
			{
				if (!this->useStream){
					rp.str.push_back(Util::node2string(this->answer[i][j].c_str()));
                    //cout<<","<<this->answer[i][j].c_str();
				}else{
					rp.str.push_back(Util::node2string(bp[j].getStr()));
                    //cout<<";"<<bp[j].getStr();
                }
			}
            //cout<<endl;
		}
	}

	return tmp;
}