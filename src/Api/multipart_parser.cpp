#include "multipart_parser.h"
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <future>

namespace web{
namespace http{

const std::string MultipartParser::boundary_prefix_("----CppRestSdkClient");
const std::string MultipartParser::rand_chars_("0123456789"
                                               "abcdefghijklmnopqrstuvwxyz"
                                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
MultipartParser::MultipartParser()
{
  int i = 0;
  int len = rand_chars_.size();
  boundary_ = boundary_prefix_;
  while(i < 16)
  {
    int idx = rand() % len;
    boundary_.push_back(rand_chars_[idx]);
    ++i;
  }
}

const std::string &MultipartParser::GenBodyContent()
{
  std::vector<std::future<std::string> > futures;
  body_content_.clear();
  for(auto &file:files_)
  {
    std::future<std::string> content_futures = std::async(std::launch::async, [&file]()
    {
      std::ifstream ifile(file.second, std::ios::binary | std::ios::ate);
      std::streamsize size = ifile.tellg();
      ifile.seekg(0, std::ios::beg);
      char *buff = new char[size];
      ifile.read(buff, size);
      ifile.close();
      std::string ret(buff, size);
      delete[] buff;
      return ret;
    });
    futures.push_back(std::move(content_futures));
  }

  for(auto &param:params_)
  {
    body_content_ += "\r\n--";
    body_content_ += boundary_;
    body_content_ += "\r\nContent-Disposition: form-data; name=\"";
    body_content_ += param.first;
    body_content_ += "\"\r\n\r\n";
    body_content_ += param.second;
  }

  for(size_t i = 0; i < files_.size(); ++i)
  {
    std::string filename;
    std::string content_type;
    std::string file_content = futures[i].get();
    _get_file_name_type(files_[i].second, &filename, &content_type);
    body_content_ += "\r\n--";
    body_content_ += boundary_;
    body_content_ += "\r\nContent-Disposition: form-data; name=\"";
    body_content_ += files_[i].first;
    body_content_ += "\"; filename=\"";
    body_content_ += filename;
    body_content_ += "\"\r\nContent-Type: ";
    body_content_ += content_type;
    body_content_ += "\r\n\r\n";
    body_content_ += file_content;
  }
  body_content_ += "\r\n--";
  body_content_ += boundary_;
  body_content_ += "--\r\n";
  return body_content_;
}

void MultipartParser::_get_file_name_type(const std::string &file_path, std::string *filename, std::string *content_type)
{
  if (filename == NULL || content_type == NULL) return;

  size_t last_spliter = file_path.find_last_of("/\\");
  *filename = file_path.substr(last_spliter + 1);
  size_t dot_pos = filename->find_last_of(".");
  if (dot_pos == std::string::npos)
  {
    *content_type = "application/octet-stream";
    return;
  }
  std::string ext = filename->substr(dot_pos + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  if (ext == "jpg" || ext == "jpeg")
  {
    *content_type = "image/jpeg";
    return;
  }
  if (ext == "txt" || ext == "log")
  {
    *content_type = "text/plain";
    return;
  }
  *content_type = "application/octet-stream";
  return;
}

} //namespace web::http
} //namespace web

