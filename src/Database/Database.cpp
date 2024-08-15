/*=============================================================================
# Filename:		Database.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified:	2018-09-05 14:20
# Description: originally written by liyouhuan, modified by zengli and chenjiaqi
=============================================================================*/

#include "Database.h"

using namespace rapidjson;
using namespace std;

Database::Database()
{
	this->name = "";
	this->store_path = "";
	string store_path = ".";
	this->signature_binary_file = "signature.binary";
	this->six_tuples_file = "six_tuples";
	this->db_info_file = "db_info_file.dat";
	this->statistics_info_file = "statistics_info_file.dat";
	this->id_tuples_file = "id_tuples";
	this->update_log = "update.log";
	this->update_log_since_backup = "update_since_backup.log";
	this->csr = NULL;

	this->type_predicate_name = "type@@TYPE@@类型";
	// this->csr = new CSR[2];

	string kv_store_path = store_path + "/kv_store";
	this->kvstore = new KVstore(kv_store_path);

	// string vstree_store_path = store_path + "/vs_store";
	// this->vstree = new VSTree(vstree_store_path);

	string stringindex_store_path = store_path + "/stringindex_store";
	this->stringindex = new StringIndex(stringindex_store_path);
	this->stringindex->SetTrie(this->kvstore->getTrie());
	// this->encode_mode = Database::STRING_MODE;
	this->encode_mode = Database::ID_MODE;
	this->is_active = false;
	this->sub_num = 0;
	this->pre_num = 0;
	this->literal_num = 0;
	this->entity_num = 0;
	this->triples_num = 0;

	this->join = NULL;
	this->pre2num = NULL;
	this->pre2sub = NULL;
	this->pre2obj = NULL;
	this->entity_buffer = NULL;
	this->entity_buffer_size = 0;
	this->literal_buffer = NULL;
	this->literal_buffer_size = 0;

	this->query_cache = new QueryCache();

	this->if_loaded = false;

	// this->trie = NULL;

	// this->resetIDinfo();
	this->initIDinfo();

	pthread_rwlock_init(&(this->update_lock), NULL);
}

Database::Database(string _name)
{
	this->name = _name;
	size_t found = this->name.find_last_not_of('/');
	if (found != string::npos)
	{
		this->name.erase(found + 1);
	}
	this->store_path = Util::global_config["db_home"] + "/" + this->name + Util::global_config["db_suffix"];

	this->signature_binary_file = "signature.binary";
	this->six_tuples_file = "six_tuples";
	this->db_info_file = "db_info_file.dat";
	this->statistics_info_file = "statistics_info_file.dat";
	this->id_tuples_file = "id_tuples";
	this->update_log = "update.log";
	this->update_log_since_backup = "update_since_backup.log";
	this->csr = NULL;
	// this->csr = new CSR[2];
	this->type_predicate_name = "type@@TYPE@@类型";
	string kv_store_path = store_path + "/kv_store";
	this->kvstore = new KVstore(kv_store_path);
	string stringindex_store_path = store_path + "/stringindex_store";
	this->stringindex = new StringIndex(stringindex_store_path);
	this->stringindex->SetTrie(this->kvstore->getTrie());
	// this->encode_mode = Database::STRING_MODE;
	this->encode_mode = Database::ID_MODE;
	this->is_active = false;
	this->sub_num = 0;
	this->pre_num = 0;
	this->literal_num = 0;
	this->entity_num = 0;
	this->triples_num = 0;

	this->if_loaded = false;

	this->join = NULL;
	this->pre2num = NULL;
	this->pre2sub = NULL;
	this->pre2obj = NULL;
	this->entity_buffer = NULL;
	this->entity_buffer_size = 0;
	this->literal_buffer = NULL;
	this->literal_buffer_size = 0;
	this->query_cache = new QueryCache();

	this->initIDinfo();

	pthread_rwlock_init(&(this->update_lock), NULL);
}

//==================================================================================================================================================
// NOTICE:
// there are 3 ways to manage a dynamic-garbage ID list
// 1. push all unused IDs into list, get top each time to alloc, push freed one to tail, push more large one if NULL
//(can use bit array if stored in disk)
// 2. when free, change the mapping for non-free one whose ID is the largest currently
//(sometimes maybe the copy cost is very high)
// 3. NULL first and push one if freed, get one if not empty(limit+1 if NULL)
//(when stored in disk, maybe consume larger space)
// However, this method can keep the used largest easily(no!! 1->2->3, delete 2 and delete 3, then the max 1 is not kept by limit!)

void Database::initIDinfo()
{
	// NOTICE:keep that limit-1 the maxium using ID
	this->free_id_file_entity = this->getStorePath() + "/freeEntityID.dat";
	this->limitID_entity = 0;
	this->freelist_entity = NULL;

	this->free_id_file_literal = this->getStorePath() + "/freeLiteralID.dat";
	this->limitID_literal = 0;
	this->freelist_literal = NULL;

	this->free_id_file_predicate = this->getStorePath() + "/freePredicateID.dat";
	this->limitID_predicate = 0;
	this->freelist_predicate = NULL;
}

void Database::resetIDinfo()
{
	if (this->if_loaded)
		this->releaseIDBlock();
	this->initIDinfo();
}

void Database::readIDinfo()
{
	this->initIDinfo();

	FILE *fp = NULL;
	// int t = -1;
	TYPE_ENTITY_LITERAL_ID t = INVALID_ENTITY_LITERAL_ID;
	BlockInfo *bp = NULL;

	fp = fopen(this->free_id_file_entity.c_str(), "r");
	if (fp == NULL)
	{
		SLOG_ERROR("read entity id info error");
		return;
	}
	// QUERY:this will reverse the original order, if change?
	// Notice that if we cannot ensure that IDs are uporder and continuous, we can
	// not keep an array for IDs like _entity_bitset
	BlockInfo *tmp = NULL, *cur = NULL;
	fread(&(this->limitID_entity), sizeof(int), 1, fp);
	fread(&t, sizeof(int), 1, fp);
	while (!feof(fp))
	{
		tmp = new BlockInfo(t);
		if (cur == NULL)
		{
			this->freelist_entity = cur = tmp;
		}
		else
		{
			cur->next = tmp;
			cur = tmp;
		}
		fread(&t, sizeof(int), 1, fp);
	}
	fclose(fp);
	fp = NULL;

	fp = fopen(this->free_id_file_literal.c_str(), "r");
	if (fp == NULL)
	{
		SLOG_ERROR("read literal id info error");
		return;
	}

	fread(&(this->limitID_literal), sizeof(int), 1, fp);
	fread(&t, sizeof(int), 1, fp);
	while (!feof(fp))
	{
		bp = new BlockInfo(t, this->freelist_literal);
		this->freelist_literal = bp;
		fread(&t, sizeof(int), 1, fp);
	}
	fclose(fp);
	fp = NULL;

	fp = fopen(this->free_id_file_predicate.c_str(), "r");
	if (fp == NULL)
	{
		SLOG_ERROR("read predicate id info error");
		return;
	}
	fread(&(this->limitID_predicate), sizeof(int), 1, fp);
	fread(&t, sizeof(int), 1, fp);
	while (!feof(fp))
	{
		bp = new BlockInfo(t, this->freelist_predicate);
		this->freelist_predicate = bp;
		fread(&t, sizeof(int), 1, fp);
	}
	fclose(fp);
	fp = NULL;
}

void Database::writeIDinfo()
{
	FILE *fp = NULL;
	BlockInfo *bp = NULL, *tp = NULL;

	fp = fopen(this->free_id_file_entity.c_str(), "w+");
	if (fp == NULL)
	{
		SLOG_ERROR("write entity id info error");
		return;
	}
	fwrite(&(this->limitID_entity), sizeof(int), 1, fp);
	bp = this->freelist_entity;
	while (bp != NULL)
	{
		fwrite(&(bp->num), sizeof(int), 1, fp);
		tp = bp->next;
		delete bp;
		bp = tp;
	}
	fclose(fp);
	fp = NULL;

	fp = fopen(this->free_id_file_literal.c_str(), "w+");
	if (fp == NULL)
	{
		SLOG_ERROR("write literal id info error");
		return;
	}
	fwrite(&(this->limitID_literal), sizeof(int), 1, fp);
	bp = this->freelist_literal;
	while (bp != NULL)
	{
		fwrite(&(bp->num), sizeof(int), 1, fp);
		tp = bp->next;
		delete bp;
		bp = tp;
	}
	fclose(fp);
	fp = NULL;

	fp = fopen(this->free_id_file_predicate.c_str(), "w+");
	if (fp == NULL)
	{
		SLOG_ERROR("write predicate id info error");
		return;
	}
	fwrite(&(this->limitID_predicate), sizeof(int), 1, fp);
	bp = this->freelist_predicate;
	while (bp != NULL)
	{
		fwrite(&(bp->num), sizeof(int), 1, fp);
		tp = bp->next;
		delete bp;
		bp = tp;
	}
	fclose(fp);
	fp = NULL;
}

void Database::saveIDinfo()
{
	FILE *fp = NULL;
	BlockInfo *bp = NULL, *tp = NULL;

	fp = fopen(this->free_id_file_entity.c_str(), "w+");
	if (fp == NULL)
	{
		SLOG_ERROR("write entity id info error");
		return;
	}
	fwrite(&(this->limitID_entity), sizeof(int), 1, fp);
	bp = this->freelist_entity;
	while (bp != NULL)
	{
		fwrite(&(bp->num), sizeof(int), 1, fp);
		tp = bp->next;
		bp = tp;
	}
	Util::Csync(fp);
	fclose(fp);
	fp = NULL;

	fp = fopen(this->free_id_file_literal.c_str(), "w+");
	if (fp == NULL)
	{
		SLOG_ERROR("write literal id info error");
		return;
	}
	fwrite(&(this->limitID_literal), sizeof(int), 1, fp);
	bp = this->freelist_literal;
	while (bp != NULL)
	{
		fwrite(&(bp->num), sizeof(int), 1, fp);
		tp = bp->next;
		bp = tp;
	}
	Util::Csync(fp);
	fclose(fp);
	fp = NULL;

	fp = fopen(this->free_id_file_predicate.c_str(), "w+");
	if (fp == NULL)
	{
		SLOG_ERROR("write predicate id info error");
		return;
	}
	fwrite(&(this->limitID_predicate), sizeof(int), 1, fp);
	bp = this->freelist_predicate;
	while (bp != NULL)
	{
		fwrite(&(bp->num), sizeof(int), 1, fp);
		tp = bp->next;
		bp = tp;
	}
	Util::Csync(fp);
	fclose(fp);
	fp = NULL;
}

// ID alloc garbage error(LITERAL_FIRST_ID or double) add base for literal
TYPE_ENTITY_LITERAL_ID
Database::allocEntityID()
{
	allocEntityID_lock.lock();
	// int t;
	TYPE_ENTITY_LITERAL_ID t = INVALID_ENTITY_LITERAL_ID;

	if (this->freelist_entity == NULL)
	{
		t = this->limitID_entity++;
		if (this->limitID_entity >= Util::LITERAL_FIRST_ID)
		{
			SLOG_ERROR("fail to alloc id for entity");
			// return -1;
			allocEntityID_lock.unlock();
			return INVALID;
		}
	}
	else
	{
		t = this->freelist_entity->num;
		BlockInfo *op = this->freelist_entity;
		this->freelist_entity = this->freelist_entity->next;
		delete op;
	}

	this->entity_num++;
	allocEntityID_lock.unlock();
	return t;
}

void Database::freeEntityID(TYPE_ENTITY_LITERAL_ID _id)
{
	allocEntityID_lock.lock();
	if (_id == this->limitID_entity - 1)
	{
		this->limitID_entity--;
	}
	else
	{
		BlockInfo *p = new BlockInfo(_id, this->freelist_entity);
		this->freelist_entity = p;
	}

	this->entity_num--;
	allocEntityID_lock.unlock();
}

TYPE_ENTITY_LITERAL_ID
Database::allocLiteralID()
{
	allocLiteralID_lock.lock();
	TYPE_ENTITY_LITERAL_ID t = INVALID_ENTITY_LITERAL_ID;

	if (this->freelist_literal == NULL)
	{
		t = this->limitID_literal++;
		if (this->limitID_literal >= Util::LITERAL_FIRST_ID)
		{
			SLOG_ERROR("fail to alloc id for literal");
			// return -1;
			allocLiteralID_lock.unlock();
			return INVALID;
		}
	}
	else
	{
		t = this->freelist_literal->num;
		BlockInfo *op = this->freelist_literal;
		this->freelist_literal = this->freelist_literal->next;
		delete op;
	}

	this->literal_num++;
	allocLiteralID_lock.unlock();
	return t + Util::LITERAL_FIRST_ID;
}

void Database::freeLiteralID(TYPE_ENTITY_LITERAL_ID _id)
{
	allocLiteralID_lock.lock();
	_id -= Util::LITERAL_FIRST_ID;

	if (_id == this->limitID_literal - 1)
	{
		this->limitID_literal--;
	}
	else
	{
		BlockInfo *p = new BlockInfo(_id, this->freelist_literal);
		this->freelist_literal = p;
	}

	this->literal_num--;
	allocLiteralID_lock.unlock();
}

TYPE_PREDICATE_ID
Database::allocPredicateID()
{
	// int t;
	allocPredicateID_lock.lock();
	TYPE_PREDICATE_ID t = INVALID_PREDICATE_ID;

	if (this->freelist_predicate == NULL)
	{
		t = this->limitID_predicate++;
		if (this->limitID_predicate >= static_cast<int>(Util::LITERAL_FIRST_ID))
		{
			SLOG_ERROR("fail to alloc id for predicate");
			// WARN:if pid is changed to unsigned type, this must be changed
			allocPredicateID_lock.unlock();
			return -1;
		}
	}
	else
	{
		t = this->freelist_predicate->num;
		BlockInfo *op = this->freelist_predicate;
		this->freelist_predicate = this->freelist_predicate->next;
		delete op;
	}

	this->pre_num++;
	allocPredicateID_lock.unlock();
	return t;
}

void Database::freePredicateID(TYPE_PREDICATE_ID _id)
{
	allocPredicateID_lock.lock();
	if (_id == this->limitID_predicate - 1)
	{
		this->limitID_predicate--;
	}
	else
	{
		BlockInfo *p = new BlockInfo(_id, this->freelist_predicate);
		this->freelist_predicate = p;
	}

	this->pre_num--;
	allocPredicateID_lock.unlock();
}

void Database::release(FILE *fp0)
{
	fprintf(fp0, "begin to delete DB!\n");
	fflush(fp0);
	fflush(fp0);
	delete this->kvstore;
	fprintf(fp0, "ok to delete kvstore!\n");
	fflush(fp0);
	fprintf(fp0, "ok to delete DB!\n");
	fflush(fp0);
}

Database::~Database()
{
	pthread_rwlock_destroy(&(this->update_lock));
	this->unload();
}

// TODO: update pre map if insert/delete
void Database::setPreMap()
{
	this->maxNumPID = this->minNumPID = INVALID_PREDICATE_ID;
	TYPE_TRIPLE_NUM max = 0, min = this->triples_num + 1;

	this->pre2num = new TYPE_TRIPLE_NUM[this->limitID_predicate];
	this->pre2sub = new TYPE_TRIPLE_NUM[this->limitID_predicate];
	this->pre2obj = new TYPE_TRIPLE_NUM[this->limitID_predicate];
	TYPE_PREDICATE_ID valid = 0, i, t;

	indicators::ProgressBar bar{
		indicators::option::BarWidth{50},
		indicators::option::Start{"["},
		indicators::option::Fill{"="},
		indicators::option::Lead{">"},
		indicators::option::Remainder{" "},
		indicators::option::End{"]"},
		indicators::option::PostfixText{"Set pre map, pre num = " + to_string(limitID_predicate)},
		indicators::option::ForegroundColor{indicators::Color::green},
		indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

	int bar_tmp = 0;
	int one_percent_num = limitID_predicate / 100;

	for (i = 0; i < this->limitID_predicate; ++i)
	{
		++bar_tmp;
		if (bar_tmp == one_percent_num)
		{
			bar.tick();
			bar_tmp = 0;
		}

		if (valid == this->pre_num)
		{
			t = 0;
		}
		else
		{
			t = this->kvstore->getPredicateDegree(i);
		}
		this->pre2num[i] = t;

		// NOTICE:only when pre2num[i]>0 then i is a valid predicate id
		if (t > 0)
		{
			valid++;
			if (static_cast<unsigned>(t) > max)
			{
				this->maxNumPID = i;
			}
			if (static_cast<unsigned>(t) < min)
			{
				this->minNumPID = i;
			}

			unsigned *list = NULL;
			unsigned len = 0;
			this->kvstore->getsubIDlistBypreID(i, list, len, true);
			this->pre2sub[i] = len;
			delete[] list;
			this->kvstore->getobjIDlistBypreID(i, list, len, true);
			this->pre2obj[i] = len;
			delete[] list;
		}
		else
		{
			this->pre2sub[i] = 0;
			this->pre2obj[i] = 0;
		}
	}

	if (!bar.is_completed())
		bar.set_progress(100);
	/*
	for(int i = 0;i < this->pre_num;i++)
	{
		SLOG_CORE("pre2num["<<i<<"]: "<<this->pre2num[i]);
		SLOG_CORE("pre2sub["<<i<<"]: "<<this->pre2sub[i]);
		SLOG_CORE("pre2obj["<<i<<"]: "<<this->pre2obj[i]);
	}
	*/
}

void Database::setStringBuffer()
{
	// BETTER: assign according to memory manager
	// BETTER?maybe different size for entity and literal, maybe different offset should be used
	this->entity_buffer_size = (this->limitID_entity < 50000000) ? this->limitID_entity : 50000000;
	this->literal_buffer_size = (this->limitID_literal < 50000000) ? this->limitID_literal : 50000000;
	this->entity_buffer = new Buffer(this->entity_buffer_size);
	this->literal_buffer = new Buffer(this->literal_buffer_size);

	// DEBUG: insert/delete we should update the size of buffer if adding new string
	// WARN: after delete and insert, IDs may be not continuous, then the string buffer will cause errors!
	TYPE_ENTITY_LITERAL_ID valid = 0, i;
	string str;
	for (i = 0; i < this->entity_buffer_size; ++i)
	{
		if (valid == this->entity_num)
		{
			str = "";
		}
		else
		{
			str = this->kvstore->getEntityByID(i);
		}
		this->entity_buffer->set(i, str);
		if (str != "")
		{
			valid++;
		}
	}

	valid = 0;
	for (i = 0; i < this->literal_buffer_size; ++i)
	{
		if (valid == this->literal_num)
		{
			str = "";
		}
		else
		{
			str = this->kvstore->getLiteralByID(i + Util::LITERAL_FIRST_ID);
		}
		this->literal_buffer->set(i, str);
		if (str != "")
		{
			valid++;
		}
	}

	// use this string buffer in StringIndex class, if not exist, then also use StringIndex to search
	this->stringindex->setBuffer(this->entity_buffer, this->literal_buffer);
}

void Database::warmUp()
{
	// the most frequent triple
	TYPE_PREDICATE_ID pid1 = this->maxNumPID;
	ResultSet rs1;
	string str1 = "select ?s ?o where { ?s " + this->kvstore->getPredicateByID(pid1) + " ?o . }";
	this->query(str1, rs1);
	// the most infrequent triple
	TYPE_PREDICATE_ID pid2 = this->minNumPID;
	ResultSet rs2;
	string str2 = "select ?s ?o where { ?s " + this->kvstore->getPredicateByID(pid2) + " ?o . }";
	this->query(str2, rs2);
}

bool Database::load(Socket &socket, bool loadCSR)
{
	if (this->if_loaded)
	{
		return true;
	}
	bool flag;

#ifndef THREAD_ON
	(this->kvstore)->open();
#else
	int kv_mode = KVstore::READ_WRITE_MODE;

	string msg = "begin to load_KVstore!";
	string resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);

	bool load_success = true;
	thread entity2id_thread(&Database::load_entity2id, this, kv_mode, ref(load_success));
	thread id2entity_thread(&Database::load_id2entity, this, kv_mode, ref(load_success));
	thread literal2id_thread(&Database::load_literal2id, this, kv_mode, ref(load_success));
	thread id2literal_thread(&Database::load_id2literal, this, kv_mode, ref(load_success));
	thread predicate2id_thread(&Database::load_predicate2id, this, kv_mode, ref(load_success));
#ifndef ONLY_READ
	thread id2predicate_thread(&Database::load_id2predicate, this, kv_mode, ref(load_success));
#endif
	thread sub2values_thread(&Database::load_sub2values, this, kv_mode, ref(load_success));
	thread obj2values_thread(&Database::load_obj2values, this, kv_mode, ref(load_success));
	thread pre2values_thread(&Database::load_pre2values, this, kv_mode, ref(load_success));
#endif

	flag = this->loadDBInfoFile();
	if (!flag)
	{
		SLOG_ERROR("load database info error. @Database::load()");
		return false;
	}
	SLOG_CORE("load database info successfully!");
	if (!(this->kvstore)->load_trie(kv_mode))
	{
		SLOG_ERROR("load kvstore failed.");
		return false;
	}
	else
	{
		SLOG_CORE("load kvstore successfully!");
	}

	msg = "begin to load stringindex!";
	SLOG_CORE(msg);
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	this->stringindex->load();
	msg = "load stringindex  successfully!";
	SLOG_CORE(msg);
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	this->readIDinfo();
	SLOG_CORE("read IDInfo file  successfully!");

#ifdef THREAD_ON
	pre2values_thread.join();
	msg = "load_pre2values successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
#endif
	this->setPreMap();

#ifdef THREAD_ON
	id2entity_thread.join();
	msg = "load_id2entity successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	id2literal_thread.join();
	msg = "load_id2literal successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
#endif

#ifndef ONLY_READ
#ifdef THREAD_ON
	id2predicate_thread.join();
	msg = "load_id2predicate successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
#endif
#endif

#ifdef THREAD_ON
	entity2id_thread.join();
	msg = "load_entity2id successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	literal2id_thread.join();
	msg = "load_literal2id successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	predicate2id_thread.join();
	msg = "load_predicate2id successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	sub2values_thread.join();
	msg = "load_sub2values successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	obj2values_thread.join();
	msg = "load_obj2values successfully!";
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	if (!load_success)
	{
		SLOG_ERROR(getName() << " data has been corrupted");
		throw runtime_error("data has been corrupted");
	}
#endif
	msg = "begin load cache!";
	SLOG_CORE(msg);
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);
	this->load_cache();
	msg = "load cache successfully!";
	SLOG_CORE(msg);
	resJson = CreateJson(1, "loading", msg);
	socket.send(resJson);

	this->if_loaded = true;

	SLOG_CORE("finish load");
	print_data_count();

