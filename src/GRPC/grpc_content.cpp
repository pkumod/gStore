#include "workflow/StringUtil.h"
#include "workflow/WFFacilities.h"

#include "grpc_content.h"

using namespace grpc;


std::string ContentType::to_str(enum content_type type)
{
    switch (type)
    {
#define CTM(name, string, suffix) case name: return #string;
        CONTENT_TYPES_MAP(CTM)
#undef CTM
        default:
            return "<unknown>";
    }
}

enum content_type ContentType::to_enum(const std::string &content_type_str)
{
    if (content_type_str.empty())
    {
        return CONTENT_TYPE_NONE;
    }
#define CTM(name, string, suffix) \
    if (StringUtil::start_with(content_type_str.c_str(), #string)) { \
        return name; \
    }
    CONTENT_TYPES_MAP(CTM)
#undef CTM
    return CONTENT_TYPE_UNDEFINED;
}

std::string ContentType::to_str_by_suffix(const std::string &str)
{
    if (str.empty())
    {
        return "";
    }
#define CTM(name, string, suffix) \
    if (str == #suffix) { \
        return #string; \
    }
    CONTENT_TYPES_MAP(CTM)
#undef CTM
    return "";
}

enum content_type ContentType::to_enum_by_suffix(const std::string &str)
{
    if (str.empty()) {
        return CONTENT_TYPE_NONE;
    }
#define CTM(name, string, suffix) \
    if (str == #suffix) { \
        return name; \
    }
    CONTENT_TYPES_MAP(CTM)
#undef CTM
    return CONTENT_TYPE_UNDEFINED;
}

std::map<std::string, std::string> UrlEncode::parse_post_body(const std::string &body)
{
    std::map<std::string, std::string> map;

    if (body.empty())
        return map;

    std::vector<std::string> arr = StringUtil::split(body, '&');

    if (arr.empty())
        return map;

    for (const auto &ele: arr)
    {
        if (ele.empty())
            continue;

        std::vector<std::string> kv = StringUtil::split(ele, '=');
        size_t kv_size = kv.size();
        std::string &key = kv[0];

        if (key.empty() || map.count(key) > 0)
            continue;

        if (kv_size == 1)
        {
            map.emplace(std::move(key), "");
            continue;
        }

        std::string &val = kv[1];

        if (val.empty())
            map.emplace(std::move(key), "");
        else
            map.emplace(std::move(key), std::move(val));
    }
    return map;
}

Timestamp Timestamp::now()
{
    uint64_t timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return Timestamp(timestamp);
}

std::string Timestamp::to_format_str() const
{
    return to_format_str("%Y-%m-%d %X");
}

std::string Timestamp::to_format_str(const char *fmt) const
{
    std::time_t time = micro_sec_since_epoch_ / k_micro_sec_per_sec;  // ms --> s
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), fmt);
    return ss.str();
}

enum multipart_parser_state_e
{
    MP_START,
    MP_PART_DATA_BEGIN,
    MP_HEADER_FIELD,
    MP_HEADER_VALUE,
    MP_HEADERS_COMPLETE,
    MP_PART_DATA,
    MP_PART_DATA_END,
    MP_BODY_END
};

struct multipart_parser_userdata
{
    Form *mp;
    multipart_parser_state_e state;
    std::string header_field;
    std::string header_value;
    std::string part_data;
    std::string name;
    std::string filename;

    StringPiece trim_pairs(const StringPiece &str, const char *pairs);
    StringPiece trim_piece(const StringPiece &str);
    std::vector<StringPiece> split_piece(const StringPiece &str, char sep);

    void handle_header();

    void handle_data();
};

StringPiece multipart_parser_userdata::trim_pairs(const StringPiece &str, const char *pairs)
{
    const char *lhs = str.begin();
    const char *rhs = str.begin() + str.size() - 1;
    const char *p = pairs;
    bool is_pair = false;
    while (*p != '\0' && *(p + 1) != '\0')
    {
        if (*lhs == *p && *rhs == *(p + 1))
        {
            is_pair = true;
            break;
        }
        p += 2;
    }
    return is_pair ? StringPiece(str.begin() + 1, str.size() - 2) : str;
}

StringPiece multipart_parser_userdata::trim_piece(const StringPiece &str)
{
   const char *lhs = str.begin();
    while (lhs != str.end() and std::isspace(*lhs)) lhs++;
    if (lhs == str.end()) return {};
    StringPiece res(str);
    res.remove_prefix(lhs - str.begin());

    const char *rhs = res.end() - 1;
    while (rhs != res.begin() and std::isspace(*rhs)) rhs--;
    if (rhs == res.begin() and std::isspace(*rhs)) return {};
    StringPiece _res(res.begin(), rhs - res.begin() + 1);
    return _res;
}

