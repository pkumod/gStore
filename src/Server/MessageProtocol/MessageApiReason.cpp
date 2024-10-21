#include "MessageApiReason.h"
#include "../ApiProvider.h"

namespace server
{
    void MessageReasonManageResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        if (type == "1")
        {
        }
        else if (type == "2")
        {
            json["list"] = nlohmann::json::array();
            if (list.is_array())
            {
                json["list"] = this->list;
            }
            json["num"] = this->num;
        }
        else if (type == "3")
        {
            json["insert_sparql"] = this->insert_sparql;
            json["delete_sparql"] = this->delete_sparql;
            json["delete_sparql"] = this->check_sparql;
        }
        else if (type == "4")
        {
            json["insert_sparql"] = this->insert_sparql;
            json["AnsNum"] = this->num;
        }
        else if (type == "5")
        {
            json["delete_sparql"] = this->delete_sparql;
            json["AnsNum"] = this->num;
        }
        else if (type == "6")
        {
            json["ruleinfo"] = this->ruleinfo;
        }
        else if (type == "7")
        {
        }
        else if (type == "8")
        {
            json["check_sparql"] = this->check_sparql;
            json["effectNum"] = this->num;
        }

        json_str = json.dump();
    }
}