#ifdef ONLY_READ
	this->kvstore->close_id2entity();
	this->kvstore->close_id2literal();
#endif

	if (loadCSR)
	{
		this->csr = new CSR[2];
		unsigned pre_num = this->getStringIndex()->getNum(StringIndexFile::Predicate);
		this->csr[0].init(pre_num);
		this->csr[1].init(pre_num);
		SLOG_CORE("pre_num: " << pre_num);
		long begin_time = Util::get_cur_time();

		// Process out-edges (csr[0])
		// i: predicate; j: subject; k: object
		for (unsigned i = 0; i < pre_num; i++)
		{
			string pre = (this->getKVstore())->getPredicateByID(i);
			SLOG_CORE("pid: " << i << "    pre: " << pre);
			unsigned *sublist = NULL;
			unsigned sublist_len = 0;
			(this->getKVstore())->getsubIDlistBypreID(i, sublist, sublist_len, true);
			unsigned offset = 0;
			unsigned index = 0;
			for (unsigned j = 0; j < sublist_len; j++)
			{
				string sub = (this->getKVstore())->getEntityByID(sublist[j]);
				unsigned *objlist = NULL;
				unsigned objlist_len = 0;
				(this->getKVstore())->getobjIDlistBysubIDpreID(sublist[j], i, objlist, objlist_len);
				unsigned len = objlist_len; // the real object list length
				for (unsigned k = 0; k < objlist_len; k++)
				{
					if (objlist[k] >= 2000000000)
					{
						--len;
						continue;
					}
					string obj = (this->getKVstore())->getEntityByID(objlist[k]);
					this->csr[0].adjacency_list[i].push_back(objlist[k]);
				}
				if (len > 0)
				{
					this->csr[0].id2vid[i].push_back(sublist[j]);
					this->csr[0].vid2id[i].insert(pair<unsigned, unsigned>(sublist[j], index));
					this->csr[0].offset_list[i].push_back(offset);
					index++;
					offset += len;
				}
			}
			SLOG_CORE(this->csr[0].offset_list[i].size());   // # of this predicate's subjects
			SLOG_CORE(this->csr[0].adjacency_list[i].size());// # of this predicate's objects
		}

		// Process out-edges (csr[1])
		// i: predicate; j: object; k: subject
		for (unsigned i = 0; i < pre_num; i++)
		{
			string pre = (this->getKVstore())->getPredicateByID(i);
			SLOG_CORE("pid: " << i << "    pre: " << pre);
			unsigned *objlist = NULL;
			unsigned objlist_len = 0;
			(this->getKVstore())->getobjIDlistBypreID(i, objlist, objlist_len, true);
			unsigned offset = 0;
			unsigned index = 0;
			for (unsigned j = 0; j < objlist_len; j++)
			{
				if (objlist[j] >= 2000000000)
					continue;
				string obj = (this->getKVstore())->getEntityByID(objlist[j]);
				unsigned *sublist = NULL;
				unsigned sublist_len = 0;
				(this->getKVstore())->getsubIDlistByobjIDpreID(objlist[j], i, sublist, sublist_len);
				unsigned len = sublist_len;
				for (unsigned k = 0; k < sublist_len; k++)
				{
					string sub = (this->getKVstore())->getEntityByID(sublist[k]);
					this->csr[1].adjacency_list[i].push_back(sublist[k]);
				}
				if (len > 0)
				{
					this->csr[1].id2vid[i].push_back(objlist[j]);
					this->csr[1].vid2id[i].insert(pair<unsigned, unsigned>(objlist[j], index));
					this->csr[1].offset_list[i].push_back(offset);
					index++;
					offset += len;
				}
			}
			SLOG_CORE(this->csr[1].offset_list[i].size());
			SLOG_CORE(this->csr[1].adjacency_list[i].size());
		}
		csr[1].n = this->entity_num;

		unsigned ret = 0;
		for (int i = 0; i < csr[1].pre_num; i++)	// Same as summing that of csr[0]
			ret += csr[1].adjacency_list[i].size();
		csr[1].m = ret;

		SLOG_CORE("total vertices " << csr[1].n);
		SLOG_CORE("total edges " << csr[1].m);
		long end_time = Util::get_cur_time();
		SLOG_CORE("after creating CSR, used " << (end_time - begin_time) << "ms");
		SLOG_CORE("CSR size = " << csr[0].sizeInBytes() + csr[1].sizeInBytes() << " (bytes)");
	}

	this->loadStatisticsInfoFile();
	return true;
}

bool Database::load(bool loadCSR)
{

	if (this->if_loaded)
	{
		return true;
	}

	// TODO: acquire this arg from memory manager
	// BETTER: get return value from subthread(using ref or file as hub)
	//  unsigned vstree_cache = gstore::LRUCache::DEFAULT_CAPACITY;
	bool flag;

	SLOG_CORE("---------Begin to Load Database `" << name << "`---------");
#ifndef THREAD_ON
	(this->kvstore)->open();
#else
	bool load_success = true;
	int kv_mode = KVstore::READ_WRITE_MODE;
	thread entity2id_thread(&Database::load_entity2id, this, kv_mode, ref(load_success));
	thread id2entity_thread(&Database::load_id2entity, this, kv_mode, ref(load_success));
	thread literal2id_thread(&Database::load_literal2id, this, kv_mode, ref(load_success));
	thread id2literal_thread(&Database::load_id2literal, this, kv_mode, ref(load_success));
	thread predicate2id_thread(&Database::load_predicate2id, this, kv_mode, ref(load_success));
#ifndef ONLY_READ
	thread id2predicate_thread(&Database::load_id2predicate, this, kv_mode, ref(load_success));
#endif
	thread sub2values_thread(&Database::load_sub2values, this, kv_mode, ref(load_success));
	thread obj2values_thread(&Database::load_obj2values, this, kv_mode, ref(load_success));
	thread pre2values_thread(&Database::load_pre2values, this, kv_mode, ref(load_success));
#endif

	// this is very fast
	flag = this->loadDBInfoFile();
	if (!flag)
	{
		SLOG_ERROR("Load database info error. @Database::load()");
		return false;
	}
	SLOG_CORE("Database info loaded successfully!");

	if (!(this->kvstore)->load_trie(kv_mode))
	{
		SLOG_ERROR("Kvstore trie load failed.");
		return false;
	}
	else
	{
		SLOG_CORE("Kvstore trie loaded successfully!");
	}

	// NOTICE: we should also run some heavy work in the main thread
	SLOG_CORE("Begin to load StringIndex ......");
	this->stringindex->load();
	SLOG_CORE("StringIndex loaded successfully!");
	SLOG_CORE("Begin to read IDInfo ......");
	this->readIDinfo();
	SLOG_CORE("Read IDInfo file successfully!");

#ifdef THREAD_ON
	pre2values_thread.join();
#endif

	SLOG_CORE("Begin to set pre map ......");
	this->setPreMap();

#ifdef THREAD_ON
	id2entity_thread.join();
	id2literal_thread.join();
#endif

	// BETTER: if we set string buffer using string index instead of B+Tree, then we can
	// avoid to load id2entity and id2literal in ONLY_READ mode

	// generate the string buffer for entity and literal, no need for predicate
	// NOTICE:the total string size should not exceed 20G, assume that most strings length < 500
	// too many empty between entity and literal, so divide them

	// this->setStringBuffer();

	// NOTICE: we should build string buffer from kvstore, not string index
	// Because when searching in string index, it will first check if in buffer(but the buffer is being built)

#ifndef ONLY_READ
#ifdef THREAD_ON
	id2predicate_thread.join();
#endif
#endif

#ifdef THREAD_ON
	entity2id_thread.join();
	literal2id_thread.join();
	predicate2id_thread.join();
	sub2values_thread.join();
	obj2values_thread.join();
	if (!load_success)
	{
		SLOG_ERROR(getName() << " data has been corrupted");
		throw runtime_error("data has been corrupted");
	}
#endif
	// load cache of sub2values and obj2values
	SLOG_CORE("Begin to load p2v, s2v and o2v cache ......");
	this->load_cache();
	SLOG_CORE("Cache loaded successfully!");
	// warm up always as finishing build(), to utilize the system buffer
	// this->warmUp();
	// DEBUG:the warmUp() calls query(), which will also output results, this is not we want

	// load the statistics file of db
	this->loadStatisticsInfoFile();
	SLOG_CORE("Statistics Info file loaded successfully!");

	this->if_loaded = true;

	// BETTER: for only-read application(like endpoint), 3 id2values trees can be closed now
	// and we should load all trees on only READ mode

	// HELP: just for checking infos(like kvstore)
	print_data_count();

	SLOG_CORE("---------Finish Database `" << name << "` Load---------");

#ifdef ONLY_READ
	this->kvstore->close_id2entity();
	this->kvstore->close_id2literal();
#endif

	if (loadCSR)
	{
		this->csr = new CSR[2];
		unsigned pre_num = this->getStringIndex()->getNum(StringIndexFile::Predicate);
		this->csr[0].init(pre_num);
		this->csr[1].init(pre_num);
		SLOG_CORE("pre_num: " << pre_num);
		long begin_time = Util::get_cur_time();

		// Process out-edges (csr[0])
		// i: predicate; j: subject; k: object
		for (unsigned i = 0; i < pre_num; i++)
		{
			string pre = (this->getKVstore())->getPredicateByID(i);
			SLOG_CORE("pid: " << i << "    pre: " << pre);
			unsigned *sublist = NULL;
			unsigned sublist_len = 0;
			// todo: check return value
			(this->getKVstore())->getsubIDlistBypreID(i, sublist, sublist_len, true);
			// bool ret = (this->getKVstore())->getsubIDlistBypreID(i, sublist, sublist_len, true);
			unsigned offset = 0;
			unsigned index = 0;
			for (unsigned j = 0; j < sublist_len; j++)
			{
				string sub = (this->getKVstore())->getEntityByID(sublist[j]);
				unsigned *objlist = NULL;
				unsigned objlist_len = 0;
				// todo: check return value
				(this->getKVstore())->getobjIDlistBysubIDpreID(sublist[j], i, objlist, objlist_len);
				// bool ret = (this->getKVstore())->getobjIDlistBysubIDpreID(sublist[j], i, objlist, objlist_len);
				unsigned len = objlist_len; // the real object list length
				for (unsigned k = 0; k < objlist_len; k++)
				{
					if (objlist[k] >= 2000000000)
					{
						--len;
						continue;
					}
					string obj = (this->getKVstore())->getEntityByID(objlist[k]);
					this->csr[0].adjacency_list[i].push_back(objlist[k]);
				}
				if (len > 0)
				{
					this->csr[0].id2vid[i].push_back(sublist[j]);
					this->csr[0].vid2id[i].insert(pair<unsigned, unsigned>(sublist[j], index));
					this->csr[0].offset_list[i].push_back(offset);
					index++;
					offset += len;
				}
				delete [] objlist;
				objlist = nullptr;
			}
			// if(this->csr[0].offset_list[i].size() == 0)
			// 	this->csr[0].valid[i] = false;
			// else
			// {
			// 	if((i==3)||(i==4))
			// 		this->csr[0].valid[i] = true;
			// 	else
			// 		this->csr[0].valid[i] = false;
			// }
			SLOG_CORE(this->csr[0].offset_list[i].size());
			SLOG_CORE(this->csr[0].adjacency_list[i].size());
			delete [] sublist;
			sublist = nullptr;
		}

		// Process out-edges (csr[1])
		// i: predicate; j: object; k: subject
		for (unsigned i = 0; i < pre_num; i++)
		{
			string pre = (this->getKVstore())->getPredicateByID(i);
			SLOG_CORE("pid: " << i << "    pre: " << pre);
			unsigned *objlist = NULL;
			unsigned objlist_len = 0;
			// todo: check return value
			(this->getKVstore())->getobjIDlistBypreID(i, objlist, objlist_len, true);
			// bool ret = (this->getKVstore())->getobjIDlistBypreID(i, objlist, objlist_len, true);
			unsigned offset = 0;
			unsigned index = 0;
			for (unsigned j = 0; j < objlist_len; j++)
			{
				if (objlist[j] >= 2000000000)
					continue;
				string obj = (this->getKVstore())->getEntityByID(objlist[j]);
				unsigned *sublist = NULL;
				unsigned sublist_len = 0;
				// todo: check return value
				(this->getKVstore())->getsubIDlistByobjIDpreID(objlist[j], i, sublist, sublist_len);
				// bool ret = (this->getKVstore())->getsubIDlistByobjIDpreID(objlist[j], i, sublist, sublist_len);
				unsigned len = sublist_len;
				for (unsigned k = 0; k < sublist_len; k++)
				{
					string sub = (this->getKVstore())->getEntityByID(sublist[k]);
					this->csr[1].adjacency_list[i].push_back(sublist[k]);
				}
				if (len > 0)
				{
					this->csr[1].id2vid[i].push_back(objlist[j]);
					this->csr[1].vid2id[i].insert(pair<unsigned, unsigned>(objlist[j], index));
					this->csr[1].offset_list[i].push_back(offset);
					index++;
					offset += len;
				}
				delete [] sublist;
				sublist = nullptr;
			}
			// if(this->csr[1].offset_list[i].size() == 0)
			// 	this->csr[1].valid[i] = false;
			// else
			// {
			// 	if((i==5)||(i==13)||(i==14)||(i==15))
			// 		this->csr[1].valid[i] = true;
			// 	else
			// 		this->csr[1].valid[i] = false;
			// }
			SLOG_CORE(this->csr[1].offset_list[i].size());
			SLOG_CORE(this->csr[1].adjacency_list[i].size());
			delete [] objlist;
			objlist = nullptr;
		}
		csr[1].n = this->entity_num;

		unsigned ret = 0;
		for (auto i = 0; i < csr[1].pre_num; i++)	// Same as summing that of csr[0]
			ret += csr[1].adjacency_list[i].size();
		csr[1].m = ret;

		SLOG_CORE("total vertices " << csr[1].n);
		SLOG_CORE("total edges " << csr[1].m);
		long end_time = Util::get_cur_time();
		SLOG_CORE("after creating CSR, used " << (end_time - begin_time) << "ms");
		SLOG_CORE("CSR size = " << csr[0].sizeInBytes() + csr[1].sizeInBytes() << " (bytes)");
	}

	return true;
}

