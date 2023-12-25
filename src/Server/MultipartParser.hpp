/*
 * @Author: wangjian 2606583267@qq.com
 * @Date: 2022-12-22 15:07:36
 * @LastEditors: wangjian 2606583267@qq.com
 * @LastEditTime: 2023-02-09 13:38:29
 * @FilePath: /gstore/Server/MultipartParser.hpp
 * @Description: Multipart/form-data Parser
 */
#ifndef MULTIPART_PARSER_HPP
#define	MULTIPART_PARSER_HPP
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <map>

// #define MULTIPART_PARSER_DEBUG

using MultipartFormData = std::map<std::string, std::pair<std::string, std::string>>;


class MultipartParser
{
private:
    /* data */
    std::shared_ptr<std::string> _data;
    std::string _boundary;

    bool _lastBoundaryFound;
    size_t _pos;
    int _lineStart;
    int _lineLength;

    std::string _partName;
    std::string _partFileName;
    std::string _partContentType;
    int _partDataStart;
    int _partDataLength;
public:
    MultipartParser(const std::shared_ptr<std::string> data, 
            const int pos, const std::string boundary);
    std::unique_ptr<MultipartFormData> parse();
private:
    /**
     * parse header, between _boundary line and blank line
     * example:
     * ----------------------------617568955343916342854790 (_boundary line)
     * Content-Disposition: form-data; name="username"      (header)
     *                                                      (blank line)
     * root
     */
    void parseHeaders();
    /**
     * parse form data
     */
    void parseFormData();
    /**
     * get next line
     * update _pos, _lineStart,_lineLength
     * @return true|false
     */
    bool getNextLine();
    /**
     * at boundary line
     * @return true|false
     */
    bool atBoundaryLine();
    /**
     * at end of data
     */
    inline bool atEndOfData(){
        return _pos >= _data->size() || _lastBoundaryFound;
    }            
    std::string getDispositionValue(const std::string source, int pos, const std::string name);
    /**
     * clear l-r blank chars
     * @return std::string
     */
    inline std::string& trim(std::string &s){
        if(s.empty()){ return s; }
        s.erase(0, s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
        return s;
    }
};

MultipartParser::MultipartParser(const std::shared_ptr<std::string> data, 
            const int pos, const std::string boundary)
{
    _data = data;
    _pos = pos;
    _boundary = boundary;
    _lastBoundaryFound = false;
}

std::unique_ptr<MultipartFormData> MultipartParser::parse()
{
    std::unique_ptr<MultipartFormData> p(new MultipartFormData);
    // get next line until at boundary line
    while (getNextLine())
    {
        if (atBoundaryLine())
        {
            break;
        }
    }
    do
    {
        // parse header
        parseHeaders();
        // if at end of data then break
        if (atEndOfData())
        {
            #if defined(MULTIPART_PARSER_DEBUG)
            std::cout << "at end of data" << std::endl;
            #endif // MULTIPART_PARSER_DEBUG
            break;
        }
        // parse form-data
        parseFormData();

        #if defined(MULTIPART_PARSER_DEBUG)        
        std::cout << "_partName:" << _partName << std::endl;
        std::cout << "_partFileName:" << _partFileName << std::endl;
        std::cout << "_partContentType:" << _partContentType << std::endl;
        std::cout << "_partDataLength:" << _partDataLength << std::endl;
        std::cout << "_partDataData:" << _data.get()->substr(_partDataStart, _partDataLength) << std::endl;
        #endif // MULTIPART_PARSER_DEBUG
        
        if (_partFileName.empty()) // normal parameter
        {
            p->insert({_partName, std::move(std::make_pair(_partName, _data.get()->substr(_partDataStart, _partDataLength)))});
            // p->insert(std::move(std::make_pair(_partName, std::make_pair(_partName, _data.get()->substr(_partDataStart, _partDataLength)))));
        }
        else // file
        {
             p->insert({"file", std::move(std::make_pair(_partFileName, _data.get()->substr(_partDataStart, _partDataLength)))});
            // p->insert(std::move(std::make_pair("file", std::make_pair(_partFileName, _data.get()->substr(_partDataStart, _partDataLength)))));
        }
    } while (!atEndOfData());
    return p;
}

bool MultipartParser::getNextLine()
{
    size_t i = _pos;
    _lineStart = -1;

    while (i < _data->size())
    {
        // line end
        if (_data->at(i) == '\n')
        {
            _lineStart = _pos;
            _lineLength = i - _pos;
            _pos = i + 1;
            // 忽略'\r'
            if (_lineLength > 0 && _data->at(i - 1) == '\r')
            {
                _lineLength--;
            }
            break;
        }
        // form-data end
        if (++i == _data->size())
        {
            _lineStart = _pos;
            _lineLength = i - _pos;
            _pos = _data->size();
        }
    }

    return _lineStart >= 0;
}

bool MultipartParser::atBoundaryLine()
{
    int boundaryLength = _boundary.size();
    // boundary line: start with _boundary, and last line: end with _boundary + "--"
    if (boundaryLength != _lineLength &&
        boundaryLength + 2 != _lineLength)
    {
        return false;
    }

    if (_data->substr(_lineStart, boundaryLength) != _boundary)
    {
        return false;
    }
    
    // for (int i = 0; i < boundaryLength; ++i)
    // {
    //     if (_data->at(i + _lineStart) != _boundary[i])
    //     {
    //         return false;
    //     }
    // }

    if (_lineLength == boundaryLength)
    {
        return true;
    }
    // is last line: end with _boundary + "--"
    if (_data->at(boundaryLength + _lineStart) != '-' ||
        _data->at(boundaryLength + _lineStart + 1) != '-')
    {
        return false;
    }

    // is last boundary
    _lastBoundaryFound = true;
    return true;
}

void MultipartParser::parseHeaders(){
    //clear data
    _partFileName.clear();
    _partName.clear();
    _partContentType.clear();

    while(getNextLine()){
        //header end with blank line
        if(_lineLength == 0){ break; }
        const std::string thisLine = _data->substr(_lineStart,_lineLength);

        int index = thisLine.find(':');
        if(index < 0){ continue; }

        const std::string header = thisLine.substr(0, index);
        if(header == "Content-Disposition"){
            _partName = getDispositionValue(thisLine, index + 1, "name");
            _partFileName = getDispositionValue(thisLine, index + 1, "filename");
        }else if(header == "Content-Type"){
            _partContentType = thisLine.substr(index + 1);
            trim(_partContentType);
        }
    }
}

std::string MultipartParser::getDispositionValue(
    const std::string source, int pos, const std::string name)
{
    // Content-Disposition：
    // case 1: Content-Disposition: form-data; name="projectName"
    // case 2: Content-Disposition: form-data; filename="demo.nt"
    // build match pattern: " name="
    std::string pattern = " " + name + "=";
    int i = source.find(pattern, pos);
    // change match pattern: ";name="
    if (i < 0)
    {
        pattern = ";" + name + "=";
        i = source.find(pattern, pos);
    }
    // change match pattern: "name="
    if (i < 0)
    {
        pattern = name + "=";
        i = source.find(pattern, pos);
    }
    // no match, return empty string
    if (i < 0)
    {
        return std::string();
    }

    i += pattern.size();
    if (source[i] == '\"')
    {
        ++i;
        int j = source.find('\"', i);
        if (j < 0 || i == j)
        {
            return std::string();
        }
        return source.substr(i, j - i);
    }
    else
    {
        int j = source.find(";", i);
        if (j < 0)
        {
            j = source.size();
        }
        auto value = source.substr(i, j - i);
        // remove blank chars
        return trim(value);
    }
}

void MultipartParser::parseFormData()
{
    _partDataStart = _pos;
    _partDataLength = -1;

    while (getNextLine())
    {
        if (atBoundaryLine())
        {
            // content data at pre line
            int indexOfEnd = _lineStart - 1;
            if (_data->at(indexOfEnd) == '\n')
            {
                indexOfEnd--;
            }
            if (_data->at(indexOfEnd) == '\r')
            {
                indexOfEnd--;
            }
            _partDataLength = indexOfEnd - _partDataStart + 1;
            break;
        }
    }
}
#endif