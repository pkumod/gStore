#include "Database.h"

string Database::getSchemaPath()
{
	return this->store_path + "/schema.json";
}

void Database::buildSchema(const string _rdf_file, const std::map<int, std::set<TYPE_ENTITY_LITERAL_ID>>& id_tuples)
{
	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		SLOG_ERROR("buildSchema: Fail to rdf open : " << _rdf_file);
		return;
	}

	std::shared_ptr<TripleWithObjType[]> triple_array(new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP], std::default_delete<TripleWithObjType[]>());
	RDFParser _parser(_fin);
	int num_lines = 0;
	// nodes
	std::set<int> id_propertys;
	// edges 实体属性映射
	std::map<std::string, std::set<std::string>> propertyMap;
	// 实体关系集合
	std::set<struct RelationInfo> relationList;
	while (true)
	{
		int parse_triple_num = 0;
		int curr_lines = _parser.parseFile(triple_array, parse_triple_num, "", num_lines);
		num_lines = curr_lines;

		if (parse_triple_num == 0)
			break;

		for (int i = 0; i < parse_triple_num; i++)
		{
			TripleWithObjType triple_for_spo = triple_array[i];
			string _sub = triple_for_spo.getSubject();
			string _pre = triple_for_spo.getPredicate();
			string _obj = triple_for_spo.getObject();
            if (_sub.empty() || _pre.empty() || _obj.empty())
                continue;
            TYPE_ENTITY_LITERAL_ID _sub_id = this->kvstore->getIDByEntity(_sub); 
			if (triple_array[i].isObjEntity() && !this->checkIsTypePredicate(_pre))
			{
				TYPE_ENTITY_LITERAL_ID _obj_id = this->kvstore->getIDByString(_obj);
				struct RelationInfo info;
				info.label = _pre;
				for (const auto& entityL : id_tuples)
				{
					if (!info.empty())
						break;
					std::string entity = this->kvstore->getEntityByID(entityL.first);
					auto sub_it = entityL.second.find(_sub_id);
					if (sub_it != entityL.second.end())
						info.source = entity;
					auto obj_it = entityL.second.find(_obj_id);
					if (obj_it != entityL.second.end())
						info.target = entity;
				}
				if (!info.empty())
					relationList.insert(info);
			}
			else if (triple_array[i].isObjLiteral())
			{
				TYPE_PREDICATE_ID _pre_id = this->kvstore->getIDByPredicate(_pre);
				for (const auto& entityL : id_tuples)
				{
					auto sub_it = entityL.second.find(_sub_id);
					if (sub_it != entityL.second.end())
					{
						propertyMap[this->kvstore->getEntityByID(entityL.first)].insert(_pre);
						break;
					}
				}
				id_propertys.insert(_pre_id);
			}
		}
	}
	_fin.close();
	createSchema(id_tuples, id_propertys, relationList, propertyMap);
}

void Database::createSchema(const std::map<int, std::set<TYPE_ENTITY_LITERAL_ID>>& id_tuples, std::set<int>& id_propertys, const std::set<struct RelationInfo>& relationList, const std::map<std::string, std::set<std::string>>& propertyMap)
{
	nlohmann::json scheam = nlohmann::json::object();
	scheam["nodes"] = nlohmann::json::array();
	scheam["edges"] = nlohmann::json::array();
	nlohmann::json item = nlohmann::json::object();
	// type 0:实体, 1:属性
	for (const auto& m : id_tuples)
	{
		item.clear();
		item["id"] = m.first;
		item["label"] = this->kvstore->getEntityByID(m.first);
		item["type"] = 0;
		scheam["nodes"] .push_back(item);
	}
	for (const auto& m : id_propertys)
	{
		item.clear();
		item["id"] = m;
		item["label"] = this->kvstore->getPredicateByID(m);
		item["type"] = 1;
		scheam["nodes"] .push_back(item);
	}

	for (const auto& m : relationList)
	{
		item.clear();
		item["source"] = this->kvstore->getIDByEntity(m.source);
		item["label"] = m.label;
		item["target"] = this->kvstore->getIDByEntity(m.target);
		scheam["edges"] .push_back(item);
	}

	for (const auto& m : propertyMap)
	{
		item.clear();
		for (const auto& n : m.second)
		{
			item["source"] = this->kvstore->getIDByEntity(m.first);
			item["target"] = this->kvstore->getIDByPredicate(n);
			scheam["edges"] .push_back(item);
		}
	}
	
	ofstream file;
	file.open(getSchemaPath());
	file << scheam.dump();
	file.close();
}

void Database::getSchemaInfo(nlohmann::json& schema, bool all)
{
	std::ifstream file(getSchemaPath());
	if (!file.is_open())
	{
		SLOG_ERROR(getSchemaPath() << " is open fail");
		return;
	}
	nlohmann::json info;
	file >> info;
	file.close();

	// entity "#FA8C16"
	// proprey "#5CDBD3"
	schema["nodes"] = nlohmann::json::array();
	schema["edges"] = nlohmann::json::array();
	nlohmann::json item;
	if (info.contains("nodes"))
	{
		for (auto& m : info["nodes"])
		{
			item.clear();
			if (m.contains("type"))
			{
				if (m["type"] == 0)
				{
					if (m.contains("id"))
					{
						TYPE_ENTITY_LITERAL_ID id = m.at("id");
						item["id"] = std::to_string(id);
					}
					if (m.contains("label"))
						item["label"] = Util::clear_angle_brackets(m.at("label"));
					if (all)
						item["color"] = "#FA8C16";
				}
				else if ( m["type"] == 1 && all)
				{
					if (m.contains("id"))
					{
						TYPE_ENTITY_LITERAL_ID id = m.at("id");
						item["id"] = std::to_string(id);
						if (m.contains("label"))
							item["label"] = Util::clear_angle_brackets(m.at("label"));
					}
					item["color"] = "#5CDBD3";
				}
				else
					continue;
			}
			schema["nodes"].push_back(item);
		}
	}
	if (info.contains("edges"))
	{
		int count = 1;
		for (auto& m : info["edges"])
		{
			item.clear();
			if (m.contains("label"))
			{
				// 关系
				if (m.contains("source"))
				{
					TYPE_ENTITY_LITERAL_ID source = m.at("source");
					item["source"] = std::to_string(source);
				}
				if (m.contains("target"))
				{
					TYPE_ENTITY_LITERAL_ID target = m.at("target");
					item["target"] = std::to_string(target);
				}
				item["label"] = Util::clear_angle_brackets(m["label"]);
			}
			else if (all)
			{
				// 属性
				if (m.contains("source"))
				{
					TYPE_ENTITY_LITERAL_ID source = m.at("source");
					item["source"] = std::to_string(source);
				}
				if (m.contains("target"))
				{
					TYPE_ENTITY_LITERAL_ID target = m.at("target");
					item["target"] = std::to_string(target);
				}
			}
			else
			{
				continue;
			}
			item["id"] = std::to_string(count);
			count++;
			schema["edges"].push_back(item);
		}
	}
}