void Database::load_cache()
{
	indicators::ProgressBar bar{
		indicators::option::BarWidth{50},
		indicators::option::Start{"["},
		indicators::option::Fill{"="},
		indicators::option::Lead{">"},
		indicators::option::Remainder{" "},
		indicators::option::End{"]"},
		indicators::option::PostfixText{"Load p2v cache 0/3"},
		indicators::option::ForegroundColor{indicators::Color::green},
		indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

	// get important pre ID
	// a pre whose degree is more than 50% of max pre degree is important pre
	this->get_important_preID();
	this->load_candidate_pre2values();

	bar.set_option(indicators::option::PostfixText{"Load s2v cache 1/3"});
	bar.set_progress(33);
	this->load_important_sub2values();

	bar.set_option(indicators::option::PostfixText{"Load o2v cache 2/3"});
	bar.set_progress(66);
	this->load_important_obj2values();

	bar.set_option(indicators::option::PostfixText{"Load cache done!"});
	bar.set_progress(100); // all done
}

void Database::get_important_preID()
{
	important_preID.clear();
	unsigned max_degree = 0;
	for (TYPE_PREDICATE_ID i = 0; i < limitID_predicate; ++i)
		if (pre2num[i] > max_degree)
			max_degree = pre2num[i];
	unsigned limit_degree = max_degree / 2;
	for (TYPE_PREDICATE_ID i = 0; i < limitID_predicate; ++i)
		if (pre2num[i] > limit_degree)
			important_preID.push_back(i);
}

void Database::load_important_obj2values()
{
	this->get_important_objID();
	this->build_CacheOfObj2values();
}

void Database::load_important_sub2values()
{
	this->get_important_subID();
	this->build_CacheOfSub2values();
}

void Database::load_candidate_pre2values()
{
	this->get_candidate_preID();
	this->build_CacheOfPre2values();
}

void Database::get_candidate_preID()
{
	unsigned now_total_size = 0;
	const unsigned max_total_size = 2000000000; // 2G
	std::priority_queue<KEY_SIZE_VALUE, deque<KEY_SIZE_VALUE>, greater<KEY_SIZE_VALUE>> rubbish;
	while (!rubbish.empty())
		rubbish.pop();
	while (!candidate_preID.empty())
		candidate_preID.pop();
	for (TYPE_PREDICATE_ID i = 0; i < limitID_predicate; ++i)
	{
		unsigned _value = 0;
		unsigned _size;
		unsigned long _tmp_size;
		_tmp_size = this->kvstore->getPreListSize(i);
		if (_tmp_size > ((unsigned long)1 << 31))
			continue;
		_size = (unsigned)_tmp_size;
		if (!VList::isLongList(_size) || _size >= max_total_size)
			continue; // only long list need to be stored in cache

		_value = pre2num[i];
		if (_value == 0)
			continue;

		if (_size + now_total_size < max_total_size)
		{
			candidate_preID.push(KEY_SIZE_VALUE(i, _size, _value));
			now_total_size += _size;
		}
		else
		{
			if (candidate_preID.empty())
				continue;
			if (_value > candidate_preID.top().value)
			{
				while (now_total_size + _size >= max_total_size)
				{
					if (candidate_preID.top().value >= _value)
						break;
					rubbish.push(candidate_preID.top());
					now_total_size -= candidate_preID.top().size;
					candidate_preID.pop();
				}
				if (now_total_size + _size < max_total_size)
				{
					now_total_size += _size;
					candidate_preID.push(KEY_SIZE_VALUE(i, _size, _value));
				}
				while (!rubbish.empty())
				{
					if (now_total_size + rubbish.top().size < max_total_size)
					{
						now_total_size += rubbish.top().size;
						candidate_preID.push(rubbish.top());
					}
					rubbish.pop();
				}
			}
		}
	}
}

void Database::build_CacheOfPre2values()
{
	while (!candidate_preID.empty())
	{
		this->kvstore->AddIntoPreCache(candidate_preID.top().key);
		candidate_preID.pop();
	}
}

void Database::build_CacheOfObj2values()
{
	while (!important_objID.empty())
	{
		this->kvstore->AddIntoObjCache(important_objID.top().key);
		important_objID.pop();
	}
}

void Database::build_CacheOfSub2values()
{
	while (!important_subID.empty())
	{
		this->kvstore->AddIntoSubCache(important_subID.top().key);
		important_subID.pop();
	}
}

void Database::get_important_subID()
{
	while (!important_subID.empty())
		important_subID.pop();
	unsigned now_total_size = 0;
	const string invalid = "";
	const unsigned max_total_size = 2000000000; // 2G
	std::priority_queue<KEY_SIZE_VALUE, deque<KEY_SIZE_VALUE>, greater<KEY_SIZE_VALUE>> rubbish;
	while (!rubbish.empty())
		rubbish.pop();
	// a sub who has largest degree with important pre is important subs
	for (TYPE_ENTITY_LITERAL_ID i = 0; i < limitID_entity; ++i)
	{
		unsigned _value = 0;
		unsigned long _tmp_size = 0;
		if (this->kvstore->getEntityByID(i) == invalid)
			continue;
		_tmp_size = this->kvstore->getSubListSize(i);
		if (_tmp_size >= ((unsigned long)1 << 31))
			continue;
		unsigned _size = (unsigned)_tmp_size;
		if (!VList::isLongList(_size) || _size >= max_total_size)
			continue; // only long list need to be stored in cache

		for (unsigned j = 0; j < important_preID.size(); ++j)
		{
			_value += this->kvstore->getSubjectPredicateDegree(i, j);
		}
		if (_size + now_total_size < max_total_size)
		{
			important_subID.push(KEY_SIZE_VALUE(i, _size, _value));
			now_total_size += _size;
		}
		else
		{
			if (important_subID.empty())
				continue;
			if (_value > important_subID.top().value)
			{
				while (now_total_size + _size >= max_total_size)
				{
					if (important_subID.top().value >= _value)
						break;
					rubbish.push(important_subID.top());
					now_total_size -= important_subID.top().size;
					important_subID.pop();
				}
				if (now_total_size + _size < max_total_size)
				{
					now_total_size += _size;
					important_subID.push(KEY_SIZE_VALUE(i, _size, _value));
				}
				while (!rubbish.empty())
				{
					if (now_total_size + rubbish.top().size < max_total_size)
					{
						now_total_size += rubbish.top().size;
						important_subID.push(rubbish.top());
					}
					rubbish.pop();
				}
			}
		}
	}
}

void Database::get_important_objID()
{
	while (!important_objID.empty())
		important_objID.pop();
	unsigned now_total_size = 0;
	const unsigned max_total_size = 2000000000; // 2G
	const string invalid = "";
	std::priority_queue<KEY_SIZE_VALUE, deque<KEY_SIZE_VALUE>, greater<KEY_SIZE_VALUE>> rubbish;
	while (!rubbish.empty())
		rubbish.pop();
	// a sub who has largest degree with important pre is important subs
	for (TYPE_ENTITY_LITERAL_ID i = 0; i < limitID_literal; ++i)
	{
		unsigned _value = 0;
		unsigned _size;
		string _tmp;
		if (i < limitID_entity)
			_tmp = this->kvstore->getEntityByID(i);
		else
			_tmp = this->kvstore->getLiteralByID(i);
		if (_tmp == invalid)
			continue;

		unsigned long _tmp_size = 0;
		_tmp_size = this->kvstore->getSubListSize(i);

		_tmp_size = this->kvstore->getObjListSize(i);
		if (_tmp_size >= ((unsigned long)1 << 31))
			continue;
		_size = (unsigned)_tmp_size;

		if (!VList::isLongList(_size) || _size >= max_total_size)
			continue; // only long list need to be stored in cache

		for (unsigned j = 0; j < important_preID.size(); ++j)
		{
			_value += this->kvstore->getObjectPredicateDegree(i, j);
		}

		if (_size + now_total_size < max_total_size)
		{
			important_objID.push(KEY_SIZE_VALUE(i, _size, _value));
			now_total_size += _size;
		}
		else
		{
			if (important_objID.empty())
				continue;
			if (_value > important_objID.top().value)
			{
				while (now_total_size + _size >= max_total_size)
				{
					if (important_objID.top().value >= _value)
						break;
					rubbish.push(important_objID.top());
					now_total_size -= important_objID.top().size;
					important_objID.pop();
				}
				if (now_total_size + _size < max_total_size)
				{
					now_total_size += _size;
					important_objID.push(KEY_SIZE_VALUE(i, _size, _value));
				}
				while (!rubbish.empty())
				{
					if (now_total_size + rubbish.top().size < max_total_size)
					{
						now_total_size += rubbish.top().size;
						important_objID.push(rubbish.top());
					}
					rubbish.pop();
				}
			}
		}
	}
}

void Database::load_entity2id(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_entity2id(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

void Database::load_id2entity(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_id2entity(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

void Database::load_literal2id(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_literal2id(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

void Database::load_id2literal(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_id2literal(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

void Database::load_predicate2id(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_predicate2id(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

void Database::load_id2predicate(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_id2predicate(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

void Database::load_sub2values(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_subID2values(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

void Database::load_obj2values(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_objID2values(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

void Database::load_pre2values(int _mode, bool& load_success)
{
	try
	{
		this->kvstore->open_preID2values(_mode);
	}
	catch (const std::runtime_error &e2)
	{
		load_success = false;
		string content = e2.what();
		SLOG_ERROR(content);
	}
	catch (...)
	{
		load_success = false;
		string content = "unknow error";
		SLOG_ERROR(content);
	}
}

// @author bookug
// @email bookug@qq.com
// @function check some parameters, statues and correctness of the database
void Database::print_data_count()
{
	SLOG_CORE("Triple num: " << this->triples_num);
	SLOG_CORE("Pre num: " << this->pre_num);
	SLOG_CORE("Entity num: " << this->entity_num);
	SLOG_CORE("Literal num: " << this->literal_num);
}

void Database::query_stringIndex(int id)
{
	string str;
	char* string_index_buffer = NULL;
	unsigned string_index_buffer_size = 0;
	this->stringindex->randomAccess(id, &str, string_index_buffer, string_index_buffer_size, true);
	if (string_index_buffer != NULL)
	{
		delete[] string_index_buffer;
		string_index_buffer = NULL;
	}
	
	SLOG_CORE("thread: " << id << " " << str);
}

// NOTICE: we ensure that if the unload() exists normally, then all updates have already been written to disk
// So when accidents happens, we only have to restore the databases that are in load status(inlucidng that unload
// not finished) later.
bool Database::unload()
{
	// TODO: do we need to update the pre2num if update queries exist??
	// or we just neglect this, that is ok because pre2num is just used to count
	delete[] this->pre2num;
	this->pre2num = NULL;
	delete[] this->pre2sub;
	this->pre2sub = NULL;
	delete[] this->pre2obj;
	this->pre2obj = NULL;
	delete this->entity_buffer;
	this->entity_buffer = NULL;
	delete this->literal_buffer;
	this->literal_buffer = NULL;

	delete this->kvstore;
	this->kvstore = NULL;
	delete this->stringindex;
	this->stringindex = NULL;

	if (if_loaded)
	{
		this->saveDBInfoFile();
		// this->saveStatisticsInfoFile();
		this->writeIDinfo();
		this->initIDinfo();
	}

	delete this->query_cache;

	delete [] this->csr;
	this->csr = NULL;

	this->if_loaded = false;
	this->clear_update_log();

	return true;
}

void Database::releaseIDBlock()
{
	if (this->freelist_entity != nullptr)
	{
		BlockInfo* p = this->freelist_entity;
		while (p != nullptr)
		{
			BlockInfo *np = p->next;
			delete p;
			p = np;
		}
	}
	if (this->freelist_literal != nullptr)
	{
		BlockInfo* p = this->freelist_literal;
		while (p != nullptr)
		{
			BlockInfo *np = p->next;
			delete p;
			p = np;
		}
	}
	if (this->freelist_predicate != nullptr)
	{
		BlockInfo* p = this->freelist_predicate;
		while (p != nullptr)
		{
			BlockInfo *np = p->next;
			delete p;
			p = np;
		}
	}
}

// this is used for checkpoint, we must ensure that modification is written to disk,
// so flush() is a must
bool Database::save()
{
	this->kvstore->flush();
	this->saveDBInfoFile();
	this->saveIDinfo();

	this->stringindex->flush();
	this->clear_update_log();

	return true;
}

void Database::clear()
{
	delete[] this->pre2num;
	this->pre2num = NULL;
	delete[] this->pre2sub;
	this->pre2sub = NULL;
	delete[] this->pre2obj;
	this->pre2obj = NULL;
	delete this->entity_buffer;
	this->entity_buffer = NULL;
	delete this->literal_buffer;
	this->literal_buffer = NULL;

	delete this->kvstore;
	this->kvstore = NULL;
	delete this->stringindex;
	this->stringindex = NULL;
}

string Database::getName()
{
	return this->name;
}

TYPE_TRIPLE_NUM
Database::getTripleNum()
{
	return this->triples_num;
}

TYPE_ENTITY_LITERAL_ID Database::getEntityNum()
{
	return this->entity_num;
}

TYPE_ENTITY_LITERAL_ID Database::getLiteralNum()
{
	return this->literal_num;
}

TYPE_ENTITY_LITERAL_ID Database::getSubNum()
{
	return this->sub_num;
}

TYPE_PREDICATE_ID Database::getPreNum()
{
	return this->pre_num;
}

KVstore *Database::getKVstore()
{
	return this->kvstore;
}

StringIndex *Database::getStringIndex()
{
	return this->stringindex;
}

QueryCache *Database::getQueryCache()
{
	return this->query_cache;
}

TYPE_TRIPLE_NUM *Database::getpre2num()
{
	return this->pre2num;
}

TYPE_TRIPLE_NUM *Database::getpre2sub()
{
	return this->pre2sub;
}

TYPE_TRIPLE_NUM *Database::getpre2obj()
{
	return this->pre2obj;
}

TYPE_ENTITY_LITERAL_ID &Database::getlimitID_literal()
{
	return this->limitID_literal;
}

TYPE_ENTITY_LITERAL_ID &Database::getlimitID_entity()
{
	return this->limitID_entity;
}

TYPE_PREDICATE_ID &Database::getlimitID_predicate()
{
	return this->limitID_predicate;
}

void Database::export_db(FILE *fp)
{
	unsigned *id_list = NULL;
	unsigned id_list_len = 0;

	for (TYPE_PREDICATE_ID i = 0; i < this->limitID_predicate; ++i)
	{
		TYPE_PREDICATE_ID pid = i;
		string p = this->kvstore->getPredicateByID(pid);
		string pre = Util::node2string(p.c_str());
		this->kvstore->getsubIDobjIDlistBypreID(pid, id_list, id_list_len, true, nullptr);
		for (unsigned j = 0; j < id_list_len; j += 2)
		{
			string s = this->kvstore->getEntityByID(id_list[j]);
			string sub = Util::node2string(s.c_str());
			string o;
			if (id_list[j + 1] >= Util::LITERAL_FIRST_ID)
				o = this->kvstore->getLiteralByID(id_list[j + 1]);
			else
				o = this->kvstore->getEntityByID(id_list[j + 1]);
			string obj = Util::node2string(o.c_str());
			string record = sub + "\t" + pre + "\t" + obj + ".\n";
			fprintf(fp, "%s", record.c_str());
		}
		delete[] id_list;
	}
}

int Database::query(const string _query, ResultSet &_result_set, FILE *_fp, bool update_flag, bool export_flag, shared_ptr<Transaction> txn)
{
	if (_result_set.ansNum > 0) 
		_result_set.release();
	string dictionary_store_path = this->store_path + "/dictionary.dc";

	this->stringindex->SetTrie(this->kvstore->getTrie());
	GeneralEvaluation general_evaluation(this->kvstore, this->stringindex, this->query_cache, this->csr,
										 this->pre2num, this->pre2sub, this->pre2obj, this->triples_num,
										 this->limitID_predicate, this->limitID_literal, this->limitID_entity, txn, this->getfreelist_entity(), this->getentity_num());
	if (txn != nullptr)
		SLOG_CORE("query in transaction............................................");
	long tv_begin = Util::get_cur_time();

	// this->query_parse_lock.lock();
	bool parse_ret = false;
	try
	{
		/* code */
		parse_ret = general_evaluation.parseQuery(_query);
	}
	catch (const std::runtime_error &e2)
	{
		SLOG_ERROR("catch run_time error exception");
		throw std::runtime_error(e2.what());
		SLOG_ERROR(e2.what() << "\n");
	}
	catch (const std::exception &e)
	{
		SLOG_ERROR(e.what() << '\n');
	}

	// this->query_parse_lock.unlock();
	if (!parse_ret)
		return -101;
	long tv_parse = Util::get_cur_time();
	SLOG_CORE("after Parsing, used " << (tv_parse - tv_begin) << "ms.");

	// for select, -100 by default, -101 means error
	// for update, non-negative means true(and the num is updated triples num), -1 means error
	int success_num = -100;
	bool need_output_answer = false;

	// Query
	if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Not_Update)
	{
		// lock_guard<mutex> (this->update_lock);  //when quit this scope the lock will be released
		// unique_guard<mutex> updateLCK(this->update_lock);
		// if(!unique_guard.try_lock())
		// BETTER: use timed lock
		if (txn == nullptr && pthread_rwlock_tryrdlock(&(this->update_lock)) != 0)
		{
			return -101;
		}
		if (txn == nullptr)
			SLOG_CORE("read priviledge of update lock acquired");

		// copy the string index for each query thread
		// StringIndex tmpsi = *this->stringindex;
		// tmpsi.emptyBuffer();
		// general_evaluation.setStringIndexPointer(&tmpsi);

		//	this->debug_lock.lock();
		if (export_flag)
		{
			general_evaluation.fp = _fp;
			general_evaluation.export_flag = export_flag;
		}

		long t1 = Util::get_cur_time();
		bool query_ret = general_evaluation.doQuery();
		long t2 = Util::get_cur_time();
		SLOG_CORE("GeneralEvaluation::doQuery used " << (t2 - t1) << "ms.");

		if (!query_ret)
		{
			success_num = -101;
		}
		//	this->debug_lock.unlock();

		long tv_bfget = Util::get_cur_time();
		// NOTICE: this lock lock ensures that StringIndex is visited sequentially
		// this->getFinalResult_lock.lock();
		general_evaluation.getFinalResult(_result_set);
		// this->getFinalResult_lock.unlock();
		long tv_afget = Util::get_cur_time();
		SLOG_CORE("during getFinalResult, used " << (tv_afget - tv_bfget) << "ms.");

		if (_fp != NULL)
			need_output_answer = true;
		// general_evaluation.setNeedOutputAnswer();

		// tmpsi.clear();
		if (txn == nullptr)
			pthread_rwlock_unlock(&(this->update_lock));
	}
	// Update
	else
	{
		if (update_flag == 0)
		{
			// if update_flag == 0, means no privilege to do update query, so we throw an error.
			string exception_msg = "no update prvilege, update query failed.";
			SLOG_CORE(exception_msg);
			throw exception_msg;
		}
#ifdef ONLY_READ
		cout << "this database is only read";
		// invalid query because updates are not allowed in ONLY_READ mode
		return -101;
#endif
		if (txn == nullptr && pthread_rwlock_trywrlock(&(this->update_lock)) != 0)
		{
			SLOG_CORE("unable to write lock");
			return -101;
		}
		if (txn == nullptr)
			SLOG_CORE("write priviledge of update lock acquired");

		success_num = 0;
		TripleWithObjType *update_triple = NULL;
		TYPE_TRIPLE_NUM update_triple_num = 0;
		/*if (trie == NULL)
		{
			trie = new Trie;
			string dictionary_path = this->store_path + "/dictionary.dc";
			if (!trie->LoadTrie(dictionary_path))
			{
				exit(0);
			}
			trie->LoadDictionary();
		}*/

		if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Insert_Data || general_evaluation.getQueryTree().getUpdateType() == QueryTree::Delete_Data)
		{
			GroupPattern &update_pattern = general_evaluation.getQueryTree().getUpdateType() == QueryTree::Insert_Data ? general_evaluation.getQueryTree().getInsertPatterns() : general_evaluation.getQueryTree().getDeletePatterns();

			update_triple_num = update_pattern.sub_group_pattern.size();
			update_triple = new TripleWithObjType[update_triple_num];

			for (TYPE_TRIPLE_NUM i = 0; i < update_triple_num; i++)
				if (update_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Pattern_type)
				{
					TripleWithObjType::ObjectType object_type = TripleWithObjType::None;
					if (update_pattern.sub_group_pattern[i].pattern.object.value[0] == '<')
						object_type = TripleWithObjType::Entity;
					else
						object_type = TripleWithObjType::Literal;

					update_triple[i] = TripleWithObjType(update_pattern.sub_group_pattern[i].pattern.subject.value,
														 update_pattern.sub_group_pattern[i].pattern.predicate.value,
														 update_pattern.sub_group_pattern[i].pattern.object.value, object_type);

					// Compress
					// update_triple[i] = trie->Compress(update_triple[i], Trie::QUERYMODE);
				}
				else
				{
					if (txn == nullptr)
					{
						pthread_rwlock_unlock(&(this->update_lock));
						throw "Database::query failed";
					}
				}

			if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Insert_Data)
			{
				// success_num = insert(update_triple, update_triple_num, false, txn);
				success_num = batch_insert(update_triple, update_triple_num, false, txn);
			}
			else if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Delete_Data)
			{
				// success_num = remove(update_triple, update_triple_num, false, txn);
				success_num = batch_remove(update_triple, update_triple_num, false, txn);
			}
		}
		else if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Delete_Where || general_evaluation.getQueryTree().getUpdateType() == QueryTree::Insert_Clause ||
				 general_evaluation.getQueryTree().getUpdateType() == QueryTree::Delete_Clause || general_evaluation.getQueryTree().getUpdateType() == QueryTree::Modify_Clause)
		{
			general_evaluation.getQueryTree().setProjectionAsterisk();
			general_evaluation.doQuery();

			if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Delete_Where || general_evaluation.getQueryTree().getUpdateType() == QueryTree::Delete_Clause || general_evaluation.getQueryTree().getUpdateType() == QueryTree::Modify_Clause)
			{
				general_evaluation.prepareUpdateTriple(general_evaluation.getQueryTree().getDeletePatterns(), update_triple, update_triple_num);
				// for(int i = 0; i < update_triple_num; i++)
				//{
				// update_triple[i] = trie->Compress(update_triple[i], Trie::QUERYMODE);
				//}
				// success_num = remove(update_triple, update_triple_num, false, txn);
				success_num = batch_remove(update_triple, update_triple_num, false, txn);
			}
			if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Insert_Clause || general_evaluation.getQueryTree().getUpdateType() == QueryTree::Modify_Clause)
			{
				general_evaluation.prepareUpdateTriple(general_evaluation.getQueryTree().getInsertPatterns(), update_triple, update_triple_num);
				// for(int i = 0; i < update_triple_num; i++)
				//{
				// update_triple[i] = trie->Compress(update_triple[i], Trie::QUERYMODE);
				//}
				// success_num = insert(update_triple, update_triple_num, false, txn);
				success_num = batch_insert(update_triple, update_triple_num, false, txn);
			}
		}

		general_evaluation.releaseResult();
		delete[] update_triple;

		// NOTICE: maybe no updates are really done!
		if (success_num > 0 && query_cache != nullptr)
		{
			this->query_cache->clear();
			SLOG_CORE("QueryCache cleared");
			if (general_evaluation.getQueryTree().getUpdateType() != QueryTree::Not_Update)
			{
				this->saveStatisticsInfoFile();
				this->saveDBInfoFile();
			}
		}
		if (txn == nullptr)
			pthread_rwlock_unlock(&(this->update_lock));
	}

	long tv_final = Util::get_cur_time();
	SLOG_CORE("Query time used (minus parsing): " << tv_final - tv_parse << "ms.");
	SLOG_CORE("Total time used: " << (tv_final - tv_begin) << "ms.");
	// if (general_evaluation.needOutputAnswer())
	//  if(export_flag)
	//  {
	if (need_output_answer)
	{
		long long ans_num = max((long long)_result_set.ansNum - _result_set.output_offset, 0LL);
		if (_result_set.output_limit != -1)
			ans_num = min(ans_num, (long long)_result_set.output_limit);
		SLOG_CORE("There has answer: " << ans_num );
		if (_fp == stdout)
		{
			SLOG_CORE("final result is : ");
			_result_set.prettyPrint();
		}
		else
		{
			_result_set.output(_fp);
			fflush(_fp); // to empty the output buffer in C (fflush(stdin) not work in GCC)
		}
	}
	// }

#ifdef DEBUG
	cout << "query success_num: " << success_num << endl;
#endif

	return success_num;
}

// NOTICE+QUERY:to save memory for large cases, we can consider building one tree at a time(then release)
// Or read the rdf file on separate segments
// WARN:the ID type is int, and entity/literal are just separated by a limit
// which means that entity num <= 10^9 literal num <= 10^9 predicate num <= 2*10^9
// If we use unsigned as type, then max triple can be 10^9(edge case)
// If we use long long, no more problem, but wasteful
// Or we can consider divide entity and literal totally
// In distributed gStore, each machine's graph should be based on unique encoding IDs,
// and require that triples in each graph no more than a limit(maybe 10^9)

void Database::InitEmptyDB() {
    this->resetIDinfo();
	Util::create_dir(this->store_path);

	string kv_store_path = store_path + "/kv_store";
	Util::create_dir(kv_store_path);

	string stringindex_store_path = store_path + "/stringindex_store";
	Util::create_dir(stringindex_store_path);

	string update_log_path = this->store_path + '/' + this->update_log;
	Util::create_file(update_log_path);
	update_log_since_backup = this->store_path + '/' + this->update_log_since_backup;
	Util::create_file(update_log_since_backup);

	string error_log = this->store_path + "/parse_error.log";
	Util::create_file(error_log);
	SLOG_CORE("Error log file:" << error_log);
}

void Database::BuildEmptyKVstore() {
    (this->kvstore)->open_entity2id(KVstore::CREATE_MODE);
    (this->kvstore)->open_id2entity(KVstore::CREATE_MODE);
    (this->kvstore)->open_predicate2id(KVstore::CREATE_MODE);
    (this->kvstore)->open_id2predicate(KVstore::CREATE_MODE);
    (this->kvstore)->open_literal2id(KVstore::CREATE_MODE);
    (this->kvstore)->open_id2literal(KVstore::CREATE_MODE);
    this->kvstore->close_entity2id();
    this->kvstore->close_id2entity();
    this->kvstore->close_literal2id();
    this->kvstore->close_id2literal();
    this->kvstore->close_predicate2id();
    this->kvstore->close_id2predicate();
    build_s2xx(nullptr);
    build_o2xx(nullptr);
    build_p2xx(nullptr);
}

bool Database::BuildEmptyDB() {
    InitEmptyDB();
    BuildEmptyKVstore();
    string _six_tuples_file = this->getSixTuplesFile();
	ofstream _six_tuples_fout(_six_tuples_file.c_str());
    _six_tuples_fout.close();
    this->stringindex->save(*this->kvstore);
    delete this->kvstore;
	this->kvstore = NULL;

	SLOG_CORE("Finish sub2id pre2id obj2id");
	SLOG_CORE("TripleNum is " << this->triples_num);
	SLOG_CORE("EntityNum is " << this->entity_num);
	SLOG_CORE("PreNum is " << this->pre_num);
	SLOG_CORE("LiteralNum is " << this->literal_num);

	if (!(this->saveDBInfoFile())) return false;
    if (!(this->saveStatisticsInfoFile())) {
		SLOG_ERROR("the statistics info file of db saved failure!");
        return false;
    }
	this->writeIDinfo();
	this->initIDinfo();
	return true;
}

bool Database::build(const string &_rdf_file, Socket &socket)
{
	string ret = Util::getExactPath(_rdf_file.c_str());
	long tv_build_begin = Util::get_cur_time();
	InitEmptyDB();
	string msg = "begin encode RDF from : " + ret + " ...";
	SLOG_CORE(msg);
	string resJson = CreateJson(1, "building", msg);
	socket.send(resJson);
	string error_log = this->store_path + "/parse_error.log";
	if (!this->encodeRDF_new(ret, error_log))
	{
		return false;
	}
	msg = "finish encode.";
	SLOG_CORE(msg);
	resJson = CreateJson(1, "building", msg);
	socket.send(resJson);

	delete this->kvstore;
	this->kvstore = NULL;

	long tv_build_end = Util::get_cur_time();

	msg = "after build, used " + to_string(tv_build_end - tv_build_begin) + "ms.\n";
	msg = msg + "finish build VS-Tree.\n";
	msg = msg + "finish sub2id pre2id obj2id\n";
	msg = msg + "tripleNum is " + to_string(this->triples_num) + "\n";
	msg = msg + "entityNum is " + to_string(this->entity_num) + "\n";
	msg = msg + "preNum is " + to_string(this->pre_num) + "\n";
	msg = msg + "literalNum is " + to_string(this->literal_num) + "\n";
	SLOG_CORE(msg);
	resJson = CreateJson(1, "building", msg);
	socket.send(resJson);
	this->saveDBInfoFile();
	this->writeIDinfo();
	this->initIDinfo();

	return true;
}

bool Database::build(const string &_rdf_file)
{
	SLOG_CORE("---------Begin to Build Database `" << name << "`---------");
	// NOTICE: it is not necessary to use multiple threads here, because some process may rely on others
	// In addition, the memory is a bootleneck and it is dangerous to build serveral indices at a time
	// For example, if we build id2string indices using different threads, they
	// all have to scan the dataset and only use a part of the data, which may be costly
	// Besides, build process is already very fast, able to build freebase in 12h

	// manage the id for a new database
	string ret = Util::getExactPath(_rdf_file.c_str());
	InitEmptyDB();
	string error_log = this->store_path + "/parse_error.log";
	FILE *fp = fopen(error_log.c_str(), "a");
	string log_msg = "Info " + Util::get_date_time() + " build parser info, file path " + ret + "\n";
	fputs(log_msg.c_str(), fp);
	fclose(fp);
	SLOG_CORE("Begin encode RDF from : " << ret << " ...");

	// BETTER+TODO:now require that dataset size < memory
	// to support really larger datasets, divide and insert into B+ tree and VStree
	//(read the value, add list and set; update the signature, remove and reinsert)
	// the query process is nearly the same
	// QUERY:build each index in one thread to speed up, but the sorting case?
	// one sorting order for several indexes

	// to be switched to new encodeRDF method.
	//    this->encodeRDF(ret);
	if (!this->encodeRDF_new(ret, error_log)) //<-- this->kvstore->id2* trees are closed
	{
		return false;
	}
	SLOG_CORE("finish encode.");

	// this->kvstore->flush();
	delete this->kvstore;
	this->kvstore = NULL;
	// sync();
	// this->kvstore->release();

	SLOG_CORE("Finish sub2id pre2id obj2id");
	SLOG_CORE("TripleNum is " << this->triples_num);
	SLOG_CORE("EntityNum is " << this->entity_num);
	SLOG_CORE("PreNum is " << this->pre_num);
	SLOG_CORE("LiteralNum is " << this->literal_num);

	// TODO: use fopen w+ to remove signature.binary file
	// string cmd = "rm -rf " + _entry_file;
	// system(cmd.c_str());

	// this->if_loaded = true;
	this->saveDBInfoFile();
	this->writeIDinfo();
	this->initIDinfo();
	return true;
}

// root Path of this DB + sixTuplesFile
string
Database::getSixTuplesFile()
{
	return this->getStorePath() + "/" + this->six_tuples_file;
}

// root Path of this DB + DBInfoFile
string
Database::getDBInfoFile()
{
	return this->getStorePath() + "/" + this->db_info_file;
}

void Database::setTypePredicateName(string &names)
{
	this->type_predicate_name = names;
}

bool Database::checkIsTypePredicate(string &predicate)
{
	vector<string> names;
	Util::split(this->type_predicate_name, "@@", names);
	for (size_t i = 0; i < names.size(); i++)
	{
		if (Util::iscontain(predicate, names[i]))
		{
			return true;
		}
	}
	return false;
}

string
Database::getIDTuplesFile()
{
	return this->getStorePath() + "/" + this->id_tuples_file;
}

bool Database::saveDBInfoFile()
{
	FILE *filePtr = fopen(this->getDBInfoFile().c_str(), "wb");
	if (filePtr == NULL)
	{
		SLOG_ERROR("error, can not create db info file. @Database::saveDBInfoFile");
		return false;
	}

	fseek(filePtr, 0, SEEK_SET);

	fwrite(&this->triples_num, sizeof(TYPE_TRIPLE_NUM), 1, filePtr);
	fwrite(&this->entity_num, sizeof(TYPE_ENTITY_LITERAL_ID), 1, filePtr);
	fwrite(&this->sub_num, sizeof(TYPE_ENTITY_LITERAL_ID), 1, filePtr);
	fwrite(&this->pre_num, sizeof(TYPE_PREDICATE_ID), 1, filePtr);
	fwrite(&this->literal_num, sizeof(TYPE_ENTITY_LITERAL_ID), 1, filePtr);
	fwrite(&this->encode_mode, sizeof(int), 1, filePtr);

	Util::Csync(filePtr);
	fclose(filePtr);

	return true;
}

bool Database::loadDBInfoFile()
{
	FILE *filePtr = fopen(this->getDBInfoFile().c_str(), "rb");

	if (filePtr == NULL)
	{
		SLOG_ERROR("error, can not open db info file. @Database::loadDBInfoFile");
		return false;
	}

	fseek(filePtr, 0, SEEK_SET);

	fread(&this->triples_num, sizeof(TYPE_TRIPLE_NUM), 1, filePtr);
	fread(&this->entity_num, sizeof(TYPE_ENTITY_LITERAL_ID), 1, filePtr);
	fread(&this->sub_num, sizeof(TYPE_ENTITY_LITERAL_ID), 1, filePtr);
	fread(&this->pre_num, sizeof(TYPE_PREDICATE_ID), 1, filePtr);
	fread(&this->literal_num, sizeof(TYPE_ENTITY_LITERAL_ID), 1, filePtr);
	fread(&this->encode_mode, sizeof(int), 1, filePtr);
	fclose(filePtr);

	return true;
}

string
Database::getStorePath()
{
	return this->store_path;
}

// check whether the relative 3-tuples exist usually, through sp2olist
bool Database::exist_triple(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	unsigned *_objidlist = NULL;
	unsigned _list_len = 0;
	// get exclusive before update!
	(this->kvstore)->getobjIDlistBysubIDpreID(_sub_id, _pre_id, _objidlist, _list_len, true, txn);

	bool is_exist = false;
	if (Util::bsearch_int_uporder(_obj_id, _objidlist, _list_len) != INVALID)
	// if (Util::bsearch_int_uporder(_obj_id, _objidlist, _list_len) != -1)
	{
		is_exist = true;
	}
	delete[] _objidlist;

	return is_exist;
}

bool Database::exist_triple(const TripleWithObjType &_triple, shared_ptr<Transaction> txn)
{
	int sub_id = this->kvstore->getIDByEntity(_triple.getSubject());
	if (sub_id == -1)
	{
		return false;
	}

	int pre_id = this->kvstore->getIDByPredicate(_triple.getPredicate());
	if (pre_id == -1)
	{
		return false;
	}

	int obj_id = -1;
	if (_triple.isObjEntity())
	{
		obj_id = this->kvstore->getIDByEntity(_triple.getObject());
	}
	else if (_triple.isObjLiteral())
	{
		obj_id = this->kvstore->getIDByLiteral(_triple.getObject());
	}
	if (obj_id == -1)
	{
		return false;
	}

	return exist_triple(sub_id, pre_id, obj_id, txn);
}

// NOTICE: all constants are transfered to ids in memory
// this maybe not ok when size is too large!
bool Database::encodeRDF_new(const string _rdf_file)
{
#ifdef DEBUG
	Util::logging("In encodeRDF_new");
#endif

	// TYPE_ENTITY_LITERAL_ID** _p_id_tuples = NULL;
	ID_TUPLE *_p_id_tuples = NULL;
	// TYPE_TRIPLE_NUM _id_tuples_max = 0;

	long t1 = Util::get_cur_time();

	// NOTICE: in encode process, we should not divide ID of entity and literal totally apart, i.e. entity is a system
	// while literal is another system
	// The reason is that if we divide entity and literal, then in triple_array and final_result we can not decide a given
	// ID is entity or not
	//(one way is to add a more structure to tell us which is entity, but this is costly)

	// map sub2id, pre2id, entity/literal in obj2id, store in kvstore, encode RDF data into signature
	if (!this->sub2id_pre2id_obj2id_RDFintoSignature(_rdf_file))
	{
		return false;
	}

	// TODO+BETTER:after encode, we can know the exact entity num, so we can decide if our system can run this dataset
	// based on the current available memory(need a memory manager globally)
	// If unbale to run, should exit and give a prompt
	// User can modify the config file to run anyway, but gStore will not ensure correctness
	// What is more, in load process, we also need to decide if gStore can run it
	//
	// TODO+BETTER: a global ID manager module, should be based on type template
	// this can be used in vstree, storage and Database

	long t2 = Util::get_cur_time();
	SLOG_CORE("after encode, used " << (t2 - t1) << "ms.");

	// build stringindex before this->kvstore->id2* trees are closed
	this->stringindex->setNum(StringIndexFile::Entity, this->entity_num);
	this->stringindex->setNum(StringIndexFile::Literal, this->literal_num);
	this->stringindex->setNum(StringIndexFile::Predicate, this->pre_num);
	this->stringindex->save(*this->kvstore);
	// NOTICE: the string index can be parallized with readIDTuples and others
	// However, we should read and build otehr indices only after the 6 trees and string index closed
	//(to save memory)

	long t3 = Util::get_cur_time();
	SLOG_CORE("after stringindex, used " << (t3 - t2) << "ms.");


	// NOTICE:close these trees now to save memory
	this->kvstore->close_entity2id();
	this->kvstore->close_id2entity();
	this->kvstore->close_literal2id();
	this->kvstore->close_id2literal();
	this->kvstore->close_predicate2id();
	this->kvstore->close_id2predicate();

	long t4 = Util::get_cur_time();
	SLOG_CORE("id2string and string2id closed, used " << (t4 - t3) << "ms.");

	// after closing the 6 trees, read the id tuples again, and remove the file     given num, a dimension,return a pointer
	// NOTICE: the file can also be used for debugging, and a program can start just from the id tuples file
	//(if copy the 6 id2string trees, no need to parse each time)
	this->readIDTuples(_p_id_tuples);

	// NOTICE: we can also build the signature when we are reading triples, and
	// update to the corresponding position in the signature file
	// However, this may be costly due to frequent read/write

	long t5 = Util::get_cur_time();
	SLOG_CORE("id tuples read, used " << (t5 - t4) << "ms.");

	// TODO: how to set the buffer of trees is a big question, fully utilize the availiable memory

	// this->kvstore->build_subID2values(_p_id_tuples, this->triples_num);
	this->build_s2xx(_p_id_tuples);

	long t6 = Util::get_cur_time();
	SLOG_CORE("after s2xx, used " << (t6 - t5) << "ms.");

	// this->kvstore->build_objID2values(_p_id_tuples, this->triples_num);
	this->build_o2xx(_p_id_tuples);

	long t7 = Util::get_cur_time();
	SLOG_CORE("after o2xx, used " << (t7 - t6) << "ms.");

	// this->kvstore->build_preID2values(_p_id_tuples, this->triples_num);
	this->build_p2xx(_p_id_tuples);

	long t8 = Util::get_cur_time();
	SLOG_CORE("after p2xx, used " << (t8 - t7) << "ms.");

	// WARN:we must free the memory for id_tuples array
	delete[] _p_id_tuples;

	// for (TYPE_TRIPLE_NUM i = 0; i < this->triples_num; ++i)
	//{
	// delete[] _p_id_tuples[i];
	//}
	// delete[] _p_id_tuples;

	// NOTICE: we should build vstree after id tuples are freed(to save memory)

	bool flag = this->saveDBInfoFile();
	if (!flag)
	{
		return false;
	}

	long t9 = Util::get_cur_time();
	SLOG_CORE("db info saved, used " << (t9 - t8) << "ms.");

	// Util::logging("finish encodeRDF_new");

	return true;
}

bool Database::encodeRDF_new(const string _rdf_file, const string _error_log)
{
#ifdef DEBUG
	Util::logging("In encodeRDF_new");
#endif

	// TYPE_ENTITY_LITERAL_ID** _p_id_tuples = NULL;
	ID_TUPLE *_p_id_tuples = NULL;
	// TYPE_TRIPLE_NUM _id_tuples_max = 0;

	// long t1 = Util::get_cur_time();

	// NOTICE: in encode process, we should not divide ID of entity and literal totally apart, i.e. entity is a system
	// while literal is another system
	// The reason is that if we divide entity and literal, then in triple_array and final_result we can not decide a given
	// ID is entity or not
	//(one way is to add a more structure to tell us which is entity, but this is costly)

	// map sub2id, pre2id, entity/literal in obj2id, store in kvstore, encode RDF data into signature
	if (!this->sub2id_pre2id_obj2id_RDFintoSignature(_rdf_file, _error_log))
	{
		return false;
	}

	// TODO+BETTER:after encode, we can know the exact entity num, so we can decide if our system can run this dataset
	// based on the current available memory(need a memory manager globally)
	// If unbale to run, should exit and give a prompt
	// User can modify the config file to run anyway, but gStore will not ensure correctness
	// What is more, in load process, we also need to decide if gStore can run it
	//
	// TODO+BETTER: a global ID manager module, should be based on type template
	// this can be used in vstree, storage and Database

	long t2 = Util::get_cur_time();

	SLOG_CORE("Begin to save StringIndex ......");
	// build stringindex before this->kvstore->id2* trees are closed
	this->stringindex->setNum(StringIndexFile::Entity, this->entity_num);
	this->stringindex->setNum(StringIndexFile::Literal, this->literal_num);
	this->stringindex->setNum(StringIndexFile::Predicate, this->pre_num);
	this->stringindex->save(*this->kvstore);
	// NOTICE: the string index can be parallized with readIDTuples and others
	// However, we should read and build otehr indices only after the 6 trees and string index closed
	//(to save memory)

	long t3 = Util::get_cur_time();
	SLOG_CORE("Saving StringIndex, used " << (t3 - t2) << "ms.");

	// NOTICE:close these trees now to save memory
	SLOG_CORE("Begin to save id2string and string2id ......");
	this->kvstore->close_entity2id();
	this->kvstore->close_id2entity();
	this->kvstore->close_literal2id();
	this->kvstore->close_id2literal();
	this->kvstore->close_predicate2id();
	this->kvstore->close_id2predicate();
	long t4 = Util::get_cur_time();
	SLOG_CORE("Finish saving id2string and string2id, used " << (t4 - t3) << "ms.");

	// after closing the 6 trees, read the id tuples again, and remove the file     given num, a dimension,return a pointer
	// NOTICE: the file can also be used for debugging, and a program can start just from the id tuples file
	//(if copy the 6 id2string trees, no need to parse each time)
	this->readIDTuples(_p_id_tuples);

	// NOTICE: we can also build the signature when we are reading triples, and
	// update to the corresponding position in the signature file
	// However, this may be costly due to frequent read/write

	long t5 = Util::get_cur_time();
	SLOG_CORE("id tuples read, used " << (t5 - t4) << "ms.");

	// TODO: how to set the buffer of trees is a big question, fully utilize the availiable memory

	SLOG_CORE("Begin to build s2values ......");
	this->build_s2xx(_p_id_tuples);
	long t6 = Util::get_cur_time();
	SLOG_CORE("Finish building s2values, used " << (t6 - t5) << "ms.");

	SLOG_CORE("Begin to build o2values ......");
	this->build_o2xx(_p_id_tuples);
	long t7 = Util::get_cur_time();
	SLOG_CORE("Finish building o2values, used " << (t7 - t6) << "ms.");

	SLOG_CORE("Begin to build p2values ......");
	this->build_p2xx(_p_id_tuples);
	long t8 = Util::get_cur_time();
	SLOG_CORE("Finish building p2values, used " << (t8 - t7) << "ms.");

	// WARN:we must free the memory for id_tuples array
	delete[] _p_id_tuples;

	bool flag = this->saveDBInfoFile();
	if (!flag)
	{
		return false;
	}
	long t9 = Util::get_cur_time();
	SLOG_CORE("db info saved, used " << (t9 - t8) << "ms.");

	flag = this->saveStatisticsInfoFile();
	if (!flag)
	{
		SLOG_ERROR("the statistics info file of db saved failure!");
	}

	return true;
}

void Database::readIDTuples(ID_TUPLE *&_p_id_tuples)
{
	_p_id_tuples = NULL;
	string fname = this->getIDTuplesFile();
	FILE *fp = fopen(fname.c_str(), "rb");
	if (fp == NULL)
	{
		SLOG_ERROR("error in Database::readIDTuples() -- unable to open file " << fname);
		return;
	}

	// NOTICE: avoid to break the unsigned limit, size_t is used in Linux C
	// size_t means long unsigned int in 64-bit machine
	// unsigned long total_num = this->triples_num * 3;
	//_p_id_tuples = new TYPE_ENTITY_LITERAL_ID[total_num];
	_p_id_tuples = new ID_TUPLE[this->triples_num];
	fread(_p_id_tuples, sizeof(ID_TUPLE), this->triples_num, fp);

	fclose(fp);
	// NOTICE: choose to empty the file or not
	Util::empty_file(fname.c_str());

	// return NULL;
}

void Database::build_s2xx(ID_TUPLE *_p_id_tuples)
{
	// NOTICE: STL sort() is generally fatser than C qsort, especially when qsort is very slow
	// STL sort() not only use qsort algorithm, it can also choose heap-sort method
#ifndef PARALLEL_SORT
	sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::spo_cmp_idtuple);
#else
	omp_set_num_threads(thread_num);
	__gnu_parallel::sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::spo_cmp_idtuple);
#endif
	// qsort(_p_id_tuples, this->triples_num, sizeof(int*), Util::_spo_cmp);

	// remove duplicates from the id tables
	TYPE_TRIPLE_NUM j = 1;
	// TODO: should output triples_num without removing duplicates for reference, or keep a unique_triples_num separately?
	for (TYPE_TRIPLE_NUM i = 1; i < this->triples_num; ++i)
	{
		if (!Util::equal(_p_id_tuples[i], _p_id_tuples[i - 1]))
		{
			_p_id_tuples[j] = _p_id_tuples[i];
			++j;
		}
	}
	if (j < this->triples_num)
		this->triples_num = j;

	this->kvstore->build_subID2values(_p_id_tuples, this->triples_num, this->entity_num);
}

void Database::build_o2xx(ID_TUPLE *_p_id_tuples)
{
#ifndef PARALLEL_SORT
	sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::ops_cmp_idtuple);
#else
	omp_set_num_threads(thread_num);
	__gnu_parallel::sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::ops_cmp_idtuple);
#endif
	// qsort(_p_id_tuples, this->triples_num, sizeof(int*), Util::_ops_cmp);
	this->kvstore->build_objID2values(_p_id_tuples, this->triples_num, this->entity_num, this->literal_num);
}

void Database::build_p2xx(ID_TUPLE *_p_id_tuples)
{
#ifndef PARALLEL_SORT
	sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::pso_cmp_idtuple);
#else
	omp_set_num_threads(thread_num);
	__gnu_parallel::sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::pso_cmp_idtuple);
#endif
	// qsort(_p_id_tuples, this->triples_num, sizeof(int*), Util::_pso_cmp);
	this->kvstore->build_preID2values(_p_id_tuples, this->triples_num, this->pre_num);
}

// NOTICE:in here and there in the insert/delete, we may get the maxium tuples num first
// and so we can avoid the cost of memcpy(scan quickly or use wc -l)
// However, if use compressed RDF format, how can we do it fi not using parser?
// CONSIDER: just an estimated value is ok or use vector!!!(but vector also copy when enlarge)
// and read file line numbers are also costly!
bool Database::sub2id_pre2id_obj2id_RDFintoSignature(const string _rdf_file)
{
	// NOTICE: if we keep the id_tuples always in memory, i.e. [unsigned*] each unsigned* is [3]
	// then for freebase, there is 2.5B triples. the mmeory cost of this array is 25*10^8*3*4 + 25*10^8*8 = 50G
	//
	// So I choose not to store the id_tuples in memory in this function, but to store them in file and read again after this function
	// Notice that the most memory-costly part of building process is this function, setup 6 trees together
	// later we can read the id_tuples and stored as [num][3], only cost 25*10^8*3*4 = 30G, and later we only build one tree at a time

	string fname = this->getIDTuplesFile();
	FILE *fp = fopen(fname.c_str(), "wb");
	if (fp == NULL)
	{
		SLOG_CORE("error in Database::sub2id_pre2id_obj2id() -- unable to open file to write " << fname);
		return false;
	}
	ID_TUPLE tmp_id_tuple;
	// NOTICE: avoid to break the unsigned limit, size_t is used in Linux C
	// size_t means long unsigned int in 64-bit machine
	// fread(_p_id_tuples, sizeof(TYPE_ENTITY_LITERAL_ID), total_num, fp);

	// TYPE_TRIPLE_NUM _id_tuples_size;
	{
		// initial
		//_id_tuples_max = 10 * 1000 * 1000;
		//_p_id_tuples = new TYPE_ENTITY_LITERAL_ID*[_id_tuples_max];
		//_id_tuples_size = 0;
		this->sub_num = 0;
		this->pre_num = 0;
		this->entity_num = 0;
		this->literal_num = 0;
		this->triples_num = 0;
		(this->kvstore)->open_entity2id(KVstore::CREATE_MODE);
		(this->kvstore)->open_id2entity(KVstore::CREATE_MODE);
		(this->kvstore)->open_predicate2id(KVstore::CREATE_MODE);
		(this->kvstore)->open_id2predicate(KVstore::CREATE_MODE);
		(this->kvstore)->open_literal2id(KVstore::CREATE_MODE);
		(this->kvstore)->open_id2literal(KVstore::CREATE_MODE);
		(this->kvstore)->load_trie(KVstore::CREATE_MODE);
	}

	// Util::logging("finish initial sub2id_pre2id_obj2id");
	SLOG_CORE("finish initial sub2id_pre2id_obj2id");

	// BETTER?:close the stdio buffer sync??

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		SLOG_ERROR("sub2id&pre2id&obj2id: Fail to rdf open : " << _rdf_file);
		// exit(0);
		return false;
	}

	string _six_tuples_file = this->getSixTuplesFile();
	ofstream _six_tuples_fout(_six_tuples_file.c_str());
	if (!_six_tuples_fout)
	{
		SLOG_ERROR("sub2id&pre2id&obj2id: Fail to tuples open: " << _six_tuples_file);
		// exit(0);
		return false;
	}

	TripleWithObjType *triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];

	// don't know the number of entity
	// pre allocate entitybitset_max EntityBitSet for storing signature, double the space until the _entity_bitset is used up.
	//
	// int entitybitset_max = 10000000;  //set larger to avoid the copy cost
	// int entitybitset_max = 10000;
	// EntityBitSet** _entity_bitset = new EntityBitSet*[entitybitset_max];
	// for (int i = 0; i < entitybitset_max; i++)
	//{
	//_entity_bitset[i] = new EntityBitSet();
	//_entity_bitset[i]->reset();
	//}
	// EntityBitSet _tmp_bitset;

	//	{
	//		long begin = Util::get_cur_time();
	//		ifstream _fin0(_rdf_file.c_str());
	//		//parse a file
	//		RDFParser _parser0(_fin0);
	//
	//		// Initialize trie
	//
	//		Trie *trie = kvstore->getTrie();
	//		while (true)
	//		{
	//			int parse_triple_num = 0;
	//			_parser0.parseFile(triple_array, parse_triple_num);
	//			if (parse_triple_num == 0)
	//			{
	//				break;
	//			}
	//
	//			//Process the Triple one by one
	//			for (int i = 0; i < parse_triple_num; i++)
	//			{
	//				string t = triple_array[i].getSubject();
	//				trie->Addstring(t);
	//				t = triple_array[i].getPredicate();
	//				trie->Addstring(t);
	//				t = triple_array[i].getObject();
	//				trie->Addstring(t);
	//			}
	//		}
	//        cout<<"Add triples to Trie to prepare for BuildPrefix"<<endl;
	//		trie->BuildPrefix();
	//		cout << "BuildPrefix done. used" <<Util::get_cur_time() - begin<< endl;
	//	}

	RDFParser _parser(_fin);
	// Util::logging("==> while(true)");

	while (true)
	{
		int parse_triple_num = 0;
		_parser.parseFile(triple_array, parse_triple_num);
		SLOG_CORE("Finish rdfparser, triple_num = " << this->triples_num);

		if (parse_triple_num == 0)
		{
			break;
		}

		// Process the Triple one by one
		for (int i = 0; i < parse_triple_num; i++)
		{
			// BETTER: assume that no duplicate triples in RDF for building
			// should judge first? using exist_triple()
			// or sub triples_num in build_subID2values(judge if two neighbor triples are same)
			this->triples_num++; // NOTE: triples_num set here

			// if the _id_tuples exceeds, double the space
			// if (_id_tuples_size == _id_tuples_max)
			//{
			// TYPE_TRIPLE_NUM _new_tuples_len = _id_tuples_max * 2;
			// TYPE_ENTITY_LITERAL_ID** _new_id_tuples = new TYPE_ENTITY_LITERAL_ID*[_new_tuples_len];
			// memcpy(_new_id_tuples, _p_id_tuples, sizeof(TYPE_ENTITY_LITERAL_ID*) * _id_tuples_max);
			// delete[] _p_id_tuples;
			//_p_id_tuples = _new_id_tuples;
			//_id_tuples_max = _new_tuples_len;
			//}

			// BETTER: use 3 threads to deal with sub, obj, pre separately
			// However, the cost of new /delete threads may be high
			// We need a thread pool!

			// For subject
			// (all subject is entity, some object is entity, the other is literal)
			string _sub = triple_array[i].getSubject();
			TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_sub);
			if (_sub_id == INVALID_ENTITY_LITERAL_ID)
			// if (_sub_id == -1)
			{
				//_sub_id = this->entity_num;
				_sub_id = this->allocEntityID();
				this->sub_num++;
				// this->entity_num++;
				(this->kvstore)->setIDByEntity(_sub, _sub_id);
				(this->kvstore)->setEntityByID(_sub_id, _sub);
			}
			//  For predicate
			string _pre = triple_array[i].getPredicate();
			TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_pre);
			if (_pre_id == INVALID_PREDICATE_ID)
			// if (_pre_id == -1)
			{
				//_pre_id = this->pre_num;
				_pre_id = this->allocPredicateID();
				// this->pre_num++;
				(this->kvstore)->setIDByPredicate(_pre, _pre_id);
				(this->kvstore)->setPredicateByID(_pre_id, _pre);
			}

			//  For object
			string _obj = triple_array[i].getObject();
			// int _obj_id = -1;
			TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
			// obj is entity
			if (triple_array[i].isObjEntity())
			{
				_obj_id = (this->kvstore)->getIDByEntity(_obj);
				if (_obj_id == INVALID_ENTITY_LITERAL_ID)
				// if (_obj_id == -1)
				{
					//_obj_id = this->entity_num;
					_obj_id = this->allocEntityID();
					// this->entity_num++;
					(this->kvstore)->setIDByEntity(_obj, _obj_id);
					(this->kvstore)->setEntityByID(_obj_id, _obj);
				}
			}
			// obj is literal
			if (triple_array[i].isObjLiteral())
			{
				_obj_id = (this->kvstore)->getIDByLiteral(_obj);
				if (_obj_id == INVALID_ENTITY_LITERAL_ID)
				// if (_obj_id == -1)
				{
					//_obj_id = Util::LITERAL_FIRST_ID + (this->literal_num);
					_obj_id = this->allocLiteralID();
					// this->literal_num++;
					(this->kvstore)->setIDByLiteral(_obj, _obj_id);
					(this->kvstore)->setLiteralByID(_obj_id, _obj);
				}
			}

			// NOTICE: we assume that there is no duplicates in the dataset
			// if not, this->triple_num will be not right, and _p_id_tuples will save useless triples
			// However, we can not use exist_triple to detect duplicates here, because it is too time-costly

			//  For id_tuples
			//_p_id_tuples[_id_tuples_size] = new TYPE_ENTITY_LITERAL_ID[3];
			//_p_id_tuples[_id_tuples_size][0] = _sub_id;
			//_p_id_tuples[_id_tuples_size][1] = _pre_id;
			//_p_id_tuples[_id_tuples_size][2] = _obj_id;
			//_id_tuples_size++;
			tmp_id_tuple.subid = _sub_id;
			tmp_id_tuple.preid = _pre_id;
			tmp_id_tuple.objid = _obj_id;
			fwrite(&tmp_id_tuple, sizeof(ID_TUPLE), 1, fp);
			// fwrite(&_sub_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);
			// fwrite(&_pre_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);
			// fwrite(&_obj_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);

#ifdef DEBUG_PRECISE
			////  save six tuples
			//_six_tuples_fout << _sub_id << '\t'
			//<< _pre_id << '\t'
			//<< _obj_id << '\t'
			//<< _sub << '\t'
			//<< _pre << '\t'
			//<< _obj << endl;
#endif

			// NOTICE: the memory cost maybe too larger if combine teh below process here
			// we can do below after this function or after all B+trees are built and closed
			// and we can decide the length of signature according to entity num then
			// 1. after all b+trees: empty id_tuples and only open id2string, reload rdf file and encode(using string for entity/literal)
			//
			// 2. after this function or after all B+trees: close others and only use id_tuples to encode(no need to read file again, which is too costly)
			// not encoded with string but all IDs(not using encode for string regex matching, then this is ok!)
			// Because we encode with ID, then Signature has to be changed(and dynamic sig length)
			// use same encode strategy for entity/literal/predicate, and adjust the rate of the 3 parts according to real case
			// What is more, if the system memory is enough(precisely, the memory you want to assign to gstore - to vstree/entity_sig_array),
			// we can also set the sig length larger(which should be included in config file)

			//_entity_bitset is used up, double the space
			// if (this->entity_num >= entitybitset_max)
			//{
			// EntityBitSet** _new_entity_bitset = new EntityBitSet*[entitybitset_max * 2];
			////BETTER?:if use pointer for array, avoid the copy cost of entitybitset, but consumes more mmeory
			////if the triple size is 1-2 billion, then the memory cost will be very large!!!
			// memcpy(_new_entity_bitset, _entity_bitset, sizeof(EntityBitSet*) * entitybitset_max);
			// delete[] _entity_bitset;
			//_entity_bitset = _new_entity_bitset;

			// int tmp = entitybitset_max * 2;
			// for (int i = entitybitset_max; i < tmp; i++)
			//{
			//_entity_bitset[i] = new EntityBitSet();
			//_entity_bitset[i]->reset();
			//}

			// entitybitset_max = tmp;
			//}

			//{
			//_tmp_bitset.reset();
			// Signature::encodePredicate2Entity(_pre_id, _tmp_bitset, Util::EDGE_OUT);
			// Signature::encodeStr2Entity(_obj.c_str(), _tmp_bitset);
			//*_entity_bitset[_sub_id] |= _tmp_bitset;
			//}

			// if (triple_array[i].isObjEntity())
			//{
			//_tmp_bitset.reset();
			// Signature::encodePredicate2Entity(_pre_id, _tmp_bitset, Util::EDGE_IN);
			// Signature::encodeStr2Entity(_sub.c_str(), _tmp_bitset);
			////when 15999 error
			////WARN:id allocated can be very large while the num is not so much
			////This means that maybe the space for the _obj_id is not allocated now
			////NOTICE:we prepare the free id list in uporder and contiguous, so in build process
			////this can work well
			//*_entity_bitset[_obj_id] |= _tmp_bitset;
			//}
		}
	}
	this->kvstore->set_if_single_thread(false);

	delete[] triple_array;
	triple_array = NULL;
	_fin.close();
	_six_tuples_fout.close();
	fclose(fp);

	return true;
}

bool Database::sub2id_pre2id_obj2id_RDFintoSignature(const string _rdf_file, const string _error_log)
{
	// NOTICE: if we keep the id_tuples always in memory, i.e. [unsigned*] each unsigned* is [3]
	// then for freebase, there is 2.5B triples. the mmeory cost of this array is 25*10^8*3*4 + 25*10^8*8 = 50G
	//
	// So I choose not to store the id_tuples in memory in this function, but to store them in file and read again after this function
	// Notice that the most memory-costly part of building process is this function, setup 6 trees together
	// later we can read the id_tuples and stored as [num][3], only cost 25*10^8*3*4 = 30G, and later we only build one tree at a time

	string fname = this->getIDTuplesFile();
	FILE *fp = fopen(fname.c_str(), "wb");
	if (fp == NULL)
	{
		SLOG_ERROR("error in Database::sub2id_pre2id_obj2id() -- unable to open file to write " << fname);
		return false;
	}
	ID_TUPLE tmp_id_tuple;
	// NOTICE: avoid to break the unsigned limit, size_t is used in Linux C
	// size_t means long unsigned int in 64-bit machine
	// fread(_p_id_tuples, sizeof(TYPE_ENTITY_LITERAL_ID), total_num, fp);

	// TYPE_TRIPLE_NUM _id_tuples_size;
	{
		// initial
		//_id_tuples_max = 10 * 1000 * 1000;
		//_p_id_tuples = new TYPE_ENTITY_LITERAL_ID*[_id_tuples_max];
		//_id_tuples_size = 0;
		this->sub_num = 0;
		this->pre_num = 0;
		this->entity_num = 0;
		this->literal_num = 0;
		this->triples_num = 0;
		(this->kvstore)->open_entity2id(KVstore::CREATE_MODE);
		(this->kvstore)->open_id2entity(KVstore::CREATE_MODE);
		(this->kvstore)->open_predicate2id(KVstore::CREATE_MODE);
		(this->kvstore)->open_id2predicate(KVstore::CREATE_MODE);
		(this->kvstore)->open_literal2id(KVstore::CREATE_MODE);
		(this->kvstore)->open_id2literal(KVstore::CREATE_MODE);
		(this->kvstore)->load_trie(KVstore::CREATE_MODE);
	}

	// Util::logging("finish initial sub2id_pre2id_obj2id");
	SLOG_CORE("Finish initial sub2id_pre2id_obj2id");

	// BETTER?:close the stdio buffer sync??

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		SLOG_ERROR("sub2id&pre2id&obj2id: Fail to rdf open : " << _rdf_file);
		// exit(0);
		return false;
	}

	string _six_tuples_file = this->getSixTuplesFile();
	ofstream _six_tuples_fout(_six_tuples_file.c_str());
	if (!_six_tuples_fout)
	{
		SLOG_ERROR("sub2id&pre2id&obj2id: Fail to tuples open: " << _six_tuples_file);
		// exit(0);
		return false;
	}

	TripleWithObjType *triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];

	SLOG_CORE("Begin to build Trie ......");
	int num_lines = 0;
	{
		long begin = Util::get_cur_time();
		ifstream _fin0(_rdf_file.c_str());
		// parse a file
		RDFParser _parser0(_fin0);

		// Initialize trie

		Trie *trie = kvstore->getTrie();
		int batch_count = 0;
		while (true)
		{
			++batch_count;
			int parse_triple_num = 0;
			// TODO: make the line numbers reported inside parseFile global
			int curr_lines = _parser0.parseFile(triple_array, parse_triple_num, _error_log, num_lines);
			num_lines = curr_lines;
			if (parse_triple_num == 0)
			{
				break;
			}

			indicators::ProgressBar bar{
				indicators::option::BarWidth{50},
				indicators::option::Start{"["},
				indicators::option::Fill{"="},
				indicators::option::Lead{">"},
				indicators::option::Remainder{" "},
				indicators::option::End{"]"},
				indicators::option::PostfixText{"Build Trie for batch " + to_string(batch_count) + ", batch size = " + to_string(parse_triple_num)},
				indicators::option::ForegroundColor{indicators::Color::green},
				indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

			int bar_tmp = 0;
			int one_percent_num = parse_triple_num / 100;

			// Process the Triple one by one
			for (int i = 0; i < parse_triple_num; i++)
			{
				++bar_tmp;
				if (bar_tmp == one_percent_num)
				{
					bar.tick();
					bar_tmp = 0;
				}

				string t = triple_array[i].getSubject();
				trie->Addstring(t);
				t = triple_array[i].getPredicate();
				trie->Addstring(t);
				t = triple_array[i].getObject();
				trie->Addstring(t);
			}
			if (!bar.is_completed())
				bar.set_progress(100);
		}
		SLOG_CORE("Add triples to Trie, begin to build Prefix ......");
		trie->BuildPrefix();
		SLOG_CORE("Build Prefix and Trie done. used " << Util::get_cur_time() - begin << "ms.");
	}

	RDFParser _parser(_fin); // RDFParser is actually invoked twice, see above
	// Util::logging("==> while(true)");

	num_lines = 0;
	SLOG_CORE( "this type predicate name is " << this->type_predicate_name );
	// string type="rdf:type";
	// this->checkIsTypePredicate(type);
	this->umap.clear();

	int batch_count = 0;
	set<TYPE_ENTITY_LITERAL_ID> sub_lists;
	std::map<int, vector<ID_TUPLE>> id_tuples;
	while (true)
	{
		++batch_count;
		int parse_triple_num = 0;

		int curr_lines = _parser.parseFile(triple_array, parse_triple_num, "NULL", num_lines);
		num_lines = curr_lines;

		// {
		// 	stringstream _ss;
		// 	_ss << "finish rdfparser" << this->triples_num + parse_triple_num << endl;
		// 	//Util::logging(_ss.str());
		// 	cout << _ss.str() << endl;
		// }

		if (parse_triple_num == 0)
		{
			break;
		}
		indicators::ProgressBar bar{
			indicators::option::BarWidth{50},
			indicators::option::Start{"["},
			indicators::option::Fill{"="},
			indicators::option::Lead{">"},
			indicators::option::Remainder{" "},
			indicators::option::End{"]"},
			indicators::option::PostfixText{"Alloc ID for triple batch " + to_string(batch_count) + ", batch size = " + to_string(parse_triple_num)},
			indicators::option::ForegroundColor{indicators::Color::green},
			indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

		int bar_tmp = 0;
		int one_percent_num = parse_triple_num / 100;
		// Process the Triple one by one
		//  triples_num will eventually be set to the sum of all parse_triple_num (which seems legit)
		for (int i = 0; i < parse_triple_num; i++)
		{
			++bar_tmp;
			if (bar_tmp == one_percent_num)
			{
				bar.tick();
				bar_tmp = 0;
			}

			// BETTER: assume that no duplicate triples in RDF for building
			// should judge first? using exist_triple()
			// or sub triples_num in build_subID2values(judge if two neighbor triples are same)
			this->triples_num++; // NOTE: triples_num set here

			// if the _id_tuples exceeds, double the space
			// if (_id_tuples_size == _id_tuples_max)
			//{
			// TYPE_TRIPLE_NUM _new_tuples_len = _id_tuples_max * 2;
			// TYPE_ENTITY_LITERAL_ID** _new_id_tuples = new TYPE_ENTITY_LITERAL_ID*[_new_tuples_len];
			// memcpy(_new_id_tuples, _p_id_tuples, sizeof(TYPE_ENTITY_LITERAL_ID*) * _id_tuples_max);
			// delete[] _p_id_tuples;
			//_p_id_tuples = _new_id_tuples;
			//_id_tuples_max = _new_tuples_len;
			//}

			// BETTER: use 3 threads to deal with sub, obj, pre separately
			// However, the cost of new /delete threads may be high
			// We need a thread pool!

			// For subject
			// (all subject is entity, some object is entity, the other is literal)
			string _sub = triple_array[i].getSubject();
			TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_sub);
			if (_sub_id == INVALID_ENTITY_LITERAL_ID)
			{
				_sub_id = this->allocEntityID();
				this->sub_num++;
				(this->kvstore)->setIDByEntity(_sub, _sub_id);
				(this->kvstore)->setEntityByID(_sub_id, _sub);
				sub_lists.insert(_sub_id);
			}
			else if (sub_lists.find(_sub_id) == sub_lists.end())
			{
				this->sub_num++;
				sub_lists.insert(_sub_id);
			}
			//  For predicate
			string _pre = triple_array[i].getPredicate();
			TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_pre);
			if (_pre_id == INVALID_PREDICATE_ID)
			{
				_pre_id = this->allocPredicateID();
				(this->kvstore)->setIDByPredicate(_pre, _pre_id);
				(this->kvstore)->setPredicateByID(_pre_id, _pre);
			}

			//  For object
			string _obj = triple_array[i].getObject();
			// int _obj_id = -1;
			TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
			// obj is entity
			if (triple_array[i].isObjEntity())
			{
				_obj_id = (this->kvstore)->getIDByEntity(_obj);
				if (_obj_id == INVALID_ENTITY_LITERAL_ID)
				{
					_obj_id = this->allocEntityID();
					(this->kvstore)->setIDByEntity(_obj, _obj_id);
					(this->kvstore)->setEntityByID(_obj_id, _obj);
				}
			}
			// obj is literal
			if (triple_array[i].isObjLiteral())
			{
				_obj_id = (this->kvstore)->getIDByLiteral(_obj);
				if (_obj_id == INVALID_ENTITY_LITERAL_ID)
				{
					_obj_id = this->allocLiteralID();
					(this->kvstore)->setIDByLiteral(_obj, _obj_id);
					(this->kvstore)->setLiteralByID(_obj_id, _obj);
				}
			}

			// NOTICE: we assume that there is no duplicates in the dataset
			// if not, this->triple_num will be not right, and _p_id_tuples will save useless triples
			// However, we can not use exist_triple to detect duplicates here, because it is too time-costly

			//  For id_tuples
			//_p_id_tuples[_id_tuples_size] = new TYPE_ENTITY_LITERAL_ID[3];
			//_p_id_tuples[_id_tuples_size][0] = _sub_id;
			//_p_id_tuples[_id_tuples_size][1] = _pre_id;
			//_p_id_tuples[_id_tuples_size][2] = _obj_id;
			//_id_tuples_size++;
			tmp_id_tuple.subid = _sub_id;
			tmp_id_tuple.preid = _pre_id;
			tmp_id_tuple.objid = _obj_id;
			// when the predicat is type
			if (triple_array[i].isObjEntity() && this->checkIsTypePredicate(_pre))
				id_tuples[_obj_id].push_back(tmp_id_tuple);
			fwrite(&tmp_id_tuple, sizeof(ID_TUPLE), 1, fp);

#ifdef DEBUG_PRECISE
			////  save six tuples
			//_six_tuples_fout << _sub_id << '\t'
			//<< _pre_id << '\t'
			//<< _obj_id << '\t'
			//<< _sub << '\t'
			//<< _pre << '\t'
			//<< _obj << endl;
#endif
		}
		if (!bar.is_completed())
			bar.set_progress(100);
	}
	for (const auto& m: id_tuples)
	{
		std::string obj_v = (this->kvstore)->getEntityByID(m.first);
		if (obj_v.empty())
			continue;
		auto obj_array = m.second;
		sort(obj_array.begin(), obj_array.end(), Util::spo_cmp_idtuple);
		auto new_end = unique(obj_array.begin(), obj_array.end(), Util::equal);
		obj_array.erase(new_end, obj_array.end());
		if (obj_array.size() == 0)
			continue;
		this->umap.insert(pair<string, unsigned long long>(obj_v, obj_array.size()));
	}

	this->kvstore->set_if_single_thread(false);

	delete[] triple_array;
	triple_array = NULL;
	_fin.close();
	_six_tuples_fout.close();
	fclose(fp);

	return true;
}