std::vector<StringPiece> multipart_parser_userdata::split_piece(const StringPiece &str, char sep)
{
    std::vector<StringPiece> res;
    if (str.empty())
        return res;

    const char *p = str.begin();
    const char *cursor = p;

    while (p != str.end())
    {
        if (*p == sep)
        {
            res.emplace_back(StringPiece(cursor, p - cursor));
            cursor = p + 1;
        }
        ++p;
    }
    res.emplace_back(StringPiece(cursor, str.end() - cursor));
    return res;
}

void multipart_parser_userdata::handle_header()
{
    if (header_field.empty() || header_value.empty()) return;
    if (strcasecmp(header_field.c_str(), "Content-Disposition") == 0)
    {
        // Content-Disposition: attachment
        // Content-Disposition: attachment; filename="filename.jpg"
        // Content-Disposition: form-data; name="avatar"; filename="user.jpg"
        StringPiece header_val_piece(header_value);
        std::vector<StringPiece> dispo_list = split_piece(header_val_piece, ';');

        for (auto &dispo: dispo_list)
        {
            auto kv = split_piece(trim_piece(dispo), '=');
            if (kv.size() == 2)
            {
                // name="file"
                // kv[0] is key(name)
                // kv[1] is value("file")
                StringPiece value = trim_pairs(kv[1], R"(""'')");
                if (kv[0].starts_with(StringPiece("name")))
                {
                    name = value.as_string();
                } else if (kv[0].starts_with(StringPiece("filename")))
                {
                    filename = value.as_string();
                }
            }
        }
    }
    header_field.clear();
    header_value.clear();
}

void multipart_parser_userdata::handle_data()
{
    if (!name.empty())
    {
        std::pair<std::string, std::string> formdata;
        formdata.first = filename;
        formdata.second = part_data;
        (*mp)[name] = formdata;
    }
    name.clear();
    filename.clear();
    part_data.clear();
}

const std::string MultiPartForm::k_default_boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

MultiPartForm::MultiPartForm()
{
    settings_ = {
            .on_header_field = header_field_cb,
            .on_header_value = header_value_cb,
            .on_part_data = part_data_cb,
            .on_part_data_begin = part_data_begin_cb,
            .on_headers_complete = headers_complete_cb,
            .on_part_data_end = part_data_end_cb,
            .on_body_end = body_end_cb
    };
}

int MultiPartForm::header_field_cb(multipart_parser *parser, const char *buf, size_t len)
{
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    userdata->handle_header();
    userdata->state = MP_HEADER_FIELD;
    userdata->header_field.append(buf, len);
    return 0;
}

int MultiPartForm::header_value_cb(multipart_parser *parser, const char *buf, size_t len)
{
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    userdata->state = MP_HEADER_VALUE;
    userdata->header_value.append(buf, len);
    return 0;
}

int MultiPartForm::part_data_cb(multipart_parser *parser, const char *buf, size_t len)
{
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    userdata->state = MP_PART_DATA;
    userdata->part_data.append(buf, len);
    return 0;
}

int MultiPartForm::part_data_begin_cb(multipart_parser *parser)
{
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    userdata->state = MP_PART_DATA_BEGIN;
    return 0;
}

int MultiPartForm::headers_complete_cb(multipart_parser *parser)
{
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    userdata->handle_header();
    userdata->state = MP_HEADERS_COMPLETE;
    return 0;
}

int MultiPartForm::part_data_end_cb(multipart_parser *parser)
{
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    userdata->state = MP_PART_DATA_END;
    userdata->handle_data();
    return 0;
}

int MultiPartForm::body_end_cb(multipart_parser *parser)
{
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    userdata->state = MP_BODY_END;
    return 0;
}

Form MultiPartForm::parse_multipart(const StringPiece &body) const
{
    Form form;
    std::string boundary = "--" + boundary_;
    multipart_parser *parser = multipart_parser_init(boundary.c_str(), &settings_);
    multipart_parser_userdata userdata;
    userdata.state = MP_START;
    userdata.mp = &form;
    multipart_parser_set_data(parser, &userdata);
    multipart_parser_execute(parser, body.data(), body.size());
    multipart_parser_free(parser);
    return form;
}

MultiPartEncoder::MultiPartEncoder()
    : boundary_(MultiPartForm::k_default_boundary)
{
}

void MultiPartEncoder::add_param(const std::string &name, const std::string &value) 
{
    params_.push_back({name, value});
}

void MultiPartEncoder::add_file(const std::string &file_name, const std::string &file_path)
{
    files_.push_back({file_name, file_path});
}

void MultiPartEncoder::set_boundary(const std::string &boundary)
{
    boundary_ = boundary;
}

void MultiPartEncoder::set_boundary(std::string &&boundary) 
{
    boundary_ = std::move(boundary);
}