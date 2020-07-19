/*=============================================================================
# Filename: ResultSet.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-24 22:01
# Description: implement functions in ResultSet.h
=============================================================================*/

#include "ResultSet.h"

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
  this->delete_another_way = 0;
}

ResultSet::~ResultSet()
{
  delete[] this->var_name;
  if (!this->useStream) {
    if (this->delete_another_way == 0) {
      for (unsigned i = 0; i < this->ansNum; i++) {
        delete[] this->answer[i];
      }
    } else {
      if (this->ansNum > 0)
        delete[] this->answer[0];
    }

    delete[] this->answer;
  } else {
    delete this->stream; //maybe NULL
  }
}

ResultSet::ResultSet(int _v_num, const string* _v_names)
{
  this->select_var_num = _v_num;
  this->var_name = new string[this->select_var_num];
  for (int i = 0; i < this->select_var_num; i++) {
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
ResultSet::setVar(const vector<string>& _var_names)
{
  this->select_var_num = _var_names.size();
  this->var_name = new string[this->select_var_num];
  for (int i = 0; i < this->select_var_num; i++) {
    this->var_name[i] = _var_names[i];
  }
}

//convert to TSV string
string
ResultSet::to_str()
{
  long long ans_num = max((long long)this->ansNum - this->output_offset, 0LL);
  if (this->output_limit != -1)
    ans_num = min(ans_num, (long long)this->output_limit);
  if (ans_num == 0) {
    return "[empty result]\n";
  }

  stringstream _buf;

  for (int i = 0; i < this->select_var_num; i++) {
    if (i != 0)
      _buf << "\t";
    _buf << this->var_name[i];
  }
  _buf << "\n";

  if (this->useStream)
    this->resetStream();

  const Bstr* bp = NULL;
  for (long long i = (!this->useStream ? this->output_offset : 0LL); i < this->ansNum; i++) {
    if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
      break;

    if (this->useStream)
      bp = this->stream->read();

    if (i >= this->output_offset) {
      for (int j = 0; j < this->select_var_num; j++) {
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
string
ResultSet::to_JSON()
{
  stringstream _buf;

  _buf << "{ \"head\": { \"link\": [], \"vars\": [";
  for (int i = 0; i < this->select_var_num; i++) {
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
  for (long long i = (!this->useStream ? this->output_offset : 0LL); i < this->ansNum; i++) {
    if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
      break;

    if (this->useStream)
      bp = this->stream->read();

    if (i >= this->output_offset) {
      if (i != this->output_offset)
        _buf << ",\n";

      _buf << "\t\t\t{ ";

      bool list_empty = true;
      for (int j = 0; j < this->select_var_num; j++) {
        string ans_type, ans_str;

        if (!this->useStream)
          ans_str = this->answer[i][j];
        else
          ans_str = string(bp[j].getStr());

        if (ans_str.length() == 0)
          continue;

        if (!list_empty)
          _buf << ",\t";
        if (ans_str[0] == '<') {
          ans_type = "uri";
          ans_str = ans_str.substr(1, ans_str.length() - 2);
          _buf << "\"" + this->var_name[j].substr(1) + "\": { ";
          _buf << "\"type\": \"" + ans_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
        } else if (ans_str[0] == '"' && ans_str.find("\"^^<") == string::npos && ans_str[ans_str.length() - 1] != '>') {
          ans_type = "literal";
          ans_str = ans_str.substr(1, ans_str.rfind('"') - 1);
          _buf << "\"" + this->var_name[j].substr(1) + "\": { ";
          _buf << "\"type\": \"" + ans_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
        } else if (ans_str[0] == '"' && ans_str.find("\"^^<") != string::npos && ans_str[ans_str.length() - 1] == '>') {
          ans_type = "typed-literal";
          int pos = ans_str.find("\"^^<");
          string data_type = ans_str.substr(pos + 4, ans_str.length() - pos - 5);
          ans_str = ans_str.substr(1, pos - 1);

          _buf << "\"" + this->var_name[j].substr(1) + "\": { ";
          _buf << "\"type\": \"" + ans_type + "\", \"datatype\": \"" + data_type + "\", \"value\": " + Util::node2string((string("\"") + ans_str + "\"").c_str()) + " }";
        } else if (ans_str == "true" || ans_str == "false") {
          ans_type = "bool";
          _buf << "\"" + this->var_name[j].substr(2) + "\": { ";
          _buf << "\"type\": \"" + ans_type + "\", \"value\": " + ans_str.c_str() + " }";
        }
        list_empty = false;
      }
      _buf << "}";
    }
  }

  _buf << "\n\t\t]\n";
  _buf << "\t}\n";
  _buf << "}\n";

  return _buf.str();
}

void
ResultSet::output(FILE* _fp)
{
  if (this->useStream) {
    long long ans_num = max((long long)this->ansNum - this->output_offset, 0LL);
    if (this->output_limit != -1)
      ans_num = min(ans_num, (long long)this->output_limit);
    if (ans_num == 0) {
      fprintf(_fp, "[empty result]\n");
      return;
    }

    fprintf(_fp, "%s", this->var_name[0].c_str());
    for (int i = 1; i < this->select_var_num; i++) {
      fprintf(_fp, "\t%s", this->var_name[i].c_str());
    }
    fprintf(_fp, "\n");

    const Bstr* bp;
    for (long long i = 0; i < this->ansNum; i++) {
      if (this->output_limit != -1 && i == this->output_offset + this->output_limit)
        break;

      bp = this->stream->read();
      if (i >= this->output_offset) {
        fprintf(_fp, "%s", Util::node2string(bp[0].getStr()).c_str());
        for (int j = 1; j < this->select_var_num; j++) {
          fprintf(_fp, "\t%s", Util::node2string(bp[j].getStr()).c_str());
        }
        fprintf(_fp, "\n");
      }
    }
  } else {
    fprintf(_fp, "%s", this->to_str().c_str());
  }
}

void
ResultSet::openStream(vector<unsigned>& _keys, vector<bool>& _desc)
{
  if (this->useStream) {
#ifdef DEBUG_STREAM
    vector<int> debug_keys;
    vector<bool> debug_desc;
    for (int i = 0; i < this->select_var_num; ++i) {
      debug_keys.push_back(i);
      debug_desc.push_back(false);
    }
#endif
    if (this->stream != NULL) {
      delete this->stream;
      this->stream = NULL;
    }
#ifdef DEBUG_STREAM
    if (this->ansNum > 0)
      this->stream = new Stream(debug_keys, debug_desc, this->ansNum, this->select_var_num, true);
#else
    if (this->ansNum > 0)
      this->stream = new Stream(_keys, _desc, this->ansNum, this->select_var_num, _keys.size() > 0);
#endif //DEBUG_STREAM
  }
}

void
ResultSet::resetStream()
{
  if (this->useStream) {
    //this->stream.reset();
    if (this->stream != NULL)
      this->stream->setEnd();
  }
}

void
ResultSet::writeToStream(string& _s)
{
  if (this->useStream) {
    if (this->stream != NULL)
      this->stream->write(_s.c_str(), _s.length());
  }
}

const Bstr*
ResultSet::getOneRecord()
{
  if (this->useStream) {
    if (this->stream == NULL) {
      fprintf(stderr, "ResultSet::getOneRecord(): no results now!\n");
      return NULL;
    }
    if (this->stream->isEnd()) {
      fprintf(stderr, "ResultSet::getOneRecord(): read till end now!\n");
      return NULL;
    }
    //NOTICE:this is one record, and donot free the memory!
    //NOTICE:Bstr[] but only one element, used as Bstr*
    return this->stream->read();
  } else {
    return NULL;
  }
}