bool Database::insertTriple(const TripleWithObjType &_triple, vector<unsigned> *_vertices, vector<unsigned> *_predicates, shared_ptr<Transaction> txn)
{
	// cout<<endl<<"the new triple is:"<<endl;
	// cout<<_triple.subject<<endl;
	// cout<<_triple.predicate<<endl;
	// cout<<_triple.object<<endl;

	// long tv_kv_store_begin = Util::get_cur_time();

	TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
	// if(txn != nullptr)
	// 	cout << "Update in Transaction...................................................." << endl;
	bool _is_new_sub = false;
	// if sub does not exist
	if (_sub_id == INVALID_ENTITY_LITERAL_ID)
	// if (_sub_id == -1)
	{
		_is_new_sub = true;
		_sub_id = this->allocEntityID();
		this->sub_num++;
		(this->kvstore)->setIDByEntity(_triple.subject, _sub_id);
		(this->kvstore)->setEntityByID(_sub_id, _triple.subject);

		if (_vertices != NULL)
			_vertices->push_back(_sub_id);
	}

	TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
	bool _is_new_pre = false;
	// if pre does not exist
	if (_pre_id == INVALID_PREDICATE_ID)
	// if (_pre_id == -1)
	{
		_is_new_pre = true;
		_pre_id = this->allocPredicateID();
		(this->kvstore)->setIDByPredicate(_triple.predicate, _pre_id);
		(this->kvstore)->setPredicateByID(_pre_id, _triple.predicate);

		if (_predicates != NULL)
			_predicates->push_back(_pre_id);
	}

	// object is either entity or literal
	TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
	// int _obj_id = -1;
	bool _is_new_obj = false;
	bool is_obj_entity = _triple.isObjEntity();
	if (is_obj_entity)
	{
		_obj_id = (this->kvstore)->getIDByEntity(_triple.object);

		// if (_obj_id == -1)
		if (_obj_id == INVALID_ENTITY_LITERAL_ID)
		{
			_is_new_obj = true;
			_obj_id = this->allocEntityID();
			(this->kvstore)->setIDByEntity(_triple.object, _obj_id);
			(this->kvstore)->setEntityByID(_obj_id, _triple.object);

			if (_vertices != NULL)
				_vertices->push_back(_obj_id);
		}
		string _pre = _triple.getPredicate();
		string _obj = _triple.getObject();
		if (this->checkIsTypePredicate(_pre))
		{
			auto obj_it = this->umap.find(_obj);
			if (obj_it != this->umap.end())
			{
				obj_it->second = obj_it->second + 1;
			}
			else
			{
				this->umap.insert(pair<string, unsigned long long>(_obj, 1));
			}
		}
	}
	else
	{
		_obj_id = (this->kvstore)->getIDByLiteral(_triple.object);
		// cout<<"check: "<<_obj_id<<" "<<INVALID_ENTITY_LITERAL_ID<<endl;

		// if (_obj_id == -1)
		if (_obj_id == INVALID_ENTITY_LITERAL_ID)
		{
			_is_new_obj = true;
			_obj_id = this->allocLiteralID();
			(this->kvstore)->setIDByLiteral(_triple.object, _obj_id);
			(this->kvstore)->setLiteralByID(_obj_id, _triple.object);

			if (_vertices != NULL)
				_vertices->push_back(_obj_id);
		}
	}

	if (txn != nullptr && (this->kvstore)->GetExclusiveLock(_sub_id, _pre_id, _obj_id, txn) == false)
	{
		// conflict
		// abort
		txn->SetState(TransactionState::ABORTED);
		SLOG_ERROR("getExclusiveLocks failed, Abort. TID:" << this_thread::get_id());
		return false;
	}

	// if this is not a new triple, return directly (in case of no transaction)
	if (txn == nullptr)
	{
		bool _triple_exist = false;
		if (!_is_new_sub && !_is_new_pre && !_is_new_obj)
			_triple_exist = this->exist_triple(_sub_id, _pre_id, _obj_id, txn);

		if (_triple_exist)
		{
			SLOG_WARN("this triple already exist");
			return false;
		}
		else
		{
			this->triples_num++; // NOTE: triples_num set here
		}
	}

	// update sp2o op2s s2po o2ps s2o o2s etc.
	bool ret = (this->kvstore)->updateTupleslist_insert(_sub_id, _pre_id, _obj_id, txn);
	if (txn != nullptr)
	{

		if (ret)
		{
			// cout << "WriteSetInsert......." << endl;
			txn->WriteSetInsert(IDTriple(_sub_id, _pre_id, _obj_id));
		}
		else
		{
			SLOG_ERROR("insert failed");
			txn->SetState(TransactionState::ABORTED);
			(this->kvstore)->ReleaseExclusiveLock(_sub_id, _pre_id, _obj_id, txn);
			return false;
		}
	}
	return true;
}

bool Database::removeTriple(const TripleWithObjType &_triple, vector<unsigned> *_vertices, vector<unsigned> *_predicates, shared_ptr<Transaction> txn)
{
	TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
	TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
	TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
	if (_triple.isObjEntity())
	{
		_obj_id = (this->kvstore)->getIDByEntity(_triple.object);
	}
	else
	{
		_obj_id = (this->kvstore)->getIDByLiteral(_triple.object);
	}

	// if (_sub_id == -1 || _pre_id == -1 || _obj_id == -1)
	if (_sub_id == INVALID_ENTITY_LITERAL_ID || _pre_id == INVALID_PREDICATE_ID || _obj_id == INVALID_ENTITY_LITERAL_ID)
	{
		return false;
	}
	if (txn != nullptr && (this->kvstore)->GetExclusiveLock(_sub_id, _pre_id, _obj_id, txn) == false)
	{
		// conflict
		// abort
		SLOG_ERROR("getExclusiveLocks...................... failed. TID:" << this_thread::get_id());
		txn->SetState(TransactionState::ABORTED);
		return false;
	}

	if (txn == nullptr)
	{
		bool _exist_triple = this->exist_triple(_sub_id, _pre_id, _obj_id, txn);
		if (!_exist_triple)
		{
			SLOG_ERROR("triple not exist! ");
			return false;
		}
		else
			this->triples_num--; // NOTE: triples_num set here
	}

	// remove from sp2o op2s s2po o2ps s2o o2s
	// sub2id, pre2id and obj2id will not be updated
	bool ret = (this->kvstore)->updateTupleslist_remove(_sub_id, _pre_id, _obj_id, txn);
	if (txn != nullptr)
	{
		if (ret)
			txn->WriteSetInsert(IDTriple(_sub_id, _pre_id, _obj_id));
		else
		{
			SLOG_ERROR(" updateTupleslist_remove failed ...............................................");
			txn->SetState(TransactionState::ABORTED);
			(this->kvstore)->ReleaseExclusiveLock(_sub_id, _pre_id, _obj_id, txn);
			return false;
		}
	}

	if (txn == nullptr)
	{
		int sub_degree = (this->kvstore)->getEntityDegree(_sub_id);
		// if subject become an isolated point, remove its corresponding entry
		if (sub_degree == 0)
		{
			this->kvstore->subEntityByID(_sub_id);
			this->kvstore->subIDByEntity(_triple.subject);
			this->freeEntityID(_sub_id);
			this->sub_num--;
			if (_vertices != NULL)
				_vertices->push_back(_sub_id);
		}
		else if((this->kvstore)->getEntityOutDegree(_sub_id) == 0)
		{
			this->sub_num--;
		}

		bool is_obj_entity = _triple.isObjEntity();
		int obj_degree;
		if (is_obj_entity)
		{
			obj_degree = this->kvstore->getEntityDegree(_obj_id);
			if (obj_degree == 0)
			{
				this->kvstore->subEntityByID(_obj_id);
				this->kvstore->subIDByEntity(_triple.object);
				this->freeEntityID(_obj_id);

				if (_vertices != NULL)
					_vertices->push_back(_obj_id);
			}
			string _pre = _triple.getPredicate();
			string _obj = _triple.getObject();
			if (this->checkIsTypePredicate(_pre))
			{
				auto it = this->umap.find(_obj);
				if (it != this->umap.end())
				{
					if (it->second <= 1)
						umap.erase(it);
					else
						it->second = it->second - 1;
				}
			}
		}
		else
		{
			obj_degree = this->kvstore->getLiteralDegree(_obj_id);
			if (obj_degree == 0)
			{
				this->kvstore->subLiteralByID(_obj_id);
				this->kvstore->subIDByLiteral(_triple.object);
				this->freeLiteralID(_obj_id);

				if (_vertices != NULL)
					_vertices->push_back(_obj_id);
			}
		}

		int pre_degree = this->kvstore->getPredicateDegree(_pre_id);
		if (pre_degree == 0)
		{
			this->kvstore->subPredicateByID(_pre_id);
			this->kvstore->subIDByPredicate(_triple.predicate);
			this->freePredicateID(_pre_id);
			if (_predicates != NULL)
				_predicates->push_back(_pre_id);
		}
	}

	return true;
}

bool Database::insert(std::string _rdf_file, bool _is_restore, shared_ptr<Transaction> txn)
{
	bool flag = _is_restore || this->load();
	// bool flag = this->load();
	if (!flag)
	{
		return false;
	}
	SLOG_CORE("finish loading");

	long tv_load = Util::get_cur_time();

	TYPE_TRIPLE_NUM success_num = 0;

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		SLOG_ERROR("fail to open : " << _rdf_file << ".@insert_test");
		// exit(0);
		return false;
	}

	// NOTICE+WARN:we can not load all triples into memory all at once!!!
	// the parameter in build and insert must be the same, because RDF parser also use this
	// for build process, this one can be big enough if memory permits
	// for insert/delete process, this can not be too large, otherwise too costly
	TripleWithObjType *triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
	// parse a file
	RDFParser _parser(_fin);

	// TYPE_TRIPLE_NUM triple_num = 0;
#ifdef DEBUG
	Util::logging("==> while(true)");
#endif
	while (true)
	{
		int parse_triple_num = 0;
		_parser.parseFile(triple_array, parse_triple_num);
#ifdef DEBUG
		stringstream _ss;
		// NOTICE:this is not same as others, use parse_triple_num directly
		_ss << "finish rdfparser" << parse_triple_num << endl;
		Util::logging(_ss.str());
		cout << _ss.str() << endl;
#endif
		if (parse_triple_num == 0)
		{
			break;
		}

		// Process the Triple one by one
		long tv_begin = Util::get_cur_time();
		success_num += this->insert(triple_array, parse_triple_num, _is_restore, txn);
		// success_num += this->batch_insert(triple_array, parse_triple_num, _is_restore, txn);
		long tv_end = Util::get_cur_time();
		SLOG_CORE("batch insert, used " << (tv_end - tv_begin) << " ms");
		// some maybe invalid or duplicate
		// triple_num += parse_triple_num;
	}

	delete[] triple_array;
	triple_array = NULL;
	long tv_insert = Util::get_cur_time();
	SLOG_CORE("after insert, used " << (tv_insert - tv_load) << "ms.");
	// BETTER:update kvstore and vstree separately, to lower the memory cost
	// flag = this->vstree->saveTree();
	// if (!flag)
	//{
	// return false;
	//}
	// flag = this->saveDBInfoFile();
	// if (!flag)
	//{
	// return false;
	//}

	SLOG_CORE("insert rdf triples done.");
	SLOG_CORE("inserted triples num: " << success_num);

	this->kvstore->set_if_single_thread(false);
	return true;
}

bool Database::remove(std::string _rdf_file, bool _is_restore, shared_ptr<Transaction> txn)
{
	bool flag = _is_restore || this->load();
	// bool flag = this->load();
	if (!flag)
	{
		return false;
	}
	SLOG_CORE("finish loading");

	long tv_load = Util::get_cur_time();
	TYPE_TRIPLE_NUM success_num = 0;

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		SLOG_ERROR("fail to open : " << _rdf_file << ".@remove_test");
		return false;
	}

	// NOTICE+WARN:we can not load all triples into memory all at once!!!
	TripleWithObjType *triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
	// parse a file
	RDFParser _parser(_fin);

	// int triple_num = 0;
#ifdef DEBUG
	Util::logging("==> while(true)");
#endif
	while (true)
	{
		int parse_triple_num = 0;
		_parser.parseFile(triple_array, parse_triple_num);
#ifdef DEBUG
		stringstream _ss;
		// NOTICE:this is not same as others, use parse_triple_num directly
		_ss << "finish rdfparser" << parse_triple_num << endl;
		Util::logging(_ss.str());
		cout << _ss.str() << endl;
#endif
		if (parse_triple_num == 0)
		{
			break;
		}

		long tv_begin = Util::get_cur_time();
		success_num += this->remove(triple_array, parse_triple_num, _is_restore, txn);
		long tv_end = Util::get_cur_time();
		SLOG_CORE("batch remove, used " << (tv_end - tv_begin) << " ms");
		// some maybe invalid or duplicate
		// triple_num -= parse_triple_num;
	}

	// BETTER: free this just after id_tuples are ok
	//(only when using group insertion/deletion)
	// or reduce the array size
	delete[] triple_array;
	triple_array = NULL;
	long tv_remove = Util::get_cur_time();
	SLOG_CORE("after remove, used " << (tv_remove - tv_load) << "ms.");

	// flag = this->vstree->saveTree();
	// if (!flag)
	//{
	// return false;
	//}
	// flag = this->saveDBInfoFile();
	// if (!flag)
	//{
	// return false;
	//}

	SLOG_CORE("remove rdf triples done.");
	SLOG_CORE("removed triples num: " << success_num );

	// if(this->vstree->isEmpty())
	if (this->triples_num == 0)
	{
		this->resetIDinfo();
	}
	this->kvstore->set_if_single_thread(false);
	return true;
}

unsigned
Database::insert(const TripleWithObjType *_triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore, shared_ptr<Transaction> txn)
{
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	TYPE_TRIPLE_NUM valid_num = 0;
	bool logging = false;
	if (!_is_restore)
	{
		if (logging)
			write_update_log(_triples, _triple_num, 1, txn);

		// string path = this->getStorePath() + '/' + this->update_log;
		// string path_all = this->getStorePath() + '/' + this->update_log_since_backup;
		// ofstream out;
		// ofstream out_all;
		// out.open(path.c_str(), ios::out | ios::app);
		// out_all.open(path_all.c_str(), ios::out | ios::app);
		// if (!out || !out_all) {
		// 	cerr << "Failed to open update log. Insertion aborted." << endl;
		// 	return 0;
		// }
		// for (int i = 0; i < _triple_num; i++) {
		// 	if (exist_triple(_triples[i], txn)) {
		// 		continue;
		// 	}
		// 	stringstream ss;
		// 	ss << "I\t" << Util::node2string(_triples[i].getSubject().c_str()) << '\t';
		// 	ss << Util::node2string(_triples[i].getPredicate().c_str()) << '\t';
		// 	ss << Util::node2string(_triples[i].getObject().c_str()) << '\t' << Util::get_cur_time() << '.' << endl;
		// 	out << ss.str();
		// 	out_all << ss.str();
		// }
		// out.close();
		// out_all.close();
	}

	// NOTICE:we deal with insertions one by one here
	// Callers should save the vstree(node and info) after calling this function
	for (TYPE_TRIPLE_NUM i = 0; i < _triple_num; ++i)
	{
		bool ret = this->insertTriple(_triples[i], &vertices, &predicates, txn);
		if (ret)
		{
			valid_num++;
		}
	}

	// update string index
	this->stringindex->change(vertices, *this->kvstore, true);
	this->stringindex->change(predicates, *this->kvstore, false);

	return valid_num;
}

unsigned
Database::remove(const TripleWithObjType *_triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore, shared_ptr<Transaction> txn)
{
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	TYPE_TRIPLE_NUM valid_num = 0;

	if (!_is_restore)
	{
		write_update_log(_triples, _triple_num, 0, txn);
		// string path = this->getStorePath() + '/' + this->update_log;
		// string path_all = this->getStorePath() + '/' + this->update_log_since_backup;
		// ofstream out;
		// ofstream out_all;
		// out.open(path.c_str(), ios::out | ios::app);
		// out_all.open(path_all.c_str(), ios::out | ios::app);
		// if (!out || !out_all) {
		// 	cerr << "Failed to open update log. Removal aborted." << endl;
		// 	return 0;
		// }
		// for (int i = 0; i < _triple_num; i++) {
		// 	if (!exist_triple(_triples[i], txn)) {
		// 		continue;
		// 	}
		// 	stringstream ss;
		// 	ss << "R\t" << Util::node2string(_triples[i].getSubject().c_str()) << '\t';
		// 	ss << Util::node2string(_triples[i].getPredicate().c_str()) << '\t';
		// 	ss << Util::node2string(_triples[i].getObject().c_str()) << '\t' << Util::get_cur_time() << '.' << endl;
		// 	out << ss.str();
		// 	out_all << ss.str();
		// }
		// out.close();
		// out_all.close();
	}

	// NOTICE:we deal with deletions one by one here
	// Callers should save the vstree(node and info) after calling this function
	for (TYPE_TRIPLE_NUM i = 0; i < _triple_num; ++i)
	{
		bool ret = this->removeTriple(_triples[i], &vertices, &predicates, txn);
		if (ret)
		{
			valid_num++;
		}
	}

	if (txn != nullptr)
	{
		this->stringindex->SetTrie(kvstore->getTrie());
		// update string index
		this->stringindex->disable(vertices, true);
		this->stringindex->disable(predicates, false);
	}
	// BETTER+TODO:this will require us to lock all when remove process not ends(in multiple threads cases)
	// An considerable idea is to check if empty after every triple removed
	// if(this->vstree->isEmpty())
	if (this->triples_num == 0)
	{
		this->resetIDinfo();
	}

	return valid_num;
}

unsigned
Database::batch_insert(std::string _rdf_file, bool _is_restore, shared_ptr<Transaction> txn)
{
	bool flag = _is_restore || this->load();
	if (!flag)
	{
		return -1;
	}
	SLOG_CORE("finish loading");

	long tv_load = Util::get_cur_time();

	unsigned success_num = 0;

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		SLOG_ERROR("fail to open : " << _rdf_file << ".@insert_test");
		// exit(0);
		return -1;
	}

	// NOTICE+WARN:we can not load all triples into memory all at once!!!
	// the parameter in build and insert must be the same, because RDF parser also use this
	// for build process, this one can be big enough if memory permits
	// for insert/delete process, this can not be too large, otherwise too costly
	TripleWithObjType *triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
	// parse a file
	RDFParser _parser(_fin);
	//parse error log
	string error_log = this->store_path + "/parse_error.log";
	SLOG_CORE("Error log file:" << error_log);
	//write build info to log
	FILE *fp = fopen(error_log.c_str(), "a");
	string log_msg = "Info " + Util::get_date_time() + " batch insert parser info, file path " + Util::getExactPath(_rdf_file.c_str()) + "\n";
	fputs(log_msg.c_str(), fp);
	fclose(fp);
	// TYPE_TRIPLE_NUM triple_num = 0;
	while (true)
	{
		int parse_triple_num = 0;
		_parser.parseFile(triple_array, parse_triple_num, error_log);
		if (parse_triple_num == 0)
		{
			break;
		}
		long tv_begin = Util::get_cur_time();
		success_num += this->batch_insert(triple_array, parse_triple_num, _is_restore, txn);
		long tv_end = Util::get_cur_time();
		SLOG_CORE("batch insert, used " << (tv_end - tv_begin) << " ms");
	}

	delete[] triple_array;
	triple_array = NULL;
	this->saveStatisticsInfoFile();
	long tv_insert = Util::get_cur_time();
	SLOG_CORE("after batch insert, used " << (tv_insert - tv_load) << "ms.");
	SLOG_CORE("insert rdf triples done.");
	SLOG_CORE("inserted triples num: " << success_num);

	return success_num;
}

unsigned
Database::batch_remove(std::string _rdf_file, bool _is_restore, shared_ptr<Transaction> txn)
{
	bool flag = _is_restore || this->load();
	if (!flag)
	{
		return -1;
	}
	SLOG_CORE("finish loading");

	long tv_load = Util::get_cur_time();
	unsigned success_num = 0;

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		SLOG_ERROR("fail to open : " << _rdf_file << ".@remove_test");
		return -1;
	}

	// NOTICE+WARN:we can not load all triples into memory all at once!!!
	TripleWithObjType *triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
	RDFParser _parser(_fin);

	while (true)
	{
		int parse_triple_num = 0;
		_parser.parseFile(triple_array, parse_triple_num);
		if (parse_triple_num == 0)
		{
			break;
		}

		long tv_begin = Util::get_cur_time();
		success_num += this->batch_remove(triple_array, parse_triple_num, _is_restore, txn);
		long tv_end = Util::get_cur_time();
		SLOG_CORE("batch remove, used " << (tv_end - tv_begin) << " ms");
	}

	delete[] triple_array;
	triple_array = NULL;
	this->saveStatisticsInfoFile();
	long tv_remove = Util::get_cur_time();
	SLOG_CORE("after batch remove, used " << (tv_remove - tv_load) << "ms.");
	SLOG_CORE("remove rdf triples done.");
	SLOG_CORE("removed triples num: " << success_num);

	if (this->triples_num == 0)
	{
		this->resetIDinfo();
	}
	return success_num;
}

// WARNING: TRANSACTIONAL batch insert is not completed yet!
unsigned
Database::batch_insert(const TripleWithObjType *_triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore, shared_ptr<Transaction> txn)
{
	if (_triple_num == 0)
		return 0;
	TYPE_TRIPLE_NUM valid_num = 0;
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	unsigned update_num_s = 0;
	unsigned update_num_p = 0;
	unsigned update_num_o = 0;
	unsigned update_num_subject = 0;
	set<TYPE_ENTITY_LITERAL_ID> sub_lists;
	set<TYPE_ENTITY_LITERAL_ID> obj_lists;

	if (!_is_restore)
	{
		write_update_log(_triples, _triple_num, 1, txn);
	}
	// bool is_obj_entity;
	vector<ID_TUPLE> id_tuples(_triple_num);
	for (unsigned i = 0; i < _triple_num; ++i)
	{
		TripleWithObjType _triple = _triples[i];
		TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
		if (_sub_id == INVALID_ENTITY_LITERAL_ID)
		{
			_sub_id = this->allocEntityID();
			update_num_subject = update_num_subject + 1;
			(this->kvstore)->setIDByEntity(_triple.subject, _sub_id);
			(this->kvstore)->setEntityByID(_sub_id, _triple.subject);
			vertices.push_back(_sub_id);
			sub_lists.insert(_sub_id);
		}
		else if (obj_lists.find(_sub_id) != obj_lists.end() && sub_lists.find(_sub_id) == sub_lists.end())
		{
			update_num_subject = update_num_subject + 1;
			sub_lists.insert(_sub_id);
		}

		TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
		if (_pre_id == INVALID_PREDICATE_ID)
		{
			_pre_id = this->allocPredicateID();
			(this->kvstore)->setIDByPredicate(_triple.predicate, _pre_id);
			(this->kvstore)->setPredicateByID(_pre_id, _triple.predicate);
			predicates.push_back(_pre_id);
		}

		TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
		bool is_obj_entity = _triple.isObjEntity();
		if (is_obj_entity)
		{
			_obj_id = (this->kvstore)->getIDByEntity(_triple.object);
			if (_obj_id == INVALID_ENTITY_LITERAL_ID)
			{
				_obj_id = this->allocEntityID();
				(this->kvstore)->setIDByEntity(_triple.object, _obj_id);
				(this->kvstore)->setEntityByID(_obj_id, _triple.object);
				vertices.push_back(_obj_id);
				obj_lists.insert(_obj_id);
			}
		}
		else
		{
			_obj_id = (this->kvstore)->getIDByLiteral(_triple.object);
			if (_obj_id == INVALID_ENTITY_LITERAL_ID)
			{
				_obj_id = this->allocLiteralID();
				(this->kvstore)->setIDByLiteral(_triple.object, _obj_id);
				(this->kvstore)->setLiteralByID(_obj_id, _triple.object);
				vertices.push_back(_obj_id);
			}
		}

		id_tuples[i].subid = _sub_id;
		id_tuples[i].preid = _pre_id;
		id_tuples[i].objid = _obj_id;
	}

	sort(id_tuples.begin(), id_tuples.end(), Util::spo_cmp_idtuple);
	auto new_end = unique(id_tuples.begin(), id_tuples.end(), Util::equal);
	id_tuples.erase(new_end, id_tuples.end());
	valid_num = id_tuples.size();
	if (txn != nullptr)
	{
		unordered_set<TYPE_ENTITY_LITERAL_ID> sids, oids;
		unordered_set<TYPE_PREDICATE_ID> pids;
		for (auto tuple : id_tuples)
		{
			sids.insert(tuple.subid);
			oids.insert(tuple.objid);
			pids.insert(tuple.preid);
		}
		vector<TYPE_ENTITY_LITERAL_ID> SLATCHES, OLATCHES;
		vector<TYPE_PREDICATE_ID> PLATCHES;
		SLATCHES.insert(SLATCHES.begin(), sids.begin(), sids.end());
		OLATCHES.insert(OLATCHES.begin(), oids.begin(), oids.end());
		PLATCHES.insert(PLATCHES.begin(), pids.begin(), pids.end());
		bool ret = (this->kvstore)->GetExclusiveLocks(SLATCHES, OLATCHES, PLATCHES, txn);
		if (ret == false)
		{
			return -1;
		}
	}
	// po inserts
	// sub_batch_update(id_tuples, valid_num, update_num_s, UPDATE_TYPE::SUBJECT_INSERT, txn);
	thread sub_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_s), UPDATE_TYPE::SUBJECT_INSERT, txn);

	// so inserts
	// sub_batch_update(id_tuples, valid_num, update_num_p, UPDATE_TYPE::PREDICATE_INSERT, txn);
	thread pre_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_p), UPDATE_TYPE::PREDICATE_INSERT, txn);

	// ps inserts
	// sub_batch_update(id_tuples, valid_num, update_num_o, UPDATE_TYPE::OBJECT_INSERT, txn);
	thread obj_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_o), UPDATE_TYPE::OBJECT_INSERT, txn);

	sub_t.join();
	pre_t.join();
	obj_t.join();

	unsigned update_num_triple = update_num_s;
	if (update_num_triple < update_num_p)
		update_num_triple = update_num_p;
	if (update_num_triple < update_num_o)
		update_num_triple = update_num_o;

	SLOG_CORE("update_num_triple:" << update_num_triple << ",update_num_s:" << update_num_s << ",update_num_p:" << update_num_p << ",update_num_o:" << update_num_o);
	// assert(update_num_o == update_num_p);
	// assert(update_num_s == update_num_o);

	this->triples_num = this->triples_num + update_num_triple;
	this->sub_num = this->sub_num + update_num_subject;
	this->stringindex->SetTrie(kvstore->getTrie());
	// update string index
	this->stringindex->change(vertices, *this->kvstore, true);
	this->stringindex->change(predicates, *this->kvstore, false);

	return update_num_s;
}

// WARNING: TRANSACTIONAL batch remove is not completed yet!
unsigned
Database::batch_remove(const TripleWithObjType *_triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore, shared_ptr<Transaction> txn)
{
	if (_triple_num == 0)
		return 0;
	TYPE_TRIPLE_NUM valid_num = 0;
	unsigned update_num_s = 0;
	unsigned update_num_p = 0;
	unsigned update_num_o = 0;
	unsigned update_num_subject=0;
	unsigned update_num_triple = 0;
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	set<unsigned> sub_ids, pre_ids, obj_ids;
	if (!_is_restore)
	{
		write_update_log(_triples, _triple_num, 0, txn);
	}

	vector<ID_TUPLE> id_tuples(_triple_num);
	for (unsigned i = 0; i < _triple_num; ++i)
	{
		TripleWithObjType _triple = _triples[i];
		TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
		if (_sub_id == INVALID_ENTITY_LITERAL_ID)
			continue;
		TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
		if (_pre_id == INVALID_PREDICATE_ID)
			continue;
		TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
		if (_triple.isObjEntity())
		{
			_obj_id = (this->kvstore)->getIDByEntity(_triple.object);
		}
		else
		{
			_obj_id = (this->kvstore)->getIDByLiteral(_triple.object);
		}

		if (_obj_id == INVALID_ENTITY_LITERAL_ID)
		{
			continue;
		}

		id_tuples[i].subid = _sub_id;
		id_tuples[i].preid = _pre_id;
		id_tuples[i].objid = _obj_id;
		sub_ids.insert(_sub_id);
		pre_ids.insert(_pre_id);
		obj_ids.insert(_obj_id);
	}

	sort(id_tuples.begin(), id_tuples.end(), Util::spo_cmp_idtuple);
	auto new_end = unique(id_tuples.begin(), id_tuples.end(), Util::equal);
	id_tuples.erase(new_end, id_tuples.end());
	valid_num = id_tuples.size();

	// po inserts
	// sub_batch_update(id_tuples, valid_num, update_num_s, UPDATE_TYPE::SUBJECT_INSERT, txn);
	thread sub_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_s), UPDATE_TYPE::SUBJECT_REMOVE, txn);

	// so inserts
	// sub_batch_update(id_tuples, valid_num, update_num_p, UPDATE_TYPE::PREDICATE_INSERT, txn);
	thread pre_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_p), UPDATE_TYPE::PREDICATE_REMOVE, txn);

	// ps inserts
	// sub_batch_update(id_tuples, valid_num, update_num_o, UPDATE_TYPE::OBJECT_INSERT, txn);
	thread obj_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_o), UPDATE_TYPE::OBJECT_REMOVE, txn);

	sub_t.join();
	pre_t.join();
	obj_t.join();
	//  assert(update_num_o == update_num_p);
	//  assert(update_num_s == update_num_o);

	if (update_num_triple < update_num_p)
		update_num_triple = update_num_p;
	if (update_num_triple < update_num_o)
		update_num_triple = update_num_o;

	SLOG_CORE("update_num_triple:" << update_num_triple << ",update_num_s:" << update_num_s << ",update_num_p:" << update_num_p << ",update_num_o:" << update_num_o);
	if (txn == nullptr)
	{
		for (auto _sub_id : sub_ids)
		{
			int sub_degree = (this->kvstore)->getEntityDegree(_sub_id);
			if (sub_degree == 0)
			{
				string subject = this->kvstore->getEntityByID(_sub_id);
				if (subject == "")
					continue;
				this->kvstore->subEntityByID(_sub_id);
				this->kvstore->subIDByEntity(subject);
				this->freeEntityID(_sub_id);
				//this->sub_num--;
				update_num_subject = update_num_subject + 1;
				vertices.push_back(_sub_id);
			}
		}

		for (auto _obj_id : obj_ids)
		{
			bool is_obj_entity = Util::is_entity_ele(_obj_id);
			int obj_degree;
			if (is_obj_entity)
			{
				obj_degree = this->kvstore->getEntityDegree(_obj_id);
				if (obj_degree == 0)
				{
					string object = this->kvstore->getEntityByID(_obj_id);
					if (object == "")
						continue;
					this->kvstore->subEntityByID(_obj_id);
					this->kvstore->subIDByEntity(object);
					this->freeEntityID(_obj_id);
					vertices.push_back(_obj_id);
				}
			}
			else
			{
				obj_degree = this->kvstore->getLiteralDegree(_obj_id);
				if (obj_degree == 0)
				{
					string object = this->kvstore->getLiteralByID(_obj_id);
					if (object == "")
						continue;
					this->kvstore->subLiteralByID(_obj_id);
					this->kvstore->subIDByLiteral(object);
					this->freeLiteralID(_obj_id);
					vertices.push_back(_obj_id);
				}
			}
		}
		for (auto _pre_id : pre_ids)
		{
			int pre_degree = this->kvstore->getPredicateDegree(_pre_id);
			if (pre_degree == 0)
			{
				string predicate = this->kvstore->getPredicateByID(_pre_id);
				if (predicate == "")
					continue;
				this->kvstore->subPredicateByID(_pre_id);
				this->kvstore->subIDByPredicate(predicate);
				this->freePredicateID(_pre_id);
				predicates.push_back(_pre_id);
			}
		}
		this->triples_num = this->triples_num - update_num_triple;
		if (this->sub_num > update_num_subject)
			this->sub_num = this->sub_num - update_num_subject;
		else
			this->sub_num = 0;
		this->stringindex->SetTrie(kvstore->getTrie());
		// update string index
		this->stringindex->disable(vertices, true);
		this->stringindex->disable(predicates, false);

		SLOG_CORE("vertices_num:" << vertices.size() << ",predicates:" << predicates.size());
	}
	return update_num_s;
}

void Database::sub_batch_update(vector<ID_TUPLE> id_tuples, TYPE_TRIPLE_NUM _triple_num, unsigned &update_num, UPDATE_TYPE type, shared_ptr<Transaction> txn)
{
	vector<unsigned> data;
	if (type == UPDATE_TYPE::SUBJECT_INSERT || type == UPDATE_TYPE::SUBJECT_REMOVE)
	{
		TYPE_ENTITY_LITERAL_ID sub_id = id_tuples[0].subid;
		for (unsigned i = 0; i < _triple_num; i++)
		{
			if (id_tuples[i].subid == sub_id)
			{
				data.push_back(id_tuples[i].preid);
				data.push_back(id_tuples[i].objid);
			}
			else
			{
				if (txn == nullptr)
				{
					if (type == UPDATE_TYPE::SUBJECT_INSERT)
						update_num += this->kvstore->updateInsert_s2values(sub_id, data);
					else if (type == UPDATE_TYPE::SUBJECT_REMOVE)
						update_num += this->kvstore->updateRemove_s2values(sub_id, data);
				}
				else
				{
				}
				data.clear();
				sub_id = id_tuples[i].subid;
				data.push_back(id_tuples[i].preid);
				data.push_back(id_tuples[i].objid);
			}
		}
		if (txn == nullptr)
		{
			if (type == UPDATE_TYPE::SUBJECT_INSERT)
				update_num += this->kvstore->updateInsert_s2values(sub_id, data);
			else if (type == UPDATE_TYPE::SUBJECT_REMOVE)
				update_num += this->kvstore->updateRemove_s2values(sub_id, data);
		}
		else
		{
		}
	}
	else if (type == UPDATE_TYPE::PREDICATE_INSERT || type == UPDATE_TYPE::PREDICATE_REMOVE)
	{
		sort(id_tuples.begin(), id_tuples.end(), Util::pso_cmp_idtuple);
		TYPE_PREDICATE_ID pre_id = id_tuples[0].preid;
		for (unsigned i = 0; i < _triple_num; i++)
		{
			if (id_tuples[i].preid == pre_id)
			{
				data.push_back(id_tuples[i].subid);
				data.push_back(id_tuples[i].objid);
			}
			else
			{
				if (txn == nullptr)
				{
					updateUmap(type, data, pre_id);
					if (type == UPDATE_TYPE::PREDICATE_INSERT)
						update_num += this->kvstore->updateInsert_p2values(pre_id, data);
					else if (type == UPDATE_TYPE::PREDICATE_REMOVE)
						update_num += this->kvstore->updateRemove_p2values(pre_id, data);
				}
				else
				{
				}
				data.clear();
				pre_id = id_tuples[i].preid;
				data.push_back(id_tuples[i].subid);
				data.push_back(id_tuples[i].objid);
			}
		}
		if (txn == nullptr)
		{
			updateUmap(type, data, pre_id);
			if (type == UPDATE_TYPE::PREDICATE_INSERT)
				update_num += this->kvstore->updateInsert_p2values(pre_id, data);
			else if (type == UPDATE_TYPE::PREDICATE_REMOVE)
				update_num += this->kvstore->updateRemove_p2values(pre_id, data);
		}
		else
		{
		}
	}
	else if (type == UPDATE_TYPE::OBJECT_INSERT || type == UPDATE_TYPE::OBJECT_REMOVE)
	{
		sort(id_tuples.begin(), id_tuples.end(), Util::ops_cmp_idtuple);
		TYPE_ENTITY_LITERAL_ID obj_id = id_tuples[0].objid;
		for (unsigned i = 0; i < _triple_num; i++)
		{
			if (id_tuples[i].objid == obj_id)
			{
				data.push_back(id_tuples[i].preid);
				data.push_back(id_tuples[i].subid);
			}
			else
			{
				if (txn == nullptr)
				{
					if (type == UPDATE_TYPE::OBJECT_INSERT)
						update_num += this->kvstore->updateInsert_o2values(obj_id, data);
					else if (type == UPDATE_TYPE::OBJECT_REMOVE)
						update_num += this->kvstore->updateRemove_o2values(obj_id, data);
				}
				else
				{
					// todo: complete this, please read DevelopDoc for instruction
					SLOG_ERROR("Uncomplete function in Database::sub_batch_update");
					exit(-1);
				}
				data.clear();
				obj_id = id_tuples[i].objid;
				data.push_back(id_tuples[i].preid);
				data.push_back(id_tuples[i].subid);
			}
		}
		if (txn == nullptr)
		{
			if (type == UPDATE_TYPE::OBJECT_INSERT)
				update_num += this->kvstore->updateInsert_o2values(obj_id, data);
			else if (type == UPDATE_TYPE::OBJECT_REMOVE)
				update_num += this->kvstore->updateRemove_o2values(obj_id, data);
		}
		else
		{
		}
	}
}

bool Database::backup()
{
	if (!Util::dir_exist(Util::backup_path))
	{
		Util::create_dir(Util::backup_path);
	}
	string backup_path = Util::backup_path + this->name + Util::global_config["db_suffix"];

	SLOG_CORE("Beginning backup, path is: "<< backup_path);

	string sys_cmd;
	if (Util::dir_exist(backup_path))
	{
		Util::remove_path(backup_path);
	}
	sys_cmd = "cp -r " + this->store_path + ' ' + backup_path;
	system(sys_cmd.c_str());
	Util::remove_path(backup_path + '/' + this->update_log);

	// this->vstree->saveTree();
	this->kvstore->flush();

	this->clear_update_log();
	string update_log_path = this->store_path + '/' + this->update_log_since_backup;
	Util::remove_path(update_log_path);
	Util::create_file(update_log_path);

	SLOG_CORE("Backup completed!");
	return true;
}

bool Database::restore()
{
	SLOG_CORE("Begining restore.");
	string sys_cmd;

	multiset<string> insertions;
	multiset<string> removals;

	int num_update = 0;
	if (!this->load())
	{
		this->clear();

		string backup_path = Util::backup_path + this->name + Util::global_config["db_suffix"];
		if (!Util::dir_exist(Util::backup_path))
		{
			SLOG_ERROR("Failed to restore!");
			return false;
		}

		num_update += Database::read_update_log(this->store_path + '/' + this->update_log_since_backup, insertions, removals);

		SLOG_CORE("Failed to restore from original db file, trying to restore from backup file.");
		SLOG_CORE("Your old db file will be stored at " << this->store_path << ".bad");

		Util::remove_path(this->store_path + ".bad");
		sys_cmd = "cp -r " + this->store_path + ' ' + this->store_path + ".bad";
		system(sys_cmd.c_str());
		Util::remove_path(this->store_path);
		sys_cmd = "cp -r " + backup_path + ' ' + this->store_path;
		system(sys_cmd.c_str());
		Util::create_file(this->store_path + '/' + this->update_log);

		if (!this->load())
		{
			this->clear();
			SLOG_ERROR("Failed to restore from backup file.");
			return false;
		}

		num_update += Database::read_update_log(this->store_path + '/' + this->update_log_since_backup, insertions, removals);
	}
	else
	{
		num_update += Database::read_update_log(this->store_path + '/' + this->update_log, insertions, removals);
	}

	SLOG_CORE("Restoring " << num_update << " updates.");

	if (!this->restore_update(insertions, removals))
	{
		SLOG_ERROR("Failed to restore updates");
		return false;
	}

	SLOG_CORE("Restore completed.");

	return true;
}

int Database::read_update_log(const string _path, multiset<string> &_i, multiset<string> &_r)
{
	ifstream in;
#ifdef DEBUG
	cout << _path << endl;
#endif
	in.open(_path.c_str(), ios::in);
	if (!in)
	{
		SLOG_ERROR("Failed to read update log.");
		return 0;
	}

	int ret = 0;
	int buffer_size = 1024 + 2;
	char buffer[buffer_size];
	in.getline(buffer, buffer_size);
	while (!in.eof() && buffer[0])
	{
		string triple;
		switch (buffer[0])
		{
		case 'I':
			triple = string(buffer + 2);
			ret++;
			_i.insert(triple);
			break;
		case 'R':
			triple = string(buffer + 2);
			ret++;
			_r.insert(triple);
			break;
		default:
			SLOG_ERROR("Bad line in update log!");
		}
		in.getline(buffer, buffer_size);
	}

	return ret;
}

bool Database::restore_update(multiset<string> &_i, multiset<string> &_r)
{
	multiset<string>::iterator pos;
	multiset<string>::iterator it_to_erase = _r.end();
	for (multiset<string>::iterator it = _r.begin(); it != _r.end(); it++)
	{
		// NOTICE: check the intersect of insert_set and remove_set
		if (it_to_erase != _r.end())
		{
			_r.erase(it_to_erase);
		}
		pos = _i.find(*it);
		if (pos != _i.end())
		{
			_i.erase(pos);
			it_to_erase = it;
		}
		else
		{
			it_to_erase = _r.end();
		}
	}
	if (it_to_erase != _r.end())
	{
		_r.erase(it_to_erase);
	}

	string tmp_path = this->store_path + "/.update_tmp";

	ofstream out_i;
	out_i.open(tmp_path.c_str(), ios::out);
	if (!out_i)
	{
		SLOG_ERROR("Failed to open temp file, restore failed!");
		return false;
	}
	for (multiset<string>::iterator it = _i.begin(); it != _i.end(); it++)
	{
		out_i << *it << endl;
	}
	out_i.close();
	if (!this->insert(tmp_path, true))
	// if (!this->remove(tmp_path, true))
	{
		return false;
	}

	ofstream out_r;
	out_r.open(tmp_path.c_str(), ios::out);
	if (!out_r)
	{
		SLOG_ERROR("Failed to open temp file!");
		return false;
	}
	for (multiset<string>::iterator it = _r.begin(); it != _r.end(); it++)
	{
		out_r << *it << endl;
	}
	out_r.close();
	if (!this->remove(tmp_path, true))
	// if (!this->insert(tmp_path, true))
	{
		return false;
	}

	Util::remove_path(tmp_path);
	return true;
}

void Database::clear_update_log()
{
	string path = this->getStorePath() + '/' + this->update_log;
	ofstream out;
	out.open(path.c_str(), ios::out);
	out.close();
}

bool Database::write_update_log(const TripleWithObjType *_triples, TYPE_TRIPLE_NUM _triple_num, int type, shared_ptr<Transaction> txn)
{
	log_lock.lock();
	string path = this->getStorePath() + '/' + this->update_log;
	string path_all = this->getStorePath() + '/' + this->update_log_since_backup;
	ofstream out;
	ofstream out_all;
	out.open(path.c_str(), ios::out | ios::app);
	out_all.open(path_all.c_str(), ios::out | ios::app);
	if (!out || !out_all)
	{
		SLOG_ERROR("Failed to open update log. Insertion aborted.");
		log_lock.unlock();
		return false;
	}

	for (unsigned i = 0; i < _triple_num; i++)
	{
		// if (type == 1 && exist_triple(_triples[i], txn)) {
		//	continue;
		// }
		// else if(type == 0 && !exist_triple(_triples[i], txn))
		// {
		//	continue;
		// }
		stringstream ss;
		if (type == 1)
		{
			ss << "I\t" << Util::node2string(_triples[i].getSubject().c_str()) << '\t';
		}
		else
		{
			ss << "R\t" << Util::node2string(_triples[i].getSubject().c_str()) << '\t';
		}
		ss << Util::node2string(_triples[i].getPredicate().c_str()) << '\t';
		ss << Util::node2string(_triples[i].getObject().c_str()) << '\t' << Util::get_cur_time() << '.' << endl;
		out << ss.str();
		out_all << ss.str();
	}
	out.close();
	out_all.close();
	log_lock.unlock();
	return true;
}

bool Database::objIDIsEntityID(TYPE_ENTITY_LITERAL_ID _id)
{
	return _id < Util::LITERAL_FIRST_ID;
}

bool Database::getFinalResult(SPARQLquery &_sparql_q, ResultSet &_result_set)
{
#ifdef DEBUG_PRECISE
	printf("getFinalResult:begins\n");
#endif
	// this is only selected var num
	int _var_num = _sparql_q.getQueryVarNum();
	_result_set.setVar(_sparql_q.getQueryVar());
	vector<BasicQuery *> &query_vec = _sparql_q.getBasicQueryVec();

	// sum the answer number
	unsigned _ans_num = 0;
#ifdef DEBUG_PRECISE
	printf("getFinalResult:before ansnum loop\n");
#endif
	for (unsigned i = 0; i < query_vec.size(); i++)
	{
		_ans_num += query_vec[i]->getResultList().size();
	}
#ifdef DEBUG_PRECISE
	printf("getFinalResult:after ansnum loop\n");
#endif

	_result_set.ansNum = _ans_num;
#ifndef STREAM_ON
	_result_set.answer = new string *[_ans_num];
	for (unsigned i = 0; i < _result_set.ansNum; i++)
	{
		_result_set.answer[i] = NULL;
	}
#else
	vector<unsigned> keys;
	vector<bool> desc;
	_result_set.openStream(keys, desc);
	//_result_set.openStream(keys, desc, 0, -1);
#ifdef DEBUG_PRECISE
	printf("getFinalResult:after open stream\n");
#endif
#endif
#ifdef DEBUG_PRECISE
	printf("getFinalResult:before main loop\n");
#endif
	unsigned tmp_ans_count = 0;
	// map int ans into string ans
	// union every basic result into total result
	for (unsigned i = 0; i < query_vec.size(); i++)
	{
		vector<unsigned *> &tmp_vec = query_vec[i]->getResultList();
		// ensure the spo order is right, but the triple order is still reversed
		// for every result group in resultlist
		// for(vector<int*>::reverse_iterator itr = tmp_vec.rbegin(); itr != tmp_vec.rend(); ++itr)
		for (vector<unsigned *>::iterator itr = tmp_vec.begin(); itr != tmp_vec.end(); ++itr)
		{
			// to ensure the order so do reversely in two nested loops
#ifndef STREAM_ON
			_result_set.answer[tmp_ans_count] = new string[_var_num];
#endif
#ifdef DEBUG_PRECISE
			printf("getFinalResult:before map loop\n");
#endif
			// NOTICE: in new join method only selec_var_num columns,
			// but before in shenxuchuan's join method, not like this.
			// though there is all graph_var_num columns in result_list,
			// we only consider the former selected vars
			// map every ans_id into ans_str
			for (int v = 0; v < _var_num; ++v)
			{
				unsigned ans_id = (*itr)[v];
				string ans_str;
				if (this->objIDIsEntityID(ans_id))
				{
					ans_str = (this->kvstore)->getEntityByID(ans_id);
				}
				else
				{
					ans_str = (this->kvstore)->getLiteralByID(ans_id);
				}
#ifndef STREAM_ON
				_result_set.answer[tmp_ans_count][v] = ans_str;
#else
				_result_set.writeToStream(ans_str);
#endif
#ifdef DEBUG_PRECISE
				printf("getFinalResult:after copy/write\n");
#endif
			}
			tmp_ans_count++;
		}
	}
#ifdef STREAM_ON
	_result_set.resetStream();
#endif
#ifdef DEBUG_PRECISE
	printf("getFinalResult:ends\n");
#endif

	return true;
}

// garbage clean
void Database::VersionClean(vector<unsigned> &sub_ids, vector<unsigned> &obj_ids, vector<unsigned> &obj_literal_ids, vector<unsigned> &pre_ids)
{
	// vector<unsigned> sub_ids , obj_ids, obj_literal_ids, pre_ids;
	(this->kvstore)->IVArrayVacuum(sub_ids, obj_ids, obj_literal_ids, pre_ids);
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	// update
	int sub_degree, obj_degree, pre_degree;
	for (auto &_sub_id : sub_ids)
	{
		sub_degree = (this->kvstore)->getEntityDegree(_sub_id);
		// if subject become an isolated point, remove its corresponding entry
		if (sub_degree == 0)
		{
			string sub_str = this->kvstore->getEntityByID(_sub_id);
			// cerr << "sub_str" << sub_str << endl;
			if (sub_str == "")
				continue;
			this->kvstore->subIDByEntity(sub_str);
			this->kvstore->subEntityByID(_sub_id);
			this->freeEntityID(_sub_id);
			this->sub_num--;
			// update the string buffer
			// if (_sub_id < this->entity_buffer_size)
			//{
			// this->entity_buffer->del(_sub_id);
			//}
			vertices.push_back(_sub_id);
		}
	}

	for (auto &_obj_id : obj_ids)
	{
		obj_degree = this->kvstore->getEntityDegree(_obj_id);
		if (obj_degree == 0)
		{
			string obj_str = this->kvstore->getEntityByID(_obj_id);
			if (obj_str == "")
				continue;
			this->kvstore->subIDByEntity(obj_str);
			this->kvstore->subEntityByID(_obj_id);
			this->freeEntityID(_obj_id);
			// update the string buffer
			// if (_obj_id < this->entity_buffer_size)
			//{
			// this->entity_buffer->del(_obj_id);
			//}
			vertices.push_back(_obj_id);
		}
	}

	for (auto &_obj_id : obj_literal_ids)
	{
		// cerr << _obj_id << endl;
		obj_degree = this->kvstore->getLiteralDegree(_obj_id);
		if (obj_degree == 0)
		{
			string obj_str = this->kvstore->getLiteralByID(_obj_id);
			SLOG_ERROR("obj_str" << obj_str << "     _obj_id" << _obj_id);
			if (obj_str == "")
				continue;

			this->kvstore->subLiteralByID(_obj_id);
			this->kvstore->subIDByLiteral(obj_str);
			this->freeLiteralID(_obj_id);
			// update the string buffer
			// TYPE_ENTITY_LITERAL_ID tid = _obj_id - Util::LITERAL_FIRST_ID;
			// if (tid < this->literal_buffer_size)
			//{
			// this->literal_buffer->del(tid);
			// }
			vertices.push_back(_obj_id);
		}
	}
	for (auto &_pre_id : pre_ids)
	{
		pre_degree = this->kvstore->getPredicateDegree(_pre_id);
		if (pre_degree == 0)
		{
			string pre_str = this->kvstore->getPredicateByID(_pre_id);
			// cerr << "pre_str" << pre_str << endl;
			this->kvstore->subIDByPredicate(pre_str);
			this->kvstore->subPredicateByID(_pre_id);
			this->freePredicateID(_pre_id);
			predicates.push_back(_pre_id);
		}
	}

	// cerr << "vertices.size()" << vertices.size() << endl;
	// cerr << "predicates.size()" << predicates.size() << endl;
	this->stringindex->SetTrie(kvstore->getTrie());
	// update string index
	this->stringindex->disable(vertices, true);
	this->stringindex->disable(predicates, false);
}

void Database::TransactionRollback(shared_ptr<Transaction> txn)
{
	if ((this->kvstore)->TransactionInvalid(txn) == false)
	{
		SLOG_ERROR("WARNING: transaction rollback exception! ");
		SLOG_ERROR("Please REBOOT service!");
	}
}

void Database::TransactionCommit(shared_ptr<Transaction> txn)
{
	if ((this->kvstore)->ReleaseAllLocks(txn) == false)
	{
		SLOG_ERROR("WARNING: not all latches get unlatched! ");
		SLOG_ERROR("Please REBOOT service!");
	}
	// if((this->kvstore)->releaseAllExclusiveLocks(txn) == false)
	// {
	// 	cerr << "WARNING: not all lockes get unlocked! " << endl;
	// 	cerr << "Please REBOOT service!" << endl;
	// }
}

std::string
Database::CreateJson(int StatusCode, std::string StatusMsg, std::string ResponseBody)
{
	StringBuffer s;
	PrettyWriter<StringBuffer> writer(s);
	writer.StartObject();
	writer.Key("ResponseBody");
	writer.String(StringRef(ResponseBody.c_str()));
	writer.Key("StatusCode");
	writer.Uint(StatusCode);
	writer.Key("StatusMsg");
	writer.String(StringRef(StatusMsg.c_str()));
	writer.EndObject();
	std::string res = s.GetString();
	return res;
}

bool Database::saveStatisticsInfoFile()
{
	ofstream file;
	string filepath = this->getStorePath() + "/" + this->statistics_info_file;
	if (Util::file_exist(filepath) == false)
	{
		SLOG_CORE("create statistics file.");
		Util::create_file(filepath);
	}
	file.open(filepath);
	int i = 0;
	for (auto &kv : this->umap)
	{
		file << kv.first << "@@" << kv.second << endl;
		i++;
	}
	file.flush();
	file.close();
	SLOG_CORE("save the statistics file successfully! total " << i << " records have been saved!");
	return true;
}

bool Database::loadStatisticsInfoFile()
{

	string filepath = this->getStorePath() + "/" + this->statistics_info_file;
	if (Util::file_exist(filepath) == false)
	{
		SLOG_ERROR("The statistics file is not exist.");
		SLOG_ERROR("Statistics file load failed!");
		return false;
	}
	ifstream file(filepath, ios::in);
	string line;
	vector<string> lines;

	unsigned long long value;
	if (file)
	{
		this->umap.clear();
		while (getline(file, line))
		{
			lines.clear();
			Util::split(line, "@@", lines);
			if (lines.size() == 2)
			{
				value = stoull(lines[1].c_str(), nullptr, 0);
				this->umap.insert(pair<string, unsigned long long>(lines[0], value));
			}
		}
		return true;
	}
	else
		return false;
}

unordered_map<string, unsigned long long> Database::getStatisticsInfo()
{
	return this->umap;
}

void Database::updateUmap(UPDATE_TYPE type, const std::vector<unsigned>& _sidoidlist, TYPE_ENTITY_LITERAL_ID pred_id)
{
	if (_sidoidlist.size() == 0)
		return;
	string _pre = this->kvstore->getPredicateByID(pred_id);
	if (!this->checkIsTypePredicate(_pre))
		return;
	umap_lock.lock();
	std::map<unsigned, set<unsigned>> sub_obj_list;
	if (type == UPDATE_TYPE::PREDICATE_INSERT)
	{
		TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
		TYPE_ENTITY_LITERAL_ID edge_list_len;
		this->kvstore->getsubIDobjIDlistBypreID(pred_id, edge_candidate_list, edge_list_len);
		edge_list_len = edge_list_len/2;
		for (size_t i =0; i < edge_list_len; i++)
		{
			auto subid = edge_candidate_list[2*i];
			auto objid = edge_candidate_list[2*i+1];
			if (!Util::is_entity_ele(objid))
				continue;
			sub_obj_list[subid].insert(objid);
		}
		delete [] edge_candidate_list;
		edge_candidate_list = nullptr;

		for(unsigned i = 0; i < _sidoidlist.size(); i += 2)
    	{
			auto subid = _sidoidlist[i];
			auto objid = _sidoidlist[i+1];
			if (!Util::is_entity_ele(objid))
				continue;
			string _obj = this->kvstore->getEntityByID(objid);
			if (_obj.empty())
				continue;
			auto it = sub_obj_list.find(subid);
			if (it != sub_obj_list.end() && it->second.find(objid) != it->second.end())
				continue;
			auto obj_it = this->umap.find(_obj);
			if (obj_it != this->umap.end())
			{
				obj_it->second = obj_it->second + 1;
			}
			else
			{
				this->umap.insert(pair<string, unsigned long long>(_obj, 1));
			}
		}
	}
	else if (type == UPDATE_TYPE::PREDICATE_REMOVE)
	{
		for (unsigned i = 0; i < _sidoidlist.size(); i += 2)
		{
			auto objid = _sidoidlist[i+1];
			if (!Util::is_entity_ele(objid))
				continue;
			string _obj = this->kvstore->getEntityByID(objid);
			if (_obj.empty())
				continue;
			auto it = this->umap.find(_obj);
			if (it != this->umap.end())
			{
				if (it->second <= 1)
					umap.erase(it);
				else
					it->second = it->second - 1;
			}
		}
	}
	umap_lock.unlock();
}