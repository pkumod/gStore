#include "Database.h"

string Database::getSchemaPath()
{
	return this->store_path + "/schema.json";
}


nlohmann::json Database::processSchemaNode(const unsigned int& nodeIndex, const std::string& label) {
    nlohmann::json nodeItem;
    
    // 处理主节点
	nodeItem["id"] = nodeIndex;
	nodeItem["label"] = label;
	nodeItem["type"] = 0;
	nodeItem["properties"] = nlohmann::json::array();
    
    // 查询属性
    ResultSet property_rs;
    std::string query_sparql = "select distinct ?p where {?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> " + label 
							 + ". ?s ?p ?o . filter(isLiteral(?o))}";
    
    bool suc = this->query(query_sparql, property_rs, nullptr);
    if (suc && property_rs.ansNum > 0) 
	{
		nodeItem["properties"].get_ref<nlohmann::json::array_t&>().reserve(property_rs.ansNum);
        for (unsigned int j = 0; j < property_rs.ansNum; j++) 
		{
			nlohmann::json propertyItem;
			propertyItem["label"] = property_rs.answer[j][0];
			propertyItem["type"] = 1;
			nodeItem["properties"].push_back(propertyItem);
        }
    }
	return nodeItem;
}

nlohmann::json Database::processSchemaEdge(const std::string& label) {
    std::string query_sparql = "select distinct ?p ?o_label where {?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> " + label 
									 + ". ?s ?p ?o . ?o <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?o_label . "
									 + "filter(?p != <http://www.w3.org/1999/02/22-rdf-syntax-ns#type>) }";
	ResultSet relation_rs;
	bool suc = this->query(query_sparql, relation_rs, nullptr);
	nlohmann::json poItems = nlohmann::json::array();
	if (suc && relation_rs.ansNum > 0)
	{
		for (unsigned int j = 0; j < relation_rs.ansNum; j++)
		{
			nlohmann::json edgeItem;
			edgeItem["source"] = label;
			edgeItem["label"] = relation_rs.answer[j][0];
			edgeItem["target"] = relation_rs.answer[j][1];
			poItems.push_back(edgeItem);
		}
	}
	return poItems;
}

void Database::updateSchema()
{
	if (!this->if_loaded) {
	    throw std::runtime_error("Database is not loaded, can't update schema.");
	}
	schema_lock.lock();
	nlohmann::json scheam = nlohmann::json::object();
	scheam["nodes"] = nlohmann::json::array();
	scheam["edges"] = nlohmann::json::array();
	unsigned int nodeIndex = 1;
	std::unordered_map<std::string, unsigned int> node_index_map;
	ResultSet rs;
	string query_sparql = "select distinct ?o where {?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?o}";
	bool suc = this->query(query_sparql, rs, nullptr);
	if (suc && rs.ansNum > 0)
	{
		// use multi-thread to process nodes
		std::vector<std::future<nlohmann::json>> futures(rs.ansNum);
		std::vector<std::future<nlohmann::json>> edge_futures(rs.ansNum);
		std::vector<nlohmann::json> node_items(rs.ansNum);
		std::vector<nlohmann::json> edge_items(rs.ansNum);
		for (unsigned int i = 0; i < rs.ansNum; i++) 
		{
			futures[i] = std::async(std::launch::async, [this, i, &rs]() {
				return this->processSchemaNode(i, rs.answer[i][0]);
			});
			edge_futures[i] = std::async(std::launch::async, [this, i, &rs]() {
				return this->processSchemaEdge(rs.answer[i][0]);
			});
		}
		for (unsigned int i = 0; i < rs.ansNum; i++) 
		{
			SLOG_INFO("processing futures " << i + 1 << "/" << rs.ansNum);
			node_items[i] = futures[i].get();
			edge_items[i] = edge_futures[i].get();
		}
		nodeIndex = rs.ansNum + 1;
		for (nlohmann::json item : node_items)
		{
			node_index_map[item["label"].get<std::string>()] = item["id"].get<unsigned int>();
			nlohmann::json properties = item["properties"];
			// remove properties from entity node
			item.erase("properties");
			scheam["nodes"] .push_back(item);
			for (nlohmann::json property : properties)
			{
				// property node
				property["id"] = nodeIndex;
				scheam["nodes"] .push_back(property);

				// entity-property edge
				nlohmann::json edgeItem;
				edgeItem["source"] = item["id"];
				edgeItem["target"] = nodeIndex;
				scheam["edges"] .push_back(edgeItem);
				nodeIndex++;
			}
		}

		for (nlohmann::json items: edge_items)
		{
			for (nlohmann::json item : items)
			{
				// edge between entity nodes
				if (node_index_map.find(item["source"].get<std::string>()) != node_index_map.end() 
					&& node_index_map.find(item["target"].get<std::string>()) != node_index_map.end())
				{
					nlohmann::json edgeItem;
					edgeItem["source"] = node_index_map[item["source"].get<std::string>()];
					edgeItem["label"] = item["label"];
					edgeItem["target"] = node_index_map[item["target"].get<std::string>()];
					scheam["edges"] .push_back(edgeItem);
				}
			}
		}
	}

	// first time generate schema, or the schema file is missing, then create it
	if (!gs::FileUtil::fileExists(getSchemaPath()))
	{
		gs::FileUtil::createFile(getSchemaPath());	
	}
	ofstream file;
	file.open(getSchemaPath());
	if (!file.is_open())
	{
		SLOG_WARN("open file " << getSchemaPath() << " failed, create a new one.");
		gs::FileUtil::removeFile(getSchemaPath());
		gs::FileUtil::createFile(getSchemaPath());
		file.open(getSchemaPath());
		if(!file.is_open()) 
		{
			schema_lock.unlock();
			throw std::runtime_error("open schema file failed, please check the file path and permission.");
		}
	}
	file << scheam.dump();
	file.close();
	schema_lock.unlock();
}

void Database::getSchemaInfo(nlohmann::json& schema, bool all)
{
	std::ifstream file(getSchemaPath());
	if (!file.is_open())
	{
		SLOG_WARN(getSchemaPath() << " is not exist, please generate it first.");
		return;
	}
	nlohmann::json info;
	try
	{
		file >> info;
		file.close();
	}
	catch (nlohmann::json::exception& e)
	{
		file.close();
		SLOG_ERROR("schema format is error, message:" << e.what() << ", exception id: " << e.id );
		return;
	}

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
						item["type"] = 0;
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
					item["type"] = 1;
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