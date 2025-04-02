#include "Database.h"

string Database::getSchemaPath()
{
	return this->store_path + "/schema.json";
}

void Database::buildSchema(const string _rdf_file, const std::map<string, std::set<std::string>>& id_tuples)
{
	if (this->name == GlobalTypedef::system_db)
		return;
	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		SLOG_ERROR("buildSchema: Fail to rdf open : " << _rdf_file);
		return;
	}
	int64_t t1 = gutil::TimeUtil::timestamp();
	std::shared_ptr<TripleWithObjType[]> triple_array(new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP], std::default_delete<TripleWithObjType[]>());
	RDFParser _parser(_fin);
	int num_lines = 0;
	// edges 实体属性映射
	std::map<std::string, std::set<std::string>> propertyMap;
	// 初始化所有的实体类型：避免忽略没有任何属性的实体
	for (const auto& entityL : id_tuples)
	{
		propertyMap[entityL.first] = std::set<std::string>();
	}
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
			if (triple_for_spo.isObjEntity() && !this->checkIsTypePredicate(_pre))
			{
				// 实体-关系-实体
				struct RelationInfo info;
				info.label = _pre;
				for (const auto& entityL : id_tuples)
				{
					if (!info.empty())
						break;
					auto sub_it = entityL.second.find(_sub);
					if (sub_it != entityL.second.end())
						info.source = entityL.first; // entityName;
					auto obj_it = entityL.second.find(_obj);
					if (obj_it != entityL.second.end())
						info.target = entityL.first; // entityName;
				}
				if (!info.empty())
					relationList.insert(info);
			}
			else if (triple_for_spo.isObjLiteral())
			{
				// 实体-属性
				for (const auto& entityL : id_tuples)
				{
					auto sub_it = entityL.second.find(_sub);
					if (sub_it != entityL.second.end())
					{
						propertyMap[entityL.first].insert(_pre);
						break;
					}
				}
			}
		}
	}
	_fin.close();
	createSchema(relationList, propertyMap);
	int64_t t2 = gutil::TimeUtil::timestamp();
	SLOG_CORE("Finish building schema, used " + to_string(t2 - t1) + "ms.");
}

void Database::createSchema(const std::set<struct RelationInfo>& relationList, const std::map<std::string, std::set<std::string>>& propertyMap)
{
	nlohmann::json scheam = nlohmann::json::object();
	scheam["nodes"] = nlohmann::json::array();
	scheam["edges"] = nlohmann::json::array();
	nlohmann::json item = nlohmann::json::object();
	// 实体id与生成index的映射关系
	std::map<std::string, int> node_index_map;
	// type 0:实体, 1:属性
	int nodeIndex = 1;
	for (const auto& m : propertyMap)
	{
		item.clear();
		item["id"] = nodeIndex;
		item["label"] = m.first;
		item["type"] = 0;
		scheam["nodes"] .push_back(item);
		node_index_map[m.first] = nodeIndex;
		nodeIndex++;
		for (const auto& n : m.second)
		{
			item.clear();
			item["id"] = nodeIndex;
			item["label"] = n;
			item["type"] = 1;
			scheam["nodes"] .push_back(item);
			// 生成实体-属性
			item.clear();
			item["source"] = node_index_map[m.first];
			item["target"] = nodeIndex;
			scheam["edges"] .push_back(item);
			nodeIndex++;
		}
	}

	// 生成实体-关系-实体
	for (const auto& m : relationList)
	{
		item.clear();
		item["source"] = node_index_map[m.source];
		item["label"] = m.label;
		item["target"] = node_index_map[m.target];
		scheam["edges"] .push_back(item);
	}
	
	ofstream file;
	file.open(getSchemaPath());
	file << scheam.dump();
	file.close();
}

void Database::updateSchema()
{
	if (this->name == GlobalTypedef::system_db)
		return;
	schema_lock.lock();
	nlohmann::json scheam = nlohmann::json::object();
	scheam["nodes"] = nlohmann::json::array();
	scheam["edges"] = nlohmann::json::array();
	nlohmann::json item = nlohmann::json::object();
	std::string rdf_type = "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>";
	// 实体id与生成index的映射关系
	std::map<std::string, int> node_index_map;
	// type 0:实体, 1:属性
	do
	{   //所有实体类型
		ResultSet rs;
		int nodeIndex = 1;
		string query_sparql = "select distinct ?o where {?s " + rdf_type + " ?o}";
		bool suc = this->query(query_sparql, rs);
		if (suc && rs.ansNum > 0)
		{
			for (unsigned int i = 0; i < rs.ansNum; i++)
			{
				item.clear();
				item["id"] = nodeIndex;
				item["label"] = rs.answer[i][0];
				item["type"] = 0;
				scheam["nodes"] .push_back(item);
				node_index_map[rs.answer[i][0]] = nodeIndex;
				nodeIndex++;
				// 所有实体属性
				ResultSet property_rs;
				query_sparql = "select distinct ?p where {?s " + rdf_type + " " + rs.answer[i][0] + ". ?s ?p ?o . filter(isLiteral(?o))}";
				suc = this->query(query_sparql, property_rs);
				if (suc && property_rs.ansNum > 0)
				{
					for (unsigned int j = 0; j < property_rs.ansNum; j++)
					{
						item.clear();
						item["id"] = nodeIndex;
						item["label"] = property_rs.answer[j][0];
						item["type"] = 1;
						scheam["nodes"] .push_back(item);
						// 生成实体-属性
						item.clear();
						item["source"] = node_index_map[rs.answer[i][0]];
						item["target"] = nodeIndex;
						scheam["edges"] .push_back(item);
						nodeIndex++;
					}
				}
			}
		}
	} while(0);

	do
	{
		// 所有边关系
		ResultSet rs;
		string query_sparql = "select distinct ?source_type ?p ?target_type where {?s " 
							+ rdf_type + " ?source_type. ?s ?p ?o. ?o " + rdf_type + " ?target_type filter(isIRI(?o))}";
		bool suc = this->query(query_sparql, rs);
		if (suc && rs.ansNum > 0)
		{
			for (unsigned int i = 0; i < rs.ansNum; i++)
			{
				item.clear();
				item["source"] = node_index_map[rs.answer[i][0]];
				item["label"] = rs.answer[i][1];
				item["target"] = node_index_map[rs.answer[i][2]];
				scheam["edges"] .push_back(item);
			}
		}
	} while (0);
	
	ofstream file;
	file.open(getSchemaPath());
	file << scheam.dump();
	file.close();
	schema_lock.unlock();
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
						item["label"] = NodeUtil::clear_angle_brackets_and_prefix(m.at("label"));
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
							item["label"] = NodeUtil::clear_angle_brackets_and_prefix(m.at("label"));
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
				item["label"] = NodeUtil::clear_angle_brackets_and_prefix(m["label"]);
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