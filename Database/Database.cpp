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
	this->statistics_info_file="statistics_info_file.dat";
	this->id_tuples_file = "id_tuples";
	this->update_log = "update.log";
	this->update_log_since_backup = "update_since_backup.log";
	this->csr = NULL;

	this->type_predicate_name="type@@TYPE@@类型";
	// this->csr = new CSR[2];

	string kv_store_path = store_path + "/kv_store";
	this->kvstore = new KVstore(kv_store_path);

	// string vstree_store_path = store_path + "/vs_store";
	// this->vstree = new VSTree(vstree_store_path);

	string stringindex_store_path = store_path + "/stringindex_store";
	this->stringindex = new StringIndex(stringindex_store_path);
	this->stringindex->SetTrie(this->kvstore->getTrie());
	//this->encode_mode = Database::STRING_MODE;
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

	//this->trie = NULL;

	//this->resetIDinfo();
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
	this->statistics_info_file="statistics_info_file.dat";
	this->id_tuples_file = "id_tuples";
	this->update_log = "update.log";
	this->update_log_since_backup = "update_since_backup.log";
	this->csr = NULL;
	// this->csr = new CSR[2];
    this->type_predicate_name="type@@TYPE@@类型";
	string kv_store_path = store_path + "/kv_store";
	this->kvstore = new KVstore(kv_store_path);
	// string vstree_store_path = store_path + "/vs_store";
	//this->vstree = new VSTree(vstree_store_path);
	// this->vstree = NULL;
	string stringindex_store_path = store_path + "/stringindex_store";
	this->stringindex = new StringIndex(stringindex_store_path);
	this->stringindex->SetTrie(this->kvstore->getTrie());
	//this->encode_mode = Database::STRING_MODE;
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

	//this->trie = NULL;

	//this->resetIDinfo();
	this->initIDinfo();

	pthread_rwlock_init(&(this->update_lock), NULL);
}

//==================================================================================================================================================
//NOTICE:
//there are 3 ways to manage a dynamic-garbage ID list
//1. push all unused IDs into list, get top each time to alloc, push freed one to tail, push more large one if NULL
//(can use bit array if stored in disk)
//2. when free, change the mapping for non-free one whose ID is the largest currently
//(sometimes maybe the copy cost is very high)
//3. NULL first and push one if freed, get one if not empty(limit+1 if NULL)
//(when stored in disk, maybe consume larger space)
//However, this method can keep the used largest easily(no!! 1->2->3, delete 2 and delete 3, then the max 1 is not kept by limit!)

void
Database::initIDinfo()
{
	//NOTICE:keep that limit-1 the maxium using ID
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

void
Database::resetIDinfo()
{
	this->initIDinfo();

	//this->limitID_entity = Database::START_ID_NUM;
	//NOTICE:add base LITERAL_FIRST_ID for literals
	//this->limitID_literal = Database::START_ID_NUM;
	//this->limitID_predicate = Database::START_ID_NUM;

	//BlockInfo* tmp = NULL;
	//for (TYPE_ENTITY_LITERAL_ID i = Database::START_ID_NUM - 1; i >= 0; --i)
	//{
	//tmp = new BlockInfo(i, this->freelist_entity);
	//this->freelist_entity = tmp;
	//tmp = new BlockInfo(i, this->freelist_literal);
	//this->freelist_literal = tmp;
	//tmp = new BlockInfo(i, this->freelist_predicate);
	//this->freelist_predicate = tmp;
	//}
}

void
Database::readIDinfo()
{
	this->initIDinfo();

	FILE* fp = NULL;
	//int t = -1;
	TYPE_ENTITY_LITERAL_ID t = INVALID_ENTITY_LITERAL_ID;
	BlockInfo* bp = NULL;

	fp = fopen(this->free_id_file_entity.c_str(), "r");
	if (fp == NULL)
	{
		cout << "read entity id info error" << endl;
		return;
	}
	//QUERY:this will reverse the original order, if change?
	//Notice that if we cannot ensure that IDs are uporder and continuous, we can
	//not keep an array for IDs like _entity_bitset
	BlockInfo *tmp = NULL, *cur = NULL;
	fread(&(this->limitID_entity), sizeof(int), 1, fp);
	fread(&t, sizeof(int), 1, fp);
	while (!feof(fp))
	{
		//if(t == 14912)
		//{
		//cout<<"Database::readIDinfo() - get 14912"<<endl;
		//}
		//bp = new BlockInfo(t, this->freelist_entity);
		//this->freelist_entity = bp;
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
		cout << "read literal id info error" << endl;
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
		cout << "read predicate id info error" << endl;
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

void
Database::writeIDinfo()
{
	//cout<<"now to write the id info"<<endl;
	FILE* fp = NULL;
	BlockInfo *bp = NULL, *tp = NULL;

	fp = fopen(this->free_id_file_entity.c_str(), "w+");
	if (fp == NULL)
	{
		cout << "write entity id info error" << endl;
		return;
	}
	fwrite(&(this->limitID_entity), sizeof(int), 1, fp);
	bp = this->freelist_entity;
	while (bp != NULL)
	{
		//if(bp->num == 14912)
		//{
		//cout<<"Database::writeIDinfo() - get 14912"<<endl;
		//}
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
		cout << "write literal id info error" << endl;
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
		cout << "write predicate id info error" << endl;
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

void
Database::saveIDinfo()
{
	//cout<<"now to write the id info"<<endl;
	FILE* fp = NULL;
	BlockInfo *bp = NULL, *tp = NULL;

	fp = fopen(this->free_id_file_entity.c_str(), "w+");
	if (fp == NULL)
	{
		cout << "write entity id info error" << endl;
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
		cout << "write literal id info error" << endl;
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
		cout << "write predicate id info error" << endl;
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


//ID alloc garbage error(LITERAL_FIRST_ID or double) add base for literal
TYPE_ENTITY_LITERAL_ID
Database::allocEntityID()
{
	allocEntityID_lock.lock();
	//int t;
	TYPE_ENTITY_LITERAL_ID t = INVALID_ENTITY_LITERAL_ID;

	if (this->freelist_entity == NULL)
	{
		t = this->limitID_entity++;
		if (this->limitID_entity >= Util::LITERAL_FIRST_ID)
		{
			cout << "fail to alloc id for entity" << endl;
			//return -1;
			allocEntityID_lock.unlock();
			return INVALID;
		}
	}
	else
	{
		t = this->freelist_entity->num;
		BlockInfo* op = this->freelist_entity;
		this->freelist_entity = this->freelist_entity->next;
		delete op;
	}

	this->entity_num++;
	allocEntityID_lock.unlock();
	return t;
}

void
Database::freeEntityID(TYPE_ENTITY_LITERAL_ID _id)
{
	allocEntityID_lock.lock();
	if (_id == this->limitID_entity - 1)
	{
		this->limitID_entity--;
	}
	else
	{
		BlockInfo* p = new BlockInfo(_id, this->freelist_entity);
		this->freelist_entity = p;
	}

	this->entity_num--;
	allocEntityID_lock.unlock();
}

TYPE_ENTITY_LITERAL_ID
Database::allocLiteralID()
{
	//int t;
	allocLiteralID_lock.lock();
	TYPE_ENTITY_LITERAL_ID t = INVALID_ENTITY_LITERAL_ID;

	if (this->freelist_literal == NULL)
	{
		t = this->limitID_literal++;
		if (this->limitID_literal >= Util::LITERAL_FIRST_ID)
		{
			cout << "fail to alloc id for literal" << endl;
			//return -1;
			allocLiteralID_lock.unlock();
			return INVALID;
		}
	}
	else
	{
		t = this->freelist_literal->num;
		BlockInfo* op = this->freelist_literal;
		this->freelist_literal = this->freelist_literal->next;
		delete op;
	}

	this->literal_num++;
	allocLiteralID_lock.unlock();
	return t + Util::LITERAL_FIRST_ID;
}

void
Database::freeLiteralID(TYPE_ENTITY_LITERAL_ID _id)
{
	allocLiteralID_lock.lock();
	_id -= Util::LITERAL_FIRST_ID;

	if (_id == this->limitID_literal - 1)
	{
		this->limitID_literal--;
	}
	else
	{
		BlockInfo* p = new BlockInfo(_id, this->freelist_literal);
		this->freelist_literal = p;
	}

	this->literal_num--;
	allocLiteralID_lock.unlock();
}

TYPE_PREDICATE_ID
Database::allocPredicateID()
{
	//int t;
	allocPredicateID_lock.lock();
	TYPE_PREDICATE_ID t = INVALID_PREDICATE_ID;

	if (this->freelist_predicate == NULL)
	{
		t = this->limitID_predicate++;
		if (this->limitID_predicate >= Util::LITERAL_FIRST_ID)
		{
			cout << "fail to alloc id for predicate" << endl;
			//WARN:if pid is changed to unsigned type, this must be changed
			allocPredicateID_lock.unlock();
			return -1;
		}
	}
	else
	{
		t = this->freelist_predicate->num;
		BlockInfo* op = this->freelist_predicate;
		this->freelist_predicate = this->freelist_predicate->next;
		delete op;
	}

	this->pre_num++;
	allocPredicateID_lock.unlock();
	return t;
}

void
Database::freePredicateID(TYPE_PREDICATE_ID _id)
{
	allocPredicateID_lock.lock();
	if (_id == this->limitID_predicate - 1)
	{
		this->limitID_predicate--;
	}
	else
	{
		BlockInfo* p = new BlockInfo(_id, this->freelist_predicate);
		this->freelist_predicate = p;
	}

	this->pre_num--;
	allocPredicateID_lock.unlock();
}

void
Database::release(FILE* fp0)
{
	fprintf(fp0, "begin to delete DB!\n");
	fflush(fp0);
	//this->vstree->saveTree();
	//delete this->vstree;
	//fprintf(fp0, "ok to delete vstree!\n");
	fflush(fp0);
	delete this->kvstore;
	fprintf(fp0, "ok to delete kvstore!\n");
	fflush(fp0);
	//fclose(Util::debug_database);
	//Util::debug_database = NULL;	//debug: when multiple databases
	fprintf(fp0, "ok to delete DB!\n");
	fflush(fp0);
}

Database::~Database()
{
	pthread_rwlock_destroy(&(this->update_lock));
	this->unload();
	//fclose(Util::debug_database);
	//Util::debug_database = NULL;	//debug: when multiple databases
}

//TODO: update pre map if insert/delete
void
Database::setPreMap()
{
	//this->maxNumPID = this->minNumPID = -1;a
	this->maxNumPID = this->minNumPID = INVALID_PREDICATE_ID;
	//int max = 0, min = this->triples_num + 1;
	TYPE_TRIPLE_NUM max = 0, min = this->triples_num + 1;

	this->pre2num = new TYPE_TRIPLE_NUM[this->limitID_predicate];
	this->pre2sub = new TYPE_TRIPLE_NUM[this->limitID_predicate];
	this->pre2obj = new TYPE_TRIPLE_NUM[this->limitID_predicate];
	TYPE_PREDICATE_ID valid = 0, i, t;

	for (i = 0; i < this->limitID_predicate; ++i)
	{
		if (valid == this->pre_num)
		{
			t = 0;
		}
		else
		{
			t = this->kvstore->getPredicateDegree(i);
		}
		this->pre2num[i] = t;

		//NOTICE:only when pre2num[i]>0 then i is a valid predicate id
		if (t > 0)
		{
			valid++;
			if (t > max)
			{
				this->maxNumPID = i;
			}
			if (t < min)
			{
				this->minNumPID = i;
			}

			unsigned *list = NULL;
			unsigned len = 0;
			this->kvstore->getsubIDlistBypreID(i,list,len,true);
			this->pre2sub[i] = len;
			free(list);
			this->kvstore->getobjIDlistBypreID(i,list,len,true);
			this->pre2obj[i] = len;
			free(list);
		}
		else
		{
			this->pre2sub[i] = 0;
			this->pre2obj[i] = 0;
		}
	}
	/*
	for(int i = 0;i < this->pre_num;i++)
	{
		cout <<"pre2num["<<i<<"]: "<<this->pre2num[i]<<endl;
		cout <<"pre2sub["<<i<<"]: "<<this->pre2sub[i]<<endl;
		cout <<"pre2obj["<<i<<"]: "<<this->pre2obj[i]<<endl;
	}
	*/

}

void
Database::setStringBuffer()
{
	//BETTER: assign according to memory manager
	//BETTER?maybe different size for entity and literal, maybe different offset should be used
	this->entity_buffer_size = (this->limitID_entity<50000000) ? this->limitID_entity : 50000000;
	this->literal_buffer_size = (this->limitID_literal<50000000) ? this->limitID_literal : 50000000;
	this->entity_buffer = new Buffer(this->entity_buffer_size);
	this->literal_buffer = new Buffer(this->literal_buffer_size);

	//DEBUG: insert/delete we should update the size of buffer if adding new string
	//WARN: after delete and insert, IDs may be not continuous, then the string buffer will cause errors!
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

	//use this string buffer in StringIndex class, if not exist, then also use StringIndex to search
	this->stringindex->setBuffer(this->entity_buffer, this->literal_buffer);
}

void
Database::warmUp()
{
	//the most frequent triple
	TYPE_PREDICATE_ID pid1 = this->maxNumPID;
	ResultSet rs1;
	string str1 = "select ?s ?o where { ?s " + this->kvstore->getPredicateByID(pid1) + " ?o . }";
	this->query(str1, rs1);
	//the most infrequent triple
	TYPE_PREDICATE_ID pid2 = this->minNumPID;
	ResultSet rs2;
	string str2 = "select ?s ?o where { ?s " + this->kvstore->getPredicateByID(pid2) + " ?o . }";
	this->query(str2, rs2);
}

bool
Database::load(bool loadCSR)
{

	if(this->if_loaded)
	{
		return true;
	}

	//TODO: acquire this arg from memory manager
	//BETTER: get return value from subthread(using ref or file as hub)
	unsigned vstree_cache = gstore::LRUCache::DEFAULT_CAPACITY;
	bool flag;

#ifndef THREAD_ON
	//flag = (this->vstree)->loadTree(vstree_cache);
	//if (!flag)
	//{
		//cout << "load tree error. @Database::load()" << endl;
		//return false;
	//}
	(this->kvstore)->open();
#else
	//thread vstree_thread(&Database::load_vstree, this, vstree_cache);
	int kv_mode = KVstore::READ_WRITE_MODE;
	thread entity2id_thread(&Database::load_entity2id, this, kv_mode);
	thread id2entity_thread(&Database::load_id2entity, this, kv_mode);
	thread literal2id_thread(&Database::load_literal2id, this, kv_mode);
	thread id2literal_thread(&Database::load_id2literal, this, kv_mode);
	thread predicate2id_thread(&Database::load_predicate2id, this, kv_mode);
#ifndef ONLY_READ
	thread id2predicate_thread(&Database::load_id2predicate, this, kv_mode);
#endif
	thread sub2values_thread(&Database::load_sub2values, this, kv_mode);
	thread obj2values_thread(&Database::load_obj2values, this, kv_mode);
	thread pre2values_thread(&Database::load_pre2values, this, kv_mode);
#endif

	//this is very fast
	flag = this->loadDBInfoFile();
	if (!flag)
	{
		cout << "load database info error. @Database::load()" << endl;
		return false;
	}
	cout << "load database info successfully!" << endl;
	if(!(this->kvstore)->load_trie(kv_mode))
	{
		cout<<"load kvstore failed."<<endl;
         return false;

	}
	else{
		cout<<"load kvstore successfully!"<<endl;
	}


	//this->stringindex->SetTrie(this->kvstore->getTrie());
	//cout<<"set Trie for stringIndex  successfully!"<<endl;
	//NOTICE: we should also run some heavy work in the main thread
	this->stringindex->load();
	cout<<"load stringindex  successfully!"<<endl;
	this->readIDinfo();
	cout<<"read IDInfo file  successfully!"<<endl;

#ifdef THREAD_ON
	pre2values_thread.join();
#endif
	this->setPreMap();
	//cout<<"set Pre Map Successfully!"<<endl;

#ifdef THREAD_ON
	id2entity_thread.join();
	//cout<<"id2entity_thread.join() Successfully!"<<endl;
	id2literal_thread.join();
	//cout<<"id2literal_thread.join() Successfully!"<<endl;
#endif

	//BETTER: if we set string buffer using string index instead of B+Tree, then we can
	//avoid to load id2entity and id2literal in ONLY_READ mode

	//generate the string buffer for entity and literal, no need for predicate
	//NOTICE:the total string size should not exceed 20G, assume that most strings length < 500
	//too many empty between entity and literal, so divide them

	//this->setStringBuffer();

	//NOTICE: we should build string buffer from kvstore, not string index
	//Because when searching in string index, it will first check if in buffer(but the buffer is being built)

#ifndef ONLY_READ
#ifdef THREAD_ON
	id2predicate_thread.join();
	//cout<<"id2predicate_thread.join() Successfully!"<<endl;
#endif
#endif

#ifdef THREAD_ON
	entity2id_thread.join();
	cout<<"entity2id_thread.join() Successfully!"<<endl;
	literal2id_thread.join();
	cout<<"literal2id_thread.join() Successfully!"<<endl;
	predicate2id_thread.join();
	cout<<"predicate2id_thread.join() Successfully!"<<endl;
	sub2values_thread.join();
	cout<<"sub2values_thread.join() Successfully!"<<endl;
	obj2values_thread.join();
	cout<<"obj2values_thread.join() Successfully!"<<endl;
	//wait for vstree thread
	//vstree_thread.join();
#endif
	//load cache of sub2values and obj2values
	cout<<"begin load cache!"<<endl;
	this->load_cache();
	cout<<"load cache successfully!"<<endl;
	//warm up always as finishing build(), to utilize the system buffer
	//this->warmUp();
	//DEBUG:the warmUp() calls query(), which will also output results, this is not we want

	// Load trie

	/*if (trie != NULL)
		delete trie;
	trie = new Trie;

	string dictionary_path = store_path + "/dictionary.dc";
	if (!trie->LoadTrie(dictionary_path))
	{
		return false;
	}*/

	// long t1 = Util::get_cur_time();
    // this->load_statistics();
    // long t2 = Util::get_cur_time();
    // cout << "load statistics, used " << (t2 - t1) << "ms." << endl;

    this->if_loaded = true;

	cout << "finish load" << endl;

	//BETTER: for only-read application(like endpoint), 3 id2values trees can be closed now
	//and we should load all trees on only READ mode

	//HELP: just for checking infos(like kvstore)
	check();

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
		cout<<"pre_num: "<<pre_num<<endl;
		long begin_time = Util::get_cur_time();

		// Process out-edges (csr[0])
		// i: predicate; j: subject; k: object
		for(unsigned i = 0;i<pre_num;i++)
		{
			string pre = (this->getKVstore())->getPredicateByID(i);
			cout<<"pid: "<<i<<"    pre: "<<pre<<endl;
			unsigned* sublist = NULL;
			unsigned sublist_len = 0;
			bool ret = (this->getKVstore())->getsubIDlistBypreID(i, sublist, sublist_len, true);
			//cout<<"    sub_num: "<<sublist_len<<endl;
			unsigned offset = 0;
			unsigned index = 0;
			for(unsigned j=0;j<sublist_len;j++)
			{
				string sub = (this->getKVstore())->getEntityByID(sublist[j]);
				//cout<<"    sid: "<<sublist[j]<<"    sub: "<<sub<<endl;
				unsigned* objlist = NULL;
				unsigned objlist_len = 0;
				bool ret = (this->getKVstore())->getobjIDlistBysubIDpreID(sublist[j], i, objlist, objlist_len); 
				unsigned len = objlist_len;	// the real object list length
				for(unsigned k=0;k<objlist_len;k++)
				{
					if(objlist[k]>=2000000000)
					{
						--len;
						continue;
					}
					string obj = (this->getKVstore())->getEntityByID(objlist[k]);
					//cout<<"        oid: "<<objlist[k]<<"    obj: "<<obj<<endl;
					this->csr[0].adjacency_list[i].push_back(objlist[k]);
				}
				//cout<<"        obj_num: "<<len<<endl;
				if(len > 0)
				{
					this->csr[0].id2vid[i].push_back(sublist[j]);
					this->csr[0].vid2id[i].insert(pair<unsigned, unsigned>(sublist[j], index));
					this->csr[0].offset_list[i].push_back(offset);
					index++;
					offset += len;
				}
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
			cout<<this->csr[0].offset_list[i].size()<<endl;	// # of this predicate's subjects
			cout<<this->csr[0].adjacency_list[i].size()<<endl;	// # of this predicate's objects
		}

		// Process out-edges (csr[1])
		// i: predicate; j: object; k: subject
		for(unsigned i = 0;i<pre_num;i++)
		{
			string pre = (this->getKVstore())->getPredicateByID(i);
			cout<<"pid: "<<i<<"    pre: "<<pre<<endl;
			unsigned* objlist = NULL;
			unsigned objlist_len = 0;
			bool ret = (this->getKVstore())->getobjIDlistBypreID(i, objlist, objlist_len, true);
			//cout<<"    obj_num: "<<objlist_len<<endl;
			unsigned offset = 0;
			unsigned index = 0;
			for(unsigned j=0;j<objlist_len;j++)
			{
				if(objlist[j]>=2000000000)
					continue;
				string obj = (this->getKVstore())->getEntityByID(objlist[j]);
				//cout<<"    oid: "<<objlist[j]<<"    obj: "<<obj<<endl;
				unsigned* sublist = NULL;
				unsigned sublist_len = 0;
				bool ret = (this->getKVstore())->getsubIDlistByobjIDpreID(objlist[j], i, sublist, sublist_len); 
				unsigned len = sublist_len;
				for(unsigned k=0;k<sublist_len;k++)
				{
					string sub = (this->getKVstore())->getEntityByID(sublist[k]);
					//cout<<"        sid: "<<sublist[k]<<"    sub: "<<sub<<endl;
					this->csr[1].adjacency_list[i].push_back(sublist[k]);
				}
				//cout<<"        sub_num: "<<len<<endl;
				if(len > 0)
				{
					this->csr[1].id2vid[i].push_back(objlist[j]);
					this->csr[1].vid2id[i].insert(pair<unsigned, unsigned>(objlist[j], index));
					this->csr[1].offset_list[i].push_back(offset);
					index++;
					offset += len;
				}
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
			cout<<this->csr[1].offset_list[i].size()<<endl;
			cout<<this->csr[1].adjacency_list[i].size()<<endl;
		}
		long end_time = Util::get_cur_time();
		cout << "after creating CSR, used " << (end_time - begin_time) << "ms" << endl;
		cout << "CSR size = " << csr[0].sizeInBytes() + csr[1].sizeInBytes() << " (bytes)" << endl;
	}

	//load the statistics file of db
	this->loadStatisticsInfoFile();
	return true;
}

void
Database::load_cache()
{
	// get important pre ID
	// a pre whose degree is more than 50% of max pre degree is important pre
	cout << "get important pre ID" << endl;
	this->get_important_preID();
	cout << "total preID num is " << pre_num << endl;
	cout << "important pre ID is: ";
	for(int i = 0; i < important_preID.size(); ++i)
		cout << important_preID[i] << ' ';
	cout << endl;
	this->load_candidate_pre2values();
	this->load_important_sub2values();
	this->load_important_obj2values();
	
	long t0 = Util::get_cur_time();
	if(this->stringindex==NULL)
	{
		cout<<"the string index is null"<<endl;
	}
	vector<StringIndexFile*> indexfile = this->stringindex->get_three_StringIndexFile();

	StringIndexFile* 	entity = indexfile[0];
	StringIndexFile* 	literal = indexfile[1];
	StringIndexFile* 	predicate = indexfile[2];

	struct stat statbuf;
	int fd;
	char tmp;
	long end;
	
	stat((entity->get_loc() + "value").c_str(), &statbuf);
	fd = open((entity->get_loc() + "value").c_str(), O_RDONLY);
	entity->mmapLength = (statbuf.st_size/4096 + 1)*4096;
	entity->Mmap = (char*)mmap(NULL, entity->mmapLength, PROT_READ, MAP_POPULATE|MAP_SHARED, fd, 0);
	close(fd);
	end = entity->mmapLength - 4096;
	for (long off = 0; off < end; off += 4096)
	{	
		tmp = entity->Mmap[off];
	}
	stat((literal->get_loc() + "value").c_str(), &statbuf);
	fd = open((literal->get_loc() + "value").c_str(), O_RDONLY);
	literal->mmapLength = (statbuf.st_size / 4096 + 1) * 4096;
	literal->Mmap = (char*)mmap(NULL, literal->mmapLength, PROT_READ, MAP_POPULATE | MAP_SHARED , fd, 0);
	close(fd);
	end = literal->mmapLength - 4096;
	for (long off = 0; off < end; off += 4096)
	{
		tmp = literal->Mmap[off];
	}
	stat((predicate->get_loc() + "value").c_str(), &statbuf);
	fd = open((predicate->get_loc() + "value").c_str(), O_RDONLY);
	predicate->mmapLength = (statbuf.st_size / 4096 + 1) * 4096;
	predicate->Mmap = (char*)mmap(NULL, predicate->mmapLength, PROT_READ, MAP_POPULATE | MAP_SHARED, fd, 0);
	close(fd);
	end = predicate->mmapLength - 4096;
	for (long off = 0; off < end; off += 4096)
	{
		tmp = predicate->Mmap[off];
	}
	cout << "Value File Preload used " << Util::get_cur_time() - t0 << " ms" << endl;
	/*
	cerr << "Get in" << endl;
	{
		pid_t p = getpid();
		char file[64] = { 0 };//�ļ���
		FILE *fd;         //�����ļ�ָ��fd
		char line_buff[256] = { 0 };  //��ȡ�еĻ�����
		sprintf(file, "/proc/%d/status", p);
		fprintf(stderr, "current pid:%d\n", p);
		fd = fopen(file, "r"); //��R���ķ�ʽ���ļ��ٸ���ָ��fd

		int i; //��ȡvmrss:ʵ�������ڴ�ռ��
		char name[32];//�����Ŀ����
		int vmrss;//����ڴ�
		for (i = 0; i<17; i++)  //��ȡVmRSS��һ�е�����
		{
			char* ret = fgets(line_buff, sizeof(line_buff), fd);
			if (i == 11 || i == 12 || i == 15 || i == 16)
			{
				sscanf(line_buff, "%s %d", name, &vmrss);
				fprintf(stderr, "%s\t%d kb\n", name, vmrss);
			}
		}
		fclose(fd);     //�ر��ļ�fd
	}*/
}

void
Database::get_important_preID()
{
	important_preID.clear();
	unsigned max_degree = 0;
	for(TYPE_PREDICATE_ID i = 0; i < limitID_predicate; ++i)
		if (pre2num[i] > max_degree)
			max_degree = pre2num[i];
	unsigned limit_degree = max_degree / 2;
	for(TYPE_PREDICATE_ID i = 0; i < limitID_predicate; ++i)
		if (pre2num[i] > limit_degree)
			important_preID.push_back(i);
}

void
Database::load_important_obj2values()
{
	cout << "get important objID..." << endl;
	this->get_important_objID();

	this->build_CacheOfObj2values();
}
void
Database::load_important_sub2values()
{
	cout << "get important subID..." << endl;
	this->get_important_subID();

	this->build_CacheOfSub2values();
}

void 
Database::load_candidate_pre2values()
{
	cout << "get candidate preID..." << endl;
	this->get_candidate_preID();

	this->build_CacheOfPre2values();
}

void
Database::get_candidate_preID()
{
	//cout << "now add cache of preID2values..." << endl;
	/*for(int i = 0; i < important_preID.size(); ++i)
	{
		unsigned _size = this->kvstore->getPreListSize(important_preID[i]);
		if (now_size + _size >= max_total_size) continue;
		now_size += _size;
		this->kvstore->AddIntoPreCache(important_preID[i]);
	}*/
	unsigned now_total_size = 0;
	const unsigned max_total_size = 2000000000;//2G
//	std::priority_queue <KEY_SIZE_VALUE> candidate_preID;
	std::priority_queue <KEY_SIZE_VALUE, deque<KEY_SIZE_VALUE>, greater<KEY_SIZE_VALUE> > rubbish;
	while(!rubbish.empty()) rubbish.pop();
	while(!candidate_preID.empty()) candidate_preID.pop();
	for(TYPE_PREDICATE_ID i = 0; i < limitID_predicate; ++i)
	{
		unsigned _value = 0;
		unsigned _size;
		unsigned long _tmp_size;
		_tmp_size = this->kvstore->getPreListSize(i);
		if (_tmp_size > (1 << 31)) continue;
		_size = (unsigned)_tmp_size;
		if (!VList::isLongList(_size) || _size >= max_total_size) continue; // only long list need to be stored in cache

		_value = pre2num[i];
		if (_value == 0) continue;

		if (_size + now_total_size < max_total_size)
		{
			candidate_preID.push(KEY_SIZE_VALUE(i, _size, _value));
			now_total_size += _size;
		}
		else
		{
			if (candidate_preID.empty()) continue;
			if (_value > candidate_preID.top().value)
			{
				while (now_total_size + _size >= max_total_size)
				{
					if (candidate_preID.top().value >= _value) break;
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
	cout << "finish getting candidate preID, the size is " << now_total_size << endl;
}

void
Database::build_CacheOfPre2values()
{
	cout << "now add cache of preID2values..." << endl;
	while (!candidate_preID.empty())
	{
		this->kvstore->AddIntoPreCache(candidate_preID.top().key);
		candidate_preID.pop();
	}
}

void
Database::build_CacheOfObj2values()
{
	cout << "now add cache of objID2values..." << endl;
	while (!important_objID.empty())
	{
		this->kvstore->AddIntoObjCache(important_objID.top().key);
		important_objID.pop();
	}
}

void
Database::build_CacheOfSub2values()
{
	cout << "now add cache of subID2values..." << endl;
	while (!important_subID.empty())
	{
		this->kvstore->AddIntoSubCache(important_subID.top().key);
		important_subID.pop();
	}
}

void
Database::get_important_subID()
{
	while(!important_subID.empty()) important_subID.pop();
	unsigned now_total_size = 0;
	const string invalid = "";
	const unsigned max_total_size = 2000000000;//2G
	std::priority_queue <KEY_SIZE_VALUE, deque<KEY_SIZE_VALUE>, greater<KEY_SIZE_VALUE> > rubbish;
	while(!rubbish.empty()) rubbish.pop();
	// a sub who has largest degree with important pre is important subs
	for(TYPE_ENTITY_LITERAL_ID i = 0; i < limitID_entity; ++i)
	{
		unsigned _value = 0;
		unsigned long _tmp_size = 0;
		if (this->kvstore->getEntityByID(i) == invalid) continue;	
		_tmp_size = this->kvstore->getSubListSize(i);
		if (_tmp_size >= (1 << 31)) continue;
		unsigned _size = (unsigned)_tmp_size;
		if (!VList::isLongList(_size) || _size >= max_total_size) continue; // only long list need to be stored in cache

		for(unsigned j = 0; j < important_preID.size(); ++j)
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
			if (important_subID.empty()) continue;
			if (_value > important_subID.top().value)
			{
				while (now_total_size + _size >= max_total_size)
				{
					if (important_subID.top().value >= _value) break;
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
	cout << "finish getting important subID, the cache size is " << now_total_size << endl;
}

void
Database::get_important_objID()
{
	while(!important_objID.empty()) important_objID.pop();
	unsigned now_total_size = 0;
	const unsigned max_total_size = 2000000000;//2G
	const string invalid = "";
	std::priority_queue <KEY_SIZE_VALUE, deque<KEY_SIZE_VALUE>, greater<KEY_SIZE_VALUE> > rubbish;
	while(!rubbish.empty()) rubbish.pop();
	// a sub who has largest degree with important pre is important subs
	for(TYPE_ENTITY_LITERAL_ID i = 0; i < limitID_literal; ++i)
	{
		unsigned _value = 0;
		unsigned _size;
		string _tmp;
		if (i < limitID_entity) _tmp = this->kvstore->getEntityByID(i);
		else _tmp = this->kvstore->getLiteralByID(i);
		if (_tmp == invalid) continue;

		unsigned long _tmp_size = 0;
		_tmp_size = this->kvstore->getSubListSize(i);
		
		_tmp_size = this->kvstore->getObjListSize(i);
		if (_tmp_size >= (1 << 31)) continue;
		_size = (unsigned)_tmp_size;

		if (!VList::isLongList(_size) || _size >= max_total_size) continue; // only long list need to be stored in cache
		
		for(unsigned j = 0; j < important_preID.size(); ++j)
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
			if (important_objID.empty()) continue;
			if (_value > important_objID.top().value)
			{
				while (now_total_size + _size >= max_total_size)
				{
					if (important_objID.top().value >= _value) break;
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
	cout << endl;
	cout << "finish getting important objID, the cache size is " << now_total_size << endl;
}

void 
Database::load_entity2id(int _mode)
{
	cout<<"begin to load_entity2id!"<<endl;
	this->kvstore->open_entity2id(_mode);
	cout<<"load_entity2id successfully!"<<endl;
}

void 
Database::load_id2entity(int _mode)
{
	cout<<"begin to load_id2entity!"<<endl;
	this->kvstore->open_id2entity(_mode);
	cout<<"load_id2entity successfully!"<<endl;
}

void 
Database::load_literal2id(int _mode)
{
	cout<<"begin to load_literal2id!"<<endl;
	this->kvstore->open_literal2id(_mode);
	cout<<"load_literal2id successfully!"<<endl;
}

void 
Database::load_id2literal(int _mode)
{
	cout<<"begin to load_id2literal!"<<endl;
	this->kvstore->open_id2literal(_mode);
	cout<<"load_id2literal successfully!"<<endl;
}

void 
Database::load_predicate2id(int _mode)
{
	cout<<"begin to load_predicate2id!"<<endl;
	this->kvstore->open_predicate2id(_mode);
	cout<<"load_predicate2id successfully!"<<endl;
}

void 
Database::load_id2predicate(int _mode)
{
	cout<<"begin to load_id2predicate!"<<endl;
	this->kvstore->open_id2predicate(_mode);
	cout<<"load_id2predicate successfully!"<<endl;
}

void 
Database::load_sub2values(int _mode)
{
	cout<<"begin to load_sub2values!"<<endl;
	this->kvstore->open_subID2values(_mode);
	cout<<"load_sub2values successfully!"<<endl;
}

void 
Database::load_obj2values(int _mode)
{
	cout<<"begin to load_obj2values!"<<endl;
	this->kvstore->open_objID2values(_mode);
	cout<<"load_obj2values successfully!"<<endl;
}

void 
Database::load_pre2values(int _mode)
{
	cout<<"begin to load_pre2values!"<<endl;
	this->kvstore->open_preID2values(_mode);
	cout<<"load_pre2values successfully!"<<endl;
}

// void 
// Database::load_vstree(unsigned _vstree_size)
// {
// 	(this->vstree)->loadTree(_vstree_size);
// 	cout<<"vstree loaded"<<endl;
// }

// @author bookug
// @email bookug@qq.com
// @function check some parameters, statues and correctness of the database
void 
Database::check()
{
cout<<"triple num: "<<this->triples_num<<endl;
cout<<"pre num: "<<this->pre_num<<endl;
cout<<"entity num: "<<this->entity_num<<endl;
cout<<"literal num: "<<this->literal_num<<endl;

string tstr;
//unsigned lid1 = this->kvstore->getIDByLiteral("\"111\"");
//cout<<"check: "<<lid1<<endl;
//unsigned lid2 = this->kvstore->getIDByLiteral("\"222\"");
//cout<<"check: "<<lid2<<endl;
//unsigned lid3 = this->kvstore->getIDByLiteral("\"Bookug Lobert\"");
//cout<<"check: "<<lid3<<endl;
//unsigned eid = this->kvstore->getIDByEntity("<bookug>");
//cout<<"check: "<<eid<<endl;
//tstr = this->kvstore->getLiteralByID(2000000004);
//cout<<"check: "<<tstr<<endl;
//tstr = this->kvstore->getEntityByID(14);
//cout<<"check: "<<tstr<<endl;

 //unsigned pid = this->kvstore->getIDByPredicate("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>");
 //cout<<"check: pre "<<pid<<endl;
 //this->stringindex->randomAccess(pid, &tstr, false);
 //cout<<"string index: "<<tstr<<endl;
 ////cout<<"kvstore: "<<this->kvstore->getPredicateByID(pid)<<endl;

	//cout<<"right pair: "<<62<<" "<<"<http://www.Department0.University0.edu/GraduateCourse11>"<<endl;
 //unsigned sid = this->kvstore->getIDByEntity("<http://www.Department0.University0.edu/GraduateCourse11>");
 //cout<<"check: sub "<<sid<<endl;
 //this->stringindex->randomAccess(sid, &tstr, true);
 //cout<<"string index: "<<tstr<<endl;
 //cout<<"kvstore: "<<this->kvstore->getEntityByID(sid)<<endl;

 //unsigned oid = this->kvstore->getIDByString("<http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Course>");
 //cout<<"check: obj "<<oid<<endl;
 //this->stringindex->randomAccess(oid, &tstr, true);
 //cout<<"string index: "<<tstr<<endl;
 //cout<<"kvstore: "<<this->kvstore->getEntityByID(oid)<<endl;

 //unsigned* list = NULL; unsigned len = 0;
 //this->kvstore->getsubIDlistByobjIDpreID(oid, pid, list, len);
 //FILE* fp = fopen("kv.txt", "w");
//for(unsigned i = 0; i < len; ++i)
//{
	//fprintf(fp, "%u\n", list[i]);
	//string ts;
	//if(Util::is_literal_ele(list[i]))
	//ts = this->kvstore->getLiteralByID(list[i]);
	//else
	//ts = this->kvstore->getEntityByID(list[i]);
	//if(ts == "")
	//{
			//fprintf(fp, "Error in id2string\n");
	//}
	//else
	//{
			//fprintf(fp, "%s\n", ts.c_str());
	//}
	//this->stringindex->randomAccess(list[i], &tstr, true);
	//fprintf(fp, "string index: %s\n", ts.c_str());
//}
//this->stringindex->randomAccess(86006539, &tstr, true);
//cout<<"check: 86006539 "<<tstr<<endl;
//cout<<this->kvstore->getStringByID(86006539)<<endl;
//this->stringindex->randomAccess(82855205, &tstr, true);
//cout<<this->kvstore->getStringByID(82855205)<<endl;
//cout<<"check: 82855205 "<<tstr<<endl;
//fclose(fp);



//test String Index for parallism
//int limit = 2;
//int limit = this->entity_num / 2;
//thread* thr_si = new thread[limit];
//for(int i = 0; i < limit; ++i)
//{
	//thr_si[i] = thread(&Database::query_stringIndex, this, i);
//}
//for(int i = 0; i < limit; ++i)
//{
	//thr_si[i].join();
//}
//delete[] thr_si;

	string spq[6];
	spq[0] = "select ?x where { ?x <ub:name> <FullProfessor0> . }";
	spq[1] = "select distinct ?x where { ?x      <rdf:type>      <ub:GraduateStudent>. ?y      <rdf:type>      <ub:University>. ?z      <rdf:type>      <ub:Department>. ?x      <ub:memberOf>   ?z. ?z      <ub:subOrganizationOf>  ?y. ?x      <ub:undergraduateDegreeFrom>    ?y. }";
	spq[2] = "select distinct ?x where { ?x      <rdf:type>      <ub:Course>. ?x      <ub:name>       ?y. }";
	spq[3] = "select ?x where { ?x    <rdf:type>    <ub:UndergraduateStudent>. ?y    <ub:name> <Course1>. ?x    <ub:takesCourse>  ?y. ?z    <ub:teacherOf>    ?y. ?z    <ub:name> <FullProfessor1>. ?z    <ub:worksFor>    ?w. ?w    <ub:name>    <Department0>. }";
		spq[4] = "select distinct ?x where { ?x    <rdf:type>    <ub:UndergraduateStudent>. }";
	spq[5] = "select ?s ?o where { ?s ?p ?o . }";
	for(int i = 0; i < 6; ++i)
	{
		//NOTICE: we need to detach it, otherwise the thread object will be released beyond the scope,
		//so the thread ends causing an exception
		//thread qt(&Database::query_thread, this, spq[i]);
		//qt.detach();
	}
	//cout<<"this function ends!"<<endl;
	//WARN: if each threda for a query, then the QueryParser will cause error in parallism!
	//so we should do the parser sequentially
}

void 
Database::query_stringIndex(int id)
{
	string str;
	this->stringindex->randomAccess(id, &str, true);
	cout<<"thread: "<<id<<" "<<str<<endl;
}

//NOTICE: we ensure that if the unload() exists normally, then all updates have already been written to disk
//So when accidents happens, we only have to restore the databases that are in load status(inlucidng that unload
//not finished) later.
bool
Database::unload()
{
	//TODO: do we need to update the pre2num if update queries exist??
	//or we just neglect this, that is ok because pre2num is just used to count
	//cout << "delete pre2num" << endl;
	delete[] this->pre2num;
	this->pre2num = NULL;
	delete[] this->pre2sub;
	this->pre2sub = NULL;
	delete[] this->pre2obj;
	this->pre2obj = NULL;
	//cout << "delete entity buffer" << endl;
	delete this->entity_buffer;
	this->entity_buffer = NULL;
	//cout << "delete literal buffer" << endl;
	delete this->literal_buffer;
	this->literal_buffer = NULL;

	delete this->kvstore;
	this->kvstore = NULL;
	//cout << "delete stringindex" << endl;
	delete this->stringindex;
	this->stringindex = NULL;

	//this->vstree->saveTree();
	//delete this->vstree;
	//this->vstree = NULL;
	//cout << "delete kvstore" << endl;
	if (if_loaded)
	{
		this->saveDBInfoFile();
		//this->saveStatisticsInfoFile();
		this->writeIDinfo();
		this->initIDinfo();
	}

	this->if_loaded = false;
	this->clear_update_log();

	/*if (this->trie != NULL)
	{
		delete this->trie;
		trie = NULL;
	}*/

	return true;
}

//this is used for checkpoint, we must ensure that modification is written to disk,
//so flush() is a must
bool Database::save()
{
	//this->vstree->saveTree();
	this->kvstore->flush();
	this->saveDBInfoFile();
	this->saveIDinfo();

	this->stringindex->flush();
	this->clear_update_log();

	//cerr<<"database checkpoint: "<<this->getName()<<endl;

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

	//delete this->vstree;
	//this->vstree = NULL;
	delete this->kvstore;
	this->kvstore = NULL;
	delete this->stringindex;
	this->stringindex = NULL;
}

string
Database::getName()
{
	return this->name;
}

TYPE_TRIPLE_NUM 
Database::getTripleNum()
{
	return this->triples_num;
}

TYPE_ENTITY_LITERAL_ID 
Database::getEntityNum()
{
	return this->entity_num;
}

TYPE_ENTITY_LITERAL_ID 
Database::getLiteralNum()
{
	return this->literal_num;
}

TYPE_ENTITY_LITERAL_ID 
Database::getSubNum()
{
	return this->sub_num;
}

TYPE_PREDICATE_ID 
Database::getPreNum()
{
	return this->pre_num;
}

// VSTree*
// Database::getVSTree()
// {
// 	return this->vstree;
// }

KVstore*
Database::getKVstore()
{
	return this->kvstore;
}

StringIndex*
Database::getStringIndex()
{
	return this->stringindex;
}

QueryCache*
Database::getQueryCache()
{
	return this->query_cache;
}

TYPE_TRIPLE_NUM*
Database::getpre2num()
{
	return this->pre2num;
}

TYPE_TRIPLE_NUM*
Database::getpre2sub()
{
	return this->pre2sub;
}

TYPE_TRIPLE_NUM*
Database::getpre2obj()
{
	return this->pre2obj;
}

TYPE_ENTITY_LITERAL_ID&
Database::getlimitID_literal()
{
	return this->limitID_literal;
}

TYPE_ENTITY_LITERAL_ID&
Database::getlimitID_entity()
{
	return this->limitID_entity;
}

TYPE_PREDICATE_ID&
Database::getlimitID_predicate()
{
	return this->limitID_predicate;
}

mutex&
Database::get_query_parse_lock()
{
	//return this->query_parse_lock;
}

void
Database::export_db(FILE* fp)
{
	unsigned* id_list = NULL;
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
  			if(id_list[j + 1] >= Util::LITERAL_FIRST_ID)
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

int
Database::query(const string _query, ResultSet& _result_set, FILE* _fp, bool update_flag, bool export_flag, shared_ptr<Transaction> txn)
{
	string dictionary_store_path = this->store_path + "/dictionary.dc"; 	

	this->stringindex->SetTrie(this->kvstore->getTrie());
	GeneralEvaluation general_evaluation(this->kvstore, this->statistics, this->stringindex, this->query_cache, \
		this->pre2num, this->pre2sub, this->pre2obj, this->triples_num, this->limitID_predicate, this->limitID_literal, \
		this->limitID_entity, this->csr, txn);
	if(txn != nullptr)
	cout << "query in transaction............................................" << endl;
	long tv_begin = Util::get_cur_time();

	//this->query_parse_lock.lock();
	bool parse_ret=false;
	try
	{
		/* code */
		parse_ret = general_evaluation.parseQuery(_query);
	}
	catch(const std::runtime_error& e2)
	{
		cout<<"catch run_time error exception"<<endl;
		throw std::runtime_error(e2.what());
		std::cerr<<e2.what()<<"\n";
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}



	//this->query_parse_lock.unlock();
	if (!parse_ret)
		return -101;
	long tv_parse = Util::get_cur_time();
	cout << "after Parsing, used " << (tv_parse - tv_begin) << "ms." << endl;
	// cout << "QueryTree:" << endl;
	// general_evaluation.getQueryTree().print();
	//return -100;

	//for select, -100 by default, -101 means error
	//for update, non-negative means true(and the num is updated triples num), -1 means error
	int success_num = -100;  
	bool need_output_answer = false;
	
	//Query
	if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Not_Update)
	{
		//lock_guard<mutex> (this->update_lock);  //when quit this scope the lock will be released
		//unique_guard<mutex> updateLCK(this->update_lock);
		//if(!unique_guard.try_lock())
		//BETTER: use timed lock
		if(txn == nullptr && pthread_rwlock_tryrdlock(&(this->update_lock)) != 0)
		{
			return -101;
		}
		if(txn == nullptr)
			cout<<"read priviledge of update lock acquired"<<endl;

		//copy the string index for each query thread
		//StringIndex tmpsi = *this->stringindex;
		//tmpsi.emptyBuffer();
		//general_evaluation.setStringIndexPointer(&tmpsi);
	
	//	this->debug_lock.lock();
		if(export_flag)
		{
			general_evaluation.fp = _fp;
			general_evaluation.export_flag = export_flag;
		}

		long t1 = Util::get_cur_time();
		bool query_ret = general_evaluation.doQuery();
		long t2 = Util::get_cur_time();
		cout << "GeneralEvaluation::doQuery used " << (t2 - t1) << "ms." << endl;

		if(!query_ret)
		{
			success_num = -101;
		}
	//	this->debug_lock.unlock();

		long tv_bfget = Util::get_cur_time();
		//NOTICE: this lock lock ensures that StringIndex is visited sequentially
		//this->getFinalResult_lock.lock();
		/*
		if (trie == NULL)
		{
			trie = new Trie;
			string dictionary_path = this->store_path + "/dictionary.dc";
			if (!trie->LoadTrie(dictionary_path))
			{
				exit(0);
			}
			trie->LoadDictionary();
		}*/
		general_evaluation.getFinalResult(_result_set);
		//this->getFinalResult_lock.unlock();
		long tv_afget = Util::get_cur_time();
		cout << "during getFinalResult, used " << (tv_afget - tv_bfget) << "ms." << endl;

		if(_fp != NULL)
			need_output_answer = true;
			//general_evaluation.setNeedOutputAnswer();

		//tmpsi.clear();
		if(txn == nullptr)
			pthread_rwlock_unlock(&(this->update_lock));
	}
	//Update 
	else
	{
		if(update_flag == 0)
		{
			//if update_flag == 0, means no privilege to do update query, so we throw an error.
			string exception_msg = "no update prvilege, update query failed.";
			cout << exception_msg << endl;
			throw exception_msg;
		}
#ifdef ONLY_READ
		cout<<"this database is only read";
		//invalid query because updates are not allowed in ONLY_READ mode
		return -101;
#endif
		if(txn == nullptr && pthread_rwlock_trywrlock(&(this->update_lock)) != 0)
		{
			cout<<"unable to write lock"<<endl;
			return -101;
		}
		if(txn == nullptr)
			cout<<"write priviledge of update lock acquired"<<endl;

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
			QueryTree::GroupPattern &update_pattern = general_evaluation.getQueryTree().getUpdateType() == QueryTree::Insert_Data ?
				general_evaluation.getQueryTree().getInsertPatterns() : general_evaluation.getQueryTree().getDeletePatterns();

			update_triple_num = update_pattern.sub_group_pattern.size();
			update_triple = new TripleWithObjType[update_triple_num];

			for (TYPE_TRIPLE_NUM i = 0; i < update_triple_num; i++)
				if (update_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
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
					if(txn == nullptr)
					{
						pthread_rwlock_unlock(&(this->update_lock));
						throw "Database::query failed";
					}
				}

			if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Insert_Data)
			{
				success_num = insert(update_triple, update_triple_num, false, txn);
			}
			else if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Delete_Data)
			{
				success_num = remove(update_triple, update_triple_num, false, txn);
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
				//for(int i = 0; i < update_triple_num; i++)
				//{
					//update_triple[i] = trie->Compress(update_triple[i], Trie::QUERYMODE);
				//}
				success_num = remove(update_triple, update_triple_num, false, txn);

			}
			if (general_evaluation.getQueryTree().getUpdateType() == QueryTree::Insert_Clause || general_evaluation.getQueryTree().getUpdateType() == QueryTree::Modify_Clause)
			{
				general_evaluation.prepareUpdateTriple(general_evaluation.getQueryTree().getInsertPatterns(), update_triple, update_triple_num);
				//for(int i = 0; i < update_triple_num; i++)
				//{
					//update_triple[i] = trie->Compress(update_triple[i], Trie::QUERYMODE);
				//}
				success_num = insert(update_triple, update_triple_num, false, txn);
			}
		}

		general_evaluation.releaseResult();
		delete[] update_triple;

		//NOTICE: maybe no updates are really done!
		if(success_num > 0 && query_cache != nullptr)
		{
			this->query_cache->clear();
			cout<<"QueryCache cleared"<<endl;
		}
		if(txn == nullptr)
			pthread_rwlock_unlock(&(this->update_lock));
	}

	long tv_final = Util::get_cur_time();
	cout << "Query time used (minus parsing): " << tv_final - tv_parse << "ms." << endl;
	cout << "Total time used: " << (tv_final - tv_begin) << "ms." << endl;
	//if (general_evaluation.needOutputAnswer())
	// if(export_flag)
	// {
		if (need_output_answer)
		{
			long long ans_num = max((long long)_result_set.ansNum - _result_set.output_offset, 0LL);
			if (_result_set.output_limit != -1)
				ans_num = min(ans_num, (long long)_result_set.output_limit);
			cout << "There has answer: " << ans_num << endl;
			cout << "final result is : " << endl;
			_result_set.output(_fp);
			fflush(_fp);       //to empty the output buffer in C (fflush(stdin) not work in GCC)
		}
	// }

#ifdef DEBUG
	cout<<"query success_num: "<<success_num<<endl;
#endif

	//cout<<"to check: "<<this->kvstore->getEntityByID(0)<<endl;
	return success_num;
}

//NOTICE+QUERY:to save memory for large cases, we can consider building one tree at a time(then release)
//Or read the rdf file on separate segments
//WARN:the ID type is int, and entity/literal are just separated by a limit
//which means that entity num <= 10^9 literal num <= 10^9 predicate num <= 2*10^9
//If we use unsigned as type, then max triple can be 10^9(edge case)
//If we use long long, no more problem, but wasteful
//Or we can consider divide entity and literal totally
//In distributed gStore, each machine's graph should be based on unique encoding IDs,
//and require that triples in each graph no more than a limit(maybe 10^9)

bool
Database::build(const string& _rdf_file, Socket& socket)
{
	this->resetIDinfo();

	string ret = Util::getExactPath(_rdf_file.c_str());
	long tv_build_begin = Util::get_cur_time();

	Util::create_dir(this->store_path);

	string kv_store_path = store_path + "/kv_store";
	Util::create_dir(kv_store_path);

	string stringindex_store_path = store_path + "/stringindex_store";
	Util::create_dir(stringindex_store_path);

	string update_log_path = this->store_path + '/' + this->update_log;
	Util::create_file(update_log_path);
	string update_log_since_backup = this->store_path + '/' + this->update_log_since_backup;
	Util::create_file(update_log_since_backup);

	string error_log = this->store_path + "/parse_error.log";
	Util::create_file(error_log);

	string msg = "begin encode RDF from : " + ret + " ...\n";
	cout << msg;
	string resJson = CreateJson(1, "building", msg);
	socket.send(resJson);

	if (!this->encodeRDF_new(ret, error_log))
	{
		return false;
	}
	msg = "finish encode.\n";
	cout << msg;
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
	cout << msg;
	resJson = CreateJson(1, "building", msg);
	socket.send(resJson);

	return true;
}

bool
Database::build(const string& _rdf_file)
{
	//NOTICE: it is not necessary to use multiple threads here, because some process may rely on others
	//In addition, the memory is a bootleneck and it is dangerous to build serveral indices at a time
	//For example, if we build id2string indices using different threads, they 
	//all have to scan the dataset and only use a part of the data, which may be costly
	//Besides, build process is already very fast, able to build freebase in 12h

	//manage the id for a new database
	this->resetIDinfo();

	string ret = Util::getExactPath(_rdf_file.c_str());
	long tv_build_begin = Util::get_cur_time();

	//string store_path = this->name;
	Util::create_dir(this->store_path);

	string kv_store_path = store_path + "/kv_store";
	Util::create_dir(kv_store_path);

	//string vstree_store_path = store_path + "/vs_store";
	//Util::create_dir(vstree_store_path);

	string stringindex_store_path = store_path + "/stringindex_store";
	Util::create_dir(stringindex_store_path);

	string update_log_path = this->store_path + '/' + this->update_log;
	Util::create_file(update_log_path);
	string update_log_since_backup = this->store_path + '/' + this->update_log_since_backup;
	Util::create_file(update_log_since_backup);
	//create a file to store the parse error tuple
	string error_log = this->store_path + "/parse_error.log";
	Util::create_file(error_log);
	cout << "begin encode RDF from : " << ret << " ..." << endl;

	//BETTER+TODO:now require that dataset size < memory
	//to support really larger datasets, divide and insert into B+ tree and VStree
	//(read the value, add list and set; update the signature, remove and reinsert)
	//the query process is nearly the same
	//QUERY:build each index in one thread to speed up, but the sorting case?
	//one sorting order for several indexes

	// to be switched to new encodeRDF method.
	//    this->encodeRDF(ret);
	if (!this->encodeRDF_new(ret,error_log))	//<-- this->kvstore->id2* trees are closed
	{
		return false;
	}
	cout << "finish encode." << endl;

	//cout<<"test kv"<<this->kvstore->getIDByPredicate("<contain>")<<endl;

	//this->kvstore->flush();
	delete this->kvstore;
	this->kvstore = NULL;
	//sync();
	//cout << "sync kvstore" << endl;
	//this->kvstore->release();
	//cout<<"release kvstore"<<endl;

	//long before_vstree = Util::get_cur_time();
	//(this->kvstore)->open();
	//string _entry_file = this->getSignatureBFile();

	//cout << "begin build VS-Tree on " << ret << "..." << endl;
	//NOTICE: we can use larger buffer for vstree in building process, because it does not compete with others
	//we only need to build vstree in this phase(no need for id tuples anymore)
	//TODO: acquire this arg from memory manager
	//unsigned vstree_cache_size = 4 * LRUCache::DEFAULT_CAPACITY;
	//BETTER: we should set the parameter according to current memory usage
	//(this->vstree)->buildTree(_entry_file, vstree_cache_size);

	long tv_build_end = Util::get_cur_time();

	//cout << "after build vstree, used " << (tv_build_end - before_vstree) << "ms." << endl;
	cout << "after build, used " << (tv_build_end - tv_build_begin) << "ms." << endl;
	cout << "finish build VS-Tree." << endl;

	cout << "finish sub2id pre2id obj2id" << endl;
	cout << "tripleNum is " << this->triples_num << endl;
	cout << "entityNum is " << this->entity_num << endl;
	cout << "preNum is " << this->pre_num << endl;
	cout << "literalNum is " << this->literal_num << endl;

	//this->vstree->saveTree();
	//delete this->vstree;
	//this->vstree = NULL;
	//sync();
	//cout << "sync vstree" << endl;

	//TODO: use fopen w+ to remove signature.binary file
	//string cmd = "rm -rf " + _entry_file;
	//system(cmd.c_str());
	//cout << "signature file removed" << endl;

	// this->if_loaded = true;
	this->saveDBInfoFile();
	this->writeIDinfo();
	this->initIDinfo();
	return true;
}

//root Path of this DB + sixTuplesFile
string
Database::getSixTuplesFile()
{
	return this->getStorePath() + "/" + this->six_tuples_file;
}

//root Path of this DB + signatureBFile 
// string
// Database::getSignatureBFile()
// {
// 	return this->getStorePath() + "/" + this->signature_binary_file;
// }

//root Path of this DB + DBInfoFile 
string
Database::getDBInfoFile()
{
	return this->getStorePath() + "/" + this->db_info_file;
}

void Database::setTypePredicateName(string& names)
{
	this->type_predicate_name=names;
}

bool Database::checkIsTypePredicate(string& predicate)
{
	vector<string> names;
	Util::split(this->type_predicate_name,"@@",names);
	for(int i=0;i<names.size();i++)
	{
       //cout<<"temp is "<<names[i]<<endl;
       if(Util::iscontain(predicate,names[i]))
	   {
		 //cout<<predicate<<" is contain "<<names[i]<<endl;
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

bool
Database::saveDBInfoFile()
{
	FILE* filePtr = fopen(this->getDBInfoFile().c_str(), "wb");
    //cout<<" save the db info file "<<endl;
	if (filePtr == NULL)
	{
		cout << "error, can not create db info file. @Database::saveDBInfoFile" << endl;
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

	//Util::triple_num = this->triples_num;
	//Util::pre_num = this->pre_num;
	//Util::entity_num = this->entity_num;
	//Util::literal_num = this->literal_num;

	return true;
}

bool
Database::loadDBInfoFile()
{
	FILE* filePtr = fopen(this->getDBInfoFile().c_str(), "rb");

	if (filePtr == NULL)
	{
		cout << "error, can not open db info file. @Database::loadDBInfoFile" << endl;
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

	//Util::triple_num = this->triples_num;
	//Util::pre_num = this->pre_num;
	//Util::entity_num = this->entity_num;
	//Util::literal_num = this->literal_num;

	return true;
}

string
Database::getStorePath()
{
	return this->store_path;
}

//encode relative signature data of the query graph
// void
// Database::buildSparqlSignature(SPARQLquery & _sparql_q)
// {
// 	vector<BasicQuery*>& _query_union = _sparql_q.getBasicQueryVec();
// 	for (unsigned i_bq = 0; i_bq < _query_union.size(); i_bq++)
// 	{
// 		BasicQuery* _basic_q = _query_union[i_bq];
// 		_basic_q->encodeBasicQuery(this->kvstore, _sparql_q.getQueryVar());
// 	}
// }

/*bool
Database::calculateEntityBitSet(TYPE_ENTITY_LITERAL_ID _entity_id, EntityBitSet & _bitset)
{
	unsigned _list_len = 0;
	//when as subject
	unsigned* _polist = NULL;
	(this->kvstore)->getpreIDobjIDlistBysubID(_entity_id, _polist, _list_len);
	//Triple _triple;
	//_triple.subject = (this->kvstore)->getEntityByID(_entity_id);
	for (unsigned i = 0; i < _list_len; i += 2)
	{
		TYPE_PREDICATE_ID _pre_id = _polist[i];
		TYPE_ENTITY_LITERAL_ID _obj_id = _polist[i + 1];
		//_triple.object = (this->kvstore)->getEntityByID(_obj_id);
		//if (_triple.object == "")
		//{
			//_triple.object = (this->kvstore)->getLiteralByID(_obj_id);
		//}
		//_triple.predicate = (this->kvstore)->getPredicateByID(_pre_id);
		//this->encodeTriple2SubEntityBitSet(_bitset, &_triple);
		this->encodeTriple2SubEntityBitSet(_bitset, _pre_id, _obj_id);
	}
	delete[] _polist;

	//when as object
	unsigned* _pslist = NULL;
	_list_len = 0;
	(this->kvstore)->getpreIDsubIDlistByobjID(_entity_id, _pslist, _list_len);
	//_triple.object = (this->kvstore)->getEntityByID(_entity_id);
	for (unsigned i = 0; i < _list_len; i += 2)
	{
		TYPE_PREDICATE_ID _pre_id = _pslist[i];
		TYPE_ENTITY_LITERAL_ID _sub_id = _pslist[i + 1];
		//_triple.subject = (this->kvstore)->getEntityByID(_sub_id);
		//_triple.predicate = (this->kvstore)->getPredicateByID(_pre_id);
		//this->encodeTriple2ObjEntityBitSet(_bitset, &_triple);
		this->encodeTriple2ObjEntityBitSet(_bitset, _pre_id, _sub_id);
	}
	delete[] _pslist;

	return true;
}
*/
//encode Triple into subject SigEntry
/*bool
Database::encodeTriple2SubEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple)
{
	TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_p_triple->predicate);
	if(_pre_id != INVALID_PREDICATE_ID)
	{
		Signature::encodePredicate2Entity(_bitset, _pre_id, Util::EDGE_OUT);
	}

	TYPE_ENTITY_LITERAL_ID _obj_id = (this->kvstore)->getIDByEntity(_p_triple->object);
	if(_obj_id == INVALID_ENTITY_LITERAL_ID)
	{
		_obj_id = (this->kvstore)->getIDByLiteral(_p_triple->object);
	}
	if(_obj_id != INVALID_ENTITY_LITERAL_ID)
	{
		Signature::encodeStr2Entity(_bitset, _obj_id, Util::EDGE_OUT);
	}

	//if (this->encode_mode == Database::ID_MODE)
	//{
	//}
	//else if (this->encode_mode == Database::STRING_MODE)
	//{
		//Signature::encodeStr2Entity((_p_triple->object).c_str(), _bitset);
	//}

	return true;
}

bool
Database::encodeTriple2SubEntityBitSet(EntityBitSet& _bitset, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id)
{
	Signature::encodeEdge2Entity(_bitset, _pre_id, _obj_id, Util::EDGE_OUT);

	return true;
}

//encode Triple into object SigEntry
bool
Database::encodeTriple2ObjEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple)
{
	TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_p_triple->predicate);
	if(_pre_id != INVALID_PREDICATE_ID)
	{
		Signature::encodePredicate2Entity(_bitset, _pre_id, Util::EDGE_IN);
	}

	TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_p_triple->subject);
	if(_sub_id != INVALID_ENTITY_LITERAL_ID)
	{
		Signature::encodeStr2Entity(_bitset, _sub_id, Util::EDGE_IN);
	}

	//Signature::encodePredicate2Entity(_pre_id, _bitset, Util::EDGE_IN);
	//if (this->encode_mode == Database::ID_MODE)
	//{
	//}
	//else if (this->encode_mode == Database::STRING_MODE)
	//{
		//Signature::encodeStr2Entity((_p_triple->subject).c_str(), _bitset);
	//}

	return true;
}

bool
Database::encodeTriple2ObjEntityBitSet(EntityBitSet& _bitset, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _sub_id)
{
	Signature::encodeEdge2Entity(_bitset, _pre_id, _sub_id, Util::EDGE_IN);

	return true;
}
*/
//check whether the relative 3-tuples exist usually, through sp2olist
bool
Database::exist_triple(TYPE_ENTITY_LITERAL_ID _sub_id, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _obj_id, shared_ptr<Transaction> txn)
{
	unsigned* _objidlist = NULL;
	unsigned _list_len = 0;
	//get exclusive before update!
	(this->kvstore)->getobjIDlistBysubIDpreID(_sub_id, _pre_id, _objidlist, _list_len, true, txn);

	bool is_exist = false;
	//	for(unsigned i = 0; i < _list_len; i ++)
	//	{
	//		if(_objidlist[i] == _obj_id)
	//		{
	//		    is_exist = true;
	//			break;
	//		}
	//	}
	if (Util::bsearch_int_uporder(_obj_id, _objidlist, _list_len) != INVALID)
	//if (Util::bsearch_int_uporder(_obj_id, _objidlist, _list_len) != -1)
	{
		is_exist = true;
	}
	delete[] _objidlist;

	return is_exist;
}

bool Database::exist_triple(const TripleWithObjType& _triple, shared_ptr<Transaction> txn) {
	int sub_id = this->kvstore->getIDByEntity(_triple.getSubject());
	if (sub_id == -1) {
		return false;
	}

	int pre_id = this->kvstore->getIDByPredicate(_triple.getPredicate());
	if (pre_id == -1) {
		return false;
	}

	int obj_id = -1;
	if (_triple.isObjEntity()) {
		obj_id = this->kvstore->getIDByEntity(_triple.getObject());
	}
	else if (_triple.isObjLiteral()) {
		obj_id = this->kvstore->getIDByLiteral(_triple.getObject());
	}
	if (obj_id == -1) {
		return false;
	}

	return exist_triple(sub_id, pre_id, obj_id, txn);
}

//NOTICE: all constants are transfered to ids in memory
//this maybe not ok when size is too large!
bool
Database::encodeRDF_new(const string _rdf_file)
{
#ifdef DEBUG
	//cout<< "now to log!!!" << endl;
	Util::logging("In encodeRDF_new");
	//cout<< "end log!!!" << endl;
#endif

	//TYPE_ENTITY_LITERAL_ID** _p_id_tuples = NULL;
	ID_TUPLE* _p_id_tuples = NULL;
	TYPE_TRIPLE_NUM _id_tuples_max = 0;

	long t1 = Util::get_cur_time();

	//NOTICE: in encode process, we should not divide ID of entity and literal totally apart, i.e. entity is a system 
	//while literal is another system
	//The reason is that if we divide entity and literal, then in triple_array and final_result we can not decide a given
	//ID is entity or not
	//(one way is to add a more structure to tell us which is entity, but this is costly)

	//map sub2id, pre2id, entity/literal in obj2id, store in kvstore, encode RDF data into signature
	if (!this->sub2id_pre2id_obj2id_RDFintoSignature(_rdf_file))
	{
		return false;
	}

	//TODO+BETTER:after encode, we can know the exact entity num, so we can decide if our system can run this dataset
	//based on the current available memory(need a memory manager globally)
	//If unbale to run, should exit and give a prompt
	//User can modify the config file to run anyway, but gStore will not ensure correctness
	//What is more, in load process, we also need to decide if gStore can run it
	//
	//TODO+BETTER: a global ID manager module, should be based on type template
	//this can be used in vstree, storage and Database

	long t2 = Util::get_cur_time();
	cout << "after encode, used " << (t2 - t1) << "ms." << endl;

	//build stringindex before this->kvstore->id2* trees are closed
	this->stringindex->setNum(StringIndexFile::Entity, this->entity_num);
	this->stringindex->setNum(StringIndexFile::Literal, this->literal_num);
	this->stringindex->setNum(StringIndexFile::Predicate, this->pre_num);
	this->stringindex->save(*this->kvstore);
	//NOTICE: the string index can be parallized with readIDTuples and others
	//However, we should read and build otehr indices only after the 6 trees and string index closed
	//(to save memory)

	long t3 = Util::get_cur_time();
	cout << "after stringindex, used " << (t3 - t2) << "ms." << endl;

	//cout<<"special id: "<<this->kvstore->getIDByEntity("<point7>")<<endl;

	//NOTICE:close these trees now to save memory
	this->kvstore->close_entity2id();
	this->kvstore->close_id2entity();
	this->kvstore->close_literal2id();
	this->kvstore->close_id2literal();
	this->kvstore->close_predicate2id();
	this->kvstore->close_id2predicate();

	long t4 = Util::get_cur_time();
	cout << "id2string and string2id closed, used " << (t4 - t3) << "ms." << endl;

	//after closing the 6 trees, read the id tuples again, and remove the file     given num, a dimension,return a pointer
	//NOTICE: the file can also be used for debugging, and a program can start just from the id tuples file
	//(if copy the 6 id2string trees, no need to parse each time)
	this->readIDTuples(_p_id_tuples);

	//NOTICE: we can also build the signature when we are reading triples, and
	//update to the corresponding position in the signature file
	//However, this may be costly due to frequent read/write

	long t5 = Util::get_cur_time();
	cout << "id tuples read, used " << (t5 - t4) << "ms." << endl;

	//TODO: how to set the buffer of trees is a big question, fully utilize the availiable memory

	//this->kvstore->build_subID2values(_p_id_tuples, this->triples_num);
	this->build_s2xx(_p_id_tuples);

	long t6 = Util::get_cur_time();
	cout << "after s2xx, used " << (t6 - t5) << "ms." << endl;

	//this->kvstore->build_objID2values(_p_id_tuples, this->triples_num);
	this->build_o2xx(_p_id_tuples);

	long t7 = Util::get_cur_time();
	cout << "after o2xx, used " << (t7 - t6) << "ms." << endl;

	//this->kvstore->build_preID2values(_p_id_tuples, this->triples_num);
	this->build_p2xx(_p_id_tuples);

	long t8 = Util::get_cur_time();
	cout << "after p2xx, used " << (t8 - t7) << "ms." << endl;

	//WARN:we must free the memory for id_tuples array
	delete[] _p_id_tuples;

	//for (TYPE_TRIPLE_NUM i = 0; i < this->triples_num; ++i)
	//{
		//delete[] _p_id_tuples[i];
	//}
	//delete[] _p_id_tuples;

	//NOTICE: we should build vstree after id tuples are freed(to save memory)

	bool flag = this->saveDBInfoFile();
	if (!flag)
	{
		return false;
	}

	long t9 = Util::get_cur_time();
	cout << "db info saved, used " << (t9 - t8) << "ms." << endl;

	//Util::logging("finish encodeRDF_new");

	return true;
}

bool
Database::encodeRDF_new(const string _rdf_file,const string _error_log)
{
#ifdef DEBUG
	//cout<< "now to log!!!" << endl;
	Util::logging("In encodeRDF_new");
	//cout<< "end log!!!" << endl;
#endif

	//TYPE_ENTITY_LITERAL_ID** _p_id_tuples = NULL;
	ID_TUPLE* _p_id_tuples = NULL;
	TYPE_TRIPLE_NUM _id_tuples_max = 0;

	long t1 = Util::get_cur_time();

	//NOTICE: in encode process, we should not divide ID of entity and literal totally apart, i.e. entity is a system
	//while literal is another system
	//The reason is that if we divide entity and literal, then in triple_array and final_result we can not decide a given
	//ID is entity or not
	//(one way is to add a more structure to tell us which is entity, but this is costly)

	//map sub2id, pre2id, entity/literal in obj2id, store in kvstore, encode RDF data into signature
	if (!this->sub2id_pre2id_obj2id_RDFintoSignature(_rdf_file,_error_log))
	{
		return false;
	}

	//TODO+BETTER:after encode, we can know the exact entity num, so we can decide if our system can run this dataset
	//based on the current available memory(need a memory manager globally)
	//If unbale to run, should exit and give a prompt
	//User can modify the config file to run anyway, but gStore will not ensure correctness
	//What is more, in load process, we also need to decide if gStore can run it
	//
	//TODO+BETTER: a global ID manager module, should be based on type template
	//this can be used in vstree, storage and Database

	long t2 = Util::get_cur_time();
	cout << "after encode, used " << (t2 - t1) << "ms." << endl;

	//build stringindex before this->kvstore->id2* trees are closed
	this->stringindex->setNum(StringIndexFile::Entity, this->entity_num);
	this->stringindex->setNum(StringIndexFile::Literal, this->literal_num);
	this->stringindex->setNum(StringIndexFile::Predicate, this->pre_num);
	this->stringindex->save(*this->kvstore);
	//NOTICE: the string index can be parallized with readIDTuples and others
	//However, we should read and build otehr indices only after the 6 trees and string index closed
	//(to save memory)

	long t3 = Util::get_cur_time();
	cout << "after stringindex, used " << (t3 - t2) << "ms." << endl;

	//cout<<"special id: "<<this->kvstore->getIDByEntity("<point7>")<<endl;

	//NOTICE:close these trees now to save memory
	this->kvstore->close_entity2id();
	this->kvstore->close_id2entity();
	this->kvstore->close_literal2id();
	this->kvstore->close_id2literal();
	this->kvstore->close_predicate2id();
	this->kvstore->close_id2predicate();


	long t4 = Util::get_cur_time();
	cout << "id2string and string2id closed, used " << (t4 - t3) << "ms." << endl;

	//after closing the 6 trees, read the id tuples again, and remove the file     given num, a dimension,return a pointer
	//NOTICE: the file can also be used for debugging, and a program can start just from the id tuples file
	//(if copy the 6 id2string trees, no need to parse each time)
	this->readIDTuples(_p_id_tuples);

	//NOTICE: we can also build the signature when we are reading triples, and 
	//update to the corresponding position in the signature file
	//However, this may be costly due to frequent read/write

	long t5 = Util::get_cur_time();
	cout << "id tuples read, used " << (t5 - t4) << "ms." << endl;

	//TODO: how to set the buffer of trees is a big question, fully utilize the availiable memory

	//this->kvstore->build_subID2values(_p_id_tuples, this->triples_num);
	this->build_s2xx(_p_id_tuples);

	long t6 = Util::get_cur_time();
	cout << "after s2xx, used " << (t6 - t5) << "ms." << endl;

	//this->kvstore->build_objID2values(_p_id_tuples, this->triples_num);
	this->build_o2xx(_p_id_tuples);

	long t7 = Util::get_cur_time();
	cout << "after o2xx, used " << (t7 - t6) << "ms." << endl;

	//this->kvstore->build_preID2values(_p_id_tuples, this->triples_num);
	this->build_p2xx(_p_id_tuples);

	long t8 = Util::get_cur_time();
	cout << "after p2xx, used " << (t8 - t7) << "ms." << endl;

	//WARN:we must free the memory for id_tuples array
	delete[] _p_id_tuples;

	//for (TYPE_TRIPLE_NUM i = 0; i < this->triples_num; ++i)
	//{
		//delete[] _p_id_tuples[i];
	//}
	//delete[] _p_id_tuples;

	//NOTICE: we should build vstree after id tuples are freed(to save memory)

	bool flag = this->saveDBInfoFile();
	if (!flag)
	{
		return false;
	}

	long t9 = Util::get_cur_time();
	cout << "db info saved, used " << (t9 - t8) << "ms." << endl;
    flag=this->saveStatisticsInfoFile();
	if(!flag)
	{
		cout<<"the statistics info file of db saved failure!"<<endl;
	}
	//Util::logging("finish encodeRDF_new");

	// not use statistics
	/*
    if(!this->kvstore->open_preID2values(KVstore::READ_WRITE_MODE) ||
       !this->kvstore->open_subID2values(KVstore::READ_WRITE_MODE) ||
       !this->kvstore->open_objID2values(KVstore::READ_WRITE_MODE) ){
        cout << "false" << endl;
    }

    long t10 = Util::get_cur_time();
    Statistics *statistics = new Statistics(this->getStorePath(), this->getlimitID_predicate());
    long t11 = Util::get_cur_time();
    cout << "init statistics done， used " << (t11 - t10) << "ms" << endl;

    if(!statistics->build_Statistics(this->kvstore)){
        cout << "statistics info build failed" << endl;
        return false;
    }

    long t12 = Util::get_cur_time();
    cout << "statistics info build succeeded, used " << (t12 - t11) << "ms." << endl;

    statistics->save_Statistics();
    long t13 = Util::get_cur_time();
    cout << "statistics info saved, used " << (t13 - t12) << "ms." << endl;

    delete statistics;
    this->kvstore->close_preID2values();
    this->kvstore->close_objID2values();
    this->kvstore->close_subID2values();

    this->kvstore->close_id2predicate();

	 */

    return true;
}

void 
Database::readIDTuples(ID_TUPLE*& _p_id_tuples)
{
	_p_id_tuples = NULL;
	string fname = this->getIDTuplesFile();
	FILE* fp = fopen(fname.c_str(), "rb");
	if(fp == NULL)
	{
		cout<<"error in Database::readIDTuples() -- unable to open file "<<fname<<endl;
		return;
	}

	//NOTICE: avoid to break the unsigned limit, size_t is used in Linux C
	//size_t means long unsigned int in 64-bit machine
	//unsigned long total_num = this->triples_num * 3;
	//_p_id_tuples = new TYPE_ENTITY_LITERAL_ID[total_num];
	_p_id_tuples = new ID_TUPLE[this->triples_num];
	fread(_p_id_tuples, sizeof(ID_TUPLE), this->triples_num, fp);

	fclose(fp);
	//NOTICE: choose to empty the file or not
	Util::empty_file(fname.c_str());

	//return NULL;
}

void
Database::build_s2xx(ID_TUPLE* _p_id_tuples)
{
	//NOTICE: STL sort() is generally fatser than C qsort, especially when qsort is very slow
	//STL sort() not only use qsort algorithm, it can also choose heap-sort method
#ifndef PARALLEL_SORT
	sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::spo_cmp_idtuple);
#else
	omp_set_num_threads(thread_num);
	__gnu_parallel::sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::spo_cmp_idtuple);
#endif
	//qsort(_p_id_tuples, this->triples_num, sizeof(int*), Util::_spo_cmp);

	cout << "triples_num before removing duplicates: " << this->triples_num << endl;
	//remove duplicates from the id tables
	TYPE_TRIPLE_NUM j = 1;
	// TODO: should output triples_num without removing duplicates for reference, or keep a unique_triples_num separately?
	for(TYPE_TRIPLE_NUM i = 1; i < this->triples_num; ++i)
	{
		if(!Util::equal(_p_id_tuples[i], _p_id_tuples[i-1]))
		{
			_p_id_tuples[j] = _p_id_tuples[i];
			++j;
		}
	}
	this->triples_num = j;	// NOTE: triples_num set here
	
	this->kvstore->build_subID2values(_p_id_tuples, this->triples_num, this->entity_num);

	//save all entity_signature into binary file
	//string sig_binary_file = this->getSignatureBFile();
	//FILE* sig_fp = fopen(sig_binary_file.c_str(), "wb");
	//if (sig_fp == NULL) 
	//{
		//cout << "Failed to open : " << sig_binary_file << endl;
		//return;
	//}

	//NOTICE:in build process, all IDs are continuous growing
	//EntityBitSet tmp_bitset;
	//tmp_bitset.reset();
	//for(TYPE_ENTITY_LITERAL_ID i = 0; i < this->entity_num; ++i)
	//{
		//SigEntry* sig = new SigEntry(EntitySig(tmp_bitset), -1);
		//fwrite(sig, sizeof(SigEntry), 1, sig_fp);
		//delete sig;
	//}

	//TYPE_ENTITY_LITERAL_ID prev_entity_id = INVALID_ENTITY_LITERAL_ID;
	//int prev_entity_id = -1;
	
	//NOTICE: i*3 + j maybe break the unsigned limit
	//for (unsigned long i = 0; i < this->triples_num; ++i)
	//for (TYPE_TRIPLE_NUM i = 0; i < this->triples_num; ++i)
	//{
		//TYPE_ENTITY_LITERAL_ID subid = _p_id_tuples[i].subid;
		//TYPE_PREDICATE_ID preid = _p_id_tuples[i].preid;
		//TYPE_ENTITY_LITERAL_ID objid = _p_id_tuples[i].objid;
		////TYPE_ENTITY_LITERAL_ID subid = _p_id_tuples[i*3+0];
		////TYPE_PREDICATE_ID preid = _p_id_tuples[i*3+1];
		////TYPE_ENTITY_LITERAL_ID objid = _p_id_tuples[i*3+2];
		//if(subid != prev_entity_id)
		//{
			//if(prev_entity_id != INVALID_ENTITY_LITERAL_ID)
			////if(prev_entity_id != -1)
			//{
//#ifdef DEBUG
				////if(prev_entity_id == 13)
				////{
					////cout<<"yy: "<<Signature::BitSet2str(tmp_bitset)<<endl;
				////}
//#endif
				////NOTICE: we must do twice, we need to locate on the same entry to deal, so we must place in order
				//SigEntry* sig = new SigEntry(EntitySig(tmp_bitset), prev_entity_id);
				////write the sig entry
				//fseek(sig_fp, sizeof(SigEntry) * prev_entity_id, SEEK_SET);
				//fwrite(sig, sizeof(SigEntry), 1, sig_fp);
				////_all_bitset |= *_entity_bitset[i];
				//delete sig;
			//}
			//prev_entity_id = subid;
			//tmp_bitset.reset();
			//Signature::encodeEdge2Entity(tmp_bitset, preid, objid, Util::EDGE_OUT);
			////Signature::encodePredicate2Entity(preid, _tmp_bitset, Util::EDGE_OUT);
			////Signature::encodeStr2Entity(objid, _tmp_bitset);
		//}
		//else
		//{
			//Signature::encodeEdge2Entity(tmp_bitset, preid, objid, Util::EDGE_OUT);
		//}
	//}

	////NOTICE: remember to write the last entity's signature
	//if(prev_entity_id != INVALID_ENTITY_LITERAL_ID)
	////if(prev_entity_id != -1)
	//{
		//SigEntry* sig = new SigEntry(EntitySig(tmp_bitset), prev_entity_id);
		////write the sig entry
		//fseek(sig_fp, sizeof(SigEntry) * prev_entity_id, SEEK_SET);
		//fwrite(sig, sizeof(SigEntry), 1, sig_fp);
		////_all_bitset |= *_entity_bitset[i];
		//delete sig;
	//}

	//fclose(sig_fp);
}

void
Database::build_o2xx(ID_TUPLE* _p_id_tuples)
{
#ifndef PARALLEL_SORT
	sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::ops_cmp_idtuple);
#else
	omp_set_num_threads(thread_num);
	__gnu_parallel::sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::ops_cmp_idtuple);
#endif
	//qsort(_p_id_tuples, this->triples_num, sizeof(int*), Util::_ops_cmp);
	this->kvstore->build_objID2values(_p_id_tuples, this->triples_num, this->entity_num, this->literal_num);

	//save all entity_signature into binary file
	//string sig_binary_file = this->getSignatureBFile();
	////NOTICE: this is different from build_s2xx, the file already exists
	//FILE* sig_fp = fopen(sig_binary_file.c_str(), "rb+");
	//if (sig_fp == NULL) 
	//{
		//cout << "Failed to open : " << sig_binary_file << endl;
		//return;
	//}

	////NOTICE:in build process, all IDs are continuous growing
	////TODO:use unsigned for type and -1 should be changed
	//TYPE_ENTITY_LITERAL_ID prev_entity_id = INVALID_ENTITY_LITERAL_ID;
	////int prev_entity_id = -1;
	//EntityBitSet tmp_bitset;

	////NOTICE: i*3 + j maybe break the unsigned limit
	////for (unsigned long i = 0; i < this->triples_num; ++i)
	//for (TYPE_TRIPLE_NUM i = 0; i < this->triples_num; ++i)
	//{
		//TYPE_ENTITY_LITERAL_ID subid = _p_id_tuples[i].subid;
		//TYPE_PREDICATE_ID preid = _p_id_tuples[i].preid;
		//TYPE_ENTITY_LITERAL_ID objid = _p_id_tuples[i].objid;
		////TYPE_ENTITY_LITERAL_ID subid = _p_id_tuples[i*3+0];
		////TYPE_PREDICATE_ID preid = _p_id_tuples[i*3+1];
		////TYPE_ENTITY_LITERAL_ID objid = _p_id_tuples[i*3+2];


		//if(Util::is_literal_ele(objid))
		//{
			//continue;
		//}

		//if(objid != prev_entity_id)
		//{
			////if(prev_entity_id != -1)
			//if(prev_entity_id != INVALID_ENTITY_LITERAL_ID)
			//{
				////NOTICE: we must do twice, we need to locate on the same entry to deal, so we must place in order
				//fseek(sig_fp, sizeof(SigEntry) * prev_entity_id, SEEK_SET);
				//SigEntry* old_sig = new SigEntry();
				//fread(old_sig, sizeof(SigEntry), 1, sig_fp);
//#ifdef DEBUG
				////cout<<"to write a signature: "<<prev_entity_id<<endl;
				////cout<<prev_entity_id<<endl;
				////if(prev_entity_id == 13)
				////{
					////cout<<"yy: "<<Signature::BitSet2str(tmp_bitset)<<endl;
				////}
//#endif
				//tmp_bitset |= old_sig->getEntitySig().entityBitSet;
				//delete old_sig;
				
//#ifdef DEBUG
				////if(prev_entity_id == 13)
				////{
					////cout<<"yy: "<<Signature::BitSet2str(tmp_bitset)<<endl;
				////}
//#endif
				
					////write the sig entry
				//SigEntry* sig = new SigEntry(EntitySig(tmp_bitset), prev_entity_id);
				//fseek(sig_fp, sizeof(SigEntry) * prev_entity_id, SEEK_SET);
				//fwrite(sig, sizeof(SigEntry), 1, sig_fp);
				////_all_bitset |= *_entity_bitset[i];
				//delete sig;
			//}

//#ifdef DEBUG
			////cout<<"now is a new signature: "<<objid<<endl;
//#endif

			//prev_entity_id = objid;
			//tmp_bitset.reset();
			////cout<<"bitset reset"<<endl;
			//Signature::encodeEdge2Entity(tmp_bitset, preid, subid, Util::EDGE_IN);
			////cout<<"edge encoded"<<endl;
			////Signature::encodePredicate2Entity(preid, _tmp_bitset, Util::EDGE_IN);
			////Signature::encodeStr2Entity(subid, _tmp_bitset);
		//}
		//else
		//{
			////cout<<"same signature: "<<objid<<" "<<preid<<" "<<subid<<endl;
			//Signature::encodeEdge2Entity(tmp_bitset, preid, subid, Util::EDGE_IN);
			////cout<<"edge encoded"<<endl;
		//}
	//}
	////cout<<"loop ended!"<<endl;

	////NOTICE: remember to write the last entity's signature
	//if(prev_entity_id != INVALID_ENTITY_LITERAL_ID)
	////if(prev_entity_id != -1)
	//{
		////cout<<"to write the last signature"<<endl;

		//fseek(sig_fp, sizeof(SigEntry) * prev_entity_id, SEEK_SET);
		//SigEntry* old_sig = new SigEntry();
		//fread(old_sig, sizeof(SigEntry), 1, sig_fp);
		//tmp_bitset |= old_sig->getEntitySig().entityBitSet;
		//delete old_sig;
		////write the sig entry
		//SigEntry* sig = new SigEntry(EntitySig(tmp_bitset), prev_entity_id);
		//fseek(sig_fp, sizeof(SigEntry) * prev_entity_id, SEEK_SET);
		//fwrite(sig, sizeof(SigEntry), 1, sig_fp);
		////_all_bitset |= *_entity_bitset[i];
		//delete sig;
	//}

	//fclose(sig_fp);
}

void
Database::build_p2xx(ID_TUPLE* _p_id_tuples)
{
#ifndef PARALLEL_SORT
	sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::pso_cmp_idtuple);
#else
	omp_set_num_threads(thread_num);
	__gnu_parallel::sort(_p_id_tuples, _p_id_tuples + this->triples_num, Util::pso_cmp_idtuple);
#endif
	//qsort(_p_id_tuples, this->triples_num, sizeof(int*), Util::_pso_cmp);
	this->kvstore->build_preID2values(_p_id_tuples, this->triples_num, this->pre_num);
}

//NOTICE:in here and there in the insert/delete, we may get the maxium tuples num first
//and so we can avoid the cost of memcpy(scan quickly or use wc -l)
//However, if use compressed RDF format, how can we do it fi not using parser?
//CONSIDER: just an estimated value is ok or use vector!!!(but vector also copy when enlarge)
//and read file line numbers are also costly!
bool
Database::sub2id_pre2id_obj2id_RDFintoSignature(const string _rdf_file)
{
	//NOTICE: if we keep the id_tuples always in memory, i.e. [unsigned*] each unsigned* is [3]
	//then for freebase, there is 2.5B triples. the mmeory cost of this array is 25*10^8*3*4 + 25*10^8*8 = 50G
	//
	//So I choose not to store the id_tuples in memory in this function, but to store them in file and read again after this function
	//Notice that the most memory-costly part of building process is this function, setup 6 trees together
	//later we can read the id_tuples and stored as [num][3], only cost 25*10^8*3*4 = 30G, and later we only build one tree at a time

	string fname = this->getIDTuplesFile();
	FILE* fp = fopen(fname.c_str(), "wb");
	if(fp == NULL)
	{
		cout<<"error in Database::sub2id_pre2id_obj2id() -- unable to open file to write "<<fname<<endl;
		return false;
	}
	ID_TUPLE tmp_id_tuple;
	//NOTICE: avoid to break the unsigned limit, size_t is used in Linux C
	//size_t means long unsigned int in 64-bit machine
	//fread(_p_id_tuples, sizeof(TYPE_ENTITY_LITERAL_ID), total_num, fp);

	TYPE_TRIPLE_NUM _id_tuples_size;
	{
		//initial
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

	//Util::logging("finish initial sub2id_pre2id_obj2id");
	cout << "finish initial sub2id_pre2id_obj2id" << endl;

	//BETTER?:close the stdio buffer sync??

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		cout << "sub2id&pre2id&obj2id: Fail to open : " << _rdf_file << endl;
		//exit(0);
		return false;
	}

	string _six_tuples_file = this->getSixTuplesFile();
	ofstream _six_tuples_fout(_six_tuples_file.c_str());
	if (!_six_tuples_fout)
	{
		cout << "sub2id&pre2id&obj2id: Fail to open: " << _six_tuples_file << endl;
		//exit(0);
		return false;
	}

	TripleWithObjType* triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];

	//don't know the number of entity
	//pre allocate entitybitset_max EntityBitSet for storing signature, double the space until the _entity_bitset is used up.
	//
	//int entitybitset_max = 10000000;  //set larger to avoid the copy cost
	//int entitybitset_max = 10000;
	//EntityBitSet** _entity_bitset = new EntityBitSet*[entitybitset_max];
	//for (int i = 0; i < entitybitset_max; i++)
	//{
		//_entity_bitset[i] = new EntityBitSet();
		//_entity_bitset[i]->reset();
	//}
	//EntityBitSet _tmp_bitset;

    //	{
    //		cout << "begin build Prefix" << endl;
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
	//Util::logging("==> while(true)");
	
	while (true)
	{
		int parse_triple_num = 0;

		_parser.parseFile(triple_array, parse_triple_num);

		{
			stringstream _ss;
			_ss << "finish rdfparser" << this->triples_num << endl;
			//Util::logging(_ss.str());
			cout << _ss.str() << endl;
		}
		cout << "after info in sub2id_" << endl;

		if (parse_triple_num == 0)
		{
			break;
		}

		//Process the Triple one by one
		for (int i = 0; i < parse_triple_num; i++)
		{
			//BETTER: assume that no duplicate triples in RDF for building
			//should judge first? using exist_triple() 
			//or sub triples_num in build_subID2values(judge if two neighbor triples are same)
			this->triples_num++;	 // NOTE: triples_num set here

			//if the _id_tuples exceeds, double the space
			//if (_id_tuples_size == _id_tuples_max)
			//{
				//TYPE_TRIPLE_NUM _new_tuples_len = _id_tuples_max * 2;
				//TYPE_ENTITY_LITERAL_ID** _new_id_tuples = new TYPE_ENTITY_LITERAL_ID*[_new_tuples_len];
				//memcpy(_new_id_tuples, _p_id_tuples, sizeof(TYPE_ENTITY_LITERAL_ID*) * _id_tuples_max);
				//delete[] _p_id_tuples;
				//_p_id_tuples = _new_id_tuples;
				//_id_tuples_max = _new_tuples_len;
			//}

			//BETTER: use 3 threads to deal with sub, obj, pre separately
			//However, the cost of new /delete threads may be high
			//We need a thread pool!

			// For subject
			// (all subject is entity, some object is entity, the other is literal)
			string _sub = triple_array[i].getSubject();
			TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_sub);
			if (_sub_id == INVALID_ENTITY_LITERAL_ID)
			//if (_sub_id == -1)
			{
				//_sub_id = this->entity_num;
				_sub_id = this->allocEntityID();
				this->sub_num++;
				//this->entity_num++;
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
				//this->pre_num++;
				(this->kvstore)->setIDByPredicate(_pre, _pre_id);
				(this->kvstore)->setPredicateByID(_pre_id, _pre);
			}

			//  For object
			string _obj = triple_array[i].getObject();
			//int _obj_id = -1;
			TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
			// obj is entity
			if (triple_array[i].isObjEntity())
			{
				_obj_id = (this->kvstore)->getIDByEntity(_obj);
				if (_obj_id == INVALID_ENTITY_LITERAL_ID)
				//if (_obj_id == -1)
				{
					//_obj_id = this->entity_num;
					_obj_id = this->allocEntityID();
					//this->entity_num++;
					(this->kvstore)->setIDByEntity(_obj, _obj_id);
					(this->kvstore)->setEntityByID(_obj_id, _obj);
				}
			}
			//obj is literal
			if (triple_array[i].isObjLiteral())
			{
				_obj_id = (this->kvstore)->getIDByLiteral(_obj);
				if (_obj_id == INVALID_ENTITY_LITERAL_ID)
				//if (_obj_id == -1)
				{
					//_obj_id = Util::LITERAL_FIRST_ID + (this->literal_num);
					_obj_id = this->allocLiteralID();
					//this->literal_num++;
					(this->kvstore)->setIDByLiteral(_obj, _obj_id);
					(this->kvstore)->setLiteralByID(_obj_id, _obj);
					//#ifdef DEBUG
					//if(_obj == "\"Bob\"")
					//{
					//cout << "this is id for Bob: " << _obj_id << endl;
					//}
					//cout<<"literal should be bob: " << kvstore->getLiteralByID(_obj_id)<<endl;
					//cout<<"id for bob: "<<kvstore->getIDByLiteral("\"Bob\"")<<endl;
					//#endif
				}
			}

			//NOTICE: we assume that there is no duplicates in the dataset
			//if not, this->triple_num will be not right, and _p_id_tuples will save useless triples
			//However, we can not use exist_triple to detect duplicates here, because it is too time-costly

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
			//fwrite(&_sub_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);
			//fwrite(&_pre_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);
			//fwrite(&_obj_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);

#ifdef DEBUG_PRECISE
			////  save six tuples
				//_six_tuples_fout << _sub_id << '\t'
					//<< _pre_id << '\t'
					//<< _obj_id << '\t'
					//<< _sub << '\t'
					//<< _pre << '\t'
					//<< _obj << endl;
#endif

			//NOTICE: the memory cost maybe too larger if combine teh below process here
			//we can do below after this function or after all B+trees are built and closed
			//and we can decide the length of signature according to entity num then
			//1. after all b+trees: empty id_tuples and only open id2string, reload rdf file and encode(using string for entity/literal)
			//
			//2. after this function or after all B+trees: close others and only use id_tuples to encode(no need to read file again, which is too costly)
			//not encoded with string but all IDs(not using encode for string regex matching, then this is ok!)
			//Because we encode with ID, then Signature has to be changed(and dynamic sig length)
			//use same encode strategy for entity/literal/predicate, and adjust the rate of the 3 parts according to real case
			//What is more, if the system memory is enough(precisely, the memory you want to assign to gstore - to vstree/entity_sig_array),
			//we can also set the sig length larger(which should be included in config file)

			//_entity_bitset is used up, double the space
			//if (this->entity_num >= entitybitset_max)
			//{
				////cout<<"to double entity bitset num"<<endl;
				//EntityBitSet** _new_entity_bitset = new EntityBitSet*[entitybitset_max * 2];
				////BETTER?:if use pointer for array, avoid the copy cost of entitybitset, but consumes more mmeory
				////if the triple size is 1-2 billion, then the memory cost will be very large!!!
				//memcpy(_new_entity_bitset, _entity_bitset, sizeof(EntityBitSet*) * entitybitset_max);
				//delete[] _entity_bitset;
				//_entity_bitset = _new_entity_bitset;

				//int tmp = entitybitset_max * 2;
				//for (int i = entitybitset_max; i < tmp; i++)
				//{
					//_entity_bitset[i] = new EntityBitSet();
					//_entity_bitset[i]->reset();
				//}

				//entitybitset_max = tmp;
			//}

			//{
				//_tmp_bitset.reset();
				//Signature::encodePredicate2Entity(_pre_id, _tmp_bitset, Util::EDGE_OUT);
				//Signature::encodeStr2Entity(_obj.c_str(), _tmp_bitset);
				//*_entity_bitset[_sub_id] |= _tmp_bitset;
			//}

			//if (triple_array[i].isObjEntity())
			//{
				//_tmp_bitset.reset();
				//Signature::encodePredicate2Entity(_pre_id, _tmp_bitset, Util::EDGE_IN);
				//Signature::encodeStr2Entity(_sub.c_str(), _tmp_bitset);
				////cout<<"objid: "<<_obj_id <<endl;
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
	//cout<<"==> end while(true)"<<endl;

	delete[] triple_array;
	triple_array = NULL;
	_fin.close();
	_six_tuples_fout.close();
	fclose(fp);


		//for (int i = 0; i < entitybitset_max; i++)
		//{
			//delete _entity_bitset[i];
		//}
		//delete[] _entity_bitset;

	//{
		//stringstream _ss;
		//_ss << "finish sub2id pre2id obj2id" << endl;
		//_ss << "tripleNum is " << this->triples_num << endl;
		//_ss << "entityNum is " << this->entity_num << endl;
		//_ss << "preNum is " << this->pre_num << endl;
		//_ss << "literalNum is " << this->literal_num << endl;
		//Util::logging(_ss.str());
		//cout << _ss.str() << endl;
	//}

	return true;
}

bool
Database::sub2id_pre2id_obj2id_RDFintoSignature(const string _rdf_file,const string _error_log)
{
	//NOTICE: if we keep the id_tuples always in memory, i.e. [unsigned*] each unsigned* is [3]
	//then for freebase, there is 2.5B triples. the mmeory cost of this array is 25*10^8*3*4 + 25*10^8*8 = 50G
	//
	//So I choose not to store the id_tuples in memory in this function, but to store them in file and read again after this function
	//Notice that the most memory-costly part of building process is this function, setup 6 trees together
	//later we can read the id_tuples and stored as [num][3], only cost 25*10^8*3*4 = 30G, and later we only build one tree at a time

	string fname = this->getIDTuplesFile();
	FILE* fp = fopen(fname.c_str(), "wb");
	if (fp == NULL)
	{
		cout << "error in Database::sub2id_pre2id_obj2id() -- unable to open file to write " << fname << endl;
		return false;
	}
	ID_TUPLE tmp_id_tuple;
	//NOTICE: avoid to break the unsigned limit, size_t is used in Linux C
	//size_t means long unsigned int in 64-bit machine
	//fread(_p_id_tuples, sizeof(TYPE_ENTITY_LITERAL_ID), total_num, fp);

	TYPE_TRIPLE_NUM _id_tuples_size;
	{
		//initial
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

	//Util::logging("finish initial sub2id_pre2id_obj2id");
	cout << "finish initial sub2id_pre2id_obj2id" << endl;

	//BETTER?:close the stdio buffer sync??

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		cout << "sub2id&pre2id&obj2id: Fail to open : " << _rdf_file << endl;
		//exit(0);
		return false;
	}

	string _six_tuples_file = this->getSixTuplesFile();
	ofstream _six_tuples_fout(_six_tuples_file.c_str());
	if (!_six_tuples_fout)
	{
		cout << "sub2id&pre2id&obj2id: Fail to open: " << _six_tuples_file << endl;
		//exit(0);
		return false;
	}

	TripleWithObjType* triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];

	//don't know the number of entity
	//pre allocate entitybitset_max EntityBitSet for storing signature, double the space until the _entity_bitset is used up.
	//
	//int entitybitset_max = 10000000;  //set larger to avoid the copy cost
	//int entitybitset_max = 10000;
	//EntityBitSet** _entity_bitset = new EntityBitSet*[entitybitset_max];
	//for (int i = 0; i < entitybitset_max; i++)
	//{
		//_entity_bitset[i] = new EntityBitSet();
		//_entity_bitset[i]->reset();
	//}
	//EntityBitSet _tmp_bitset;

	int num_lines = 0;
	{
		cout << "begin build Prefix new." << endl;
		long begin = Util::get_cur_time();
		ifstream _fin0(_rdf_file.c_str());
		//parse a file
		RDFParser _parser0(_fin0);

		// Initialize trie

		Trie* trie = kvstore->getTrie();
		while (true)
		{
			int parse_triple_num = 0;
			// TODO: make the line numbers reported inside parseFile global
			int curr_lines = _parser0.parseFile(triple_array, parse_triple_num, _error_log, num_lines);
			num_lines = curr_lines;
			if (parse_triple_num == 0)
			{
				break;
			}

			//Process the Triple one by one
			for (int i = 0; i < parse_triple_num; i++)
			{
				string t = triple_array[i].getSubject();
				trie->Addstring(t);
				t = triple_array[i].getPredicate();
				trie->Addstring(t);
				t = triple_array[i].getObject();
				trie->Addstring(t);
			}
		}
		cout << "Add triples to Trie to prepare for BuildPrefix" << endl;
		trie->BuildPrefix();
		cout << "BuildPrefix done. used" << Util::get_cur_time() - begin << endl;
	}

	RDFParser _parser(_fin);	// RDFParser is actually invoked twice, see above
	//Util::logging("==> while(true)");

	num_lines = 0;
	std::cout<<"this type predicate name is "<<this->type_predicate_name<<endl;
	// string type="rdf:type";
	// this->checkIsTypePredicate(type);
	this->umap.clear();
	while (true)
	{
		int parse_triple_num = 0;

		int curr_lines = _parser.parseFile(triple_array, parse_triple_num, "NULL", num_lines);
		num_lines = curr_lines;

		{
			stringstream _ss;
			_ss << "finish rdfparser" << this->triples_num + parse_triple_num << endl;
			//Util::logging(_ss.str());
			cout << _ss.str() << endl;
		}
		cout << "after info in sub2id_" << endl;

		if (parse_triple_num == 0)
		{
			break;
		}

		//Process the Triple one by one
		// triples_num will eventually be set to the sum of all parse_triple_num (which seems legit)
		for (int i = 0; i < parse_triple_num; i++)
		{
			//BETTER: assume that no duplicate triples in RDF for building
			//should judge first? using exist_triple()
			//or sub triples_num in build_subID2values(judge if two neighbor triples are same)
			this->triples_num++;	// NOTE: triples_num set here

			//if the _id_tuples exceeds, double the space
			//if (_id_tuples_size == _id_tuples_max)
			//{
				//TYPE_TRIPLE_NUM _new_tuples_len = _id_tuples_max * 2;
				//TYPE_ENTITY_LITERAL_ID** _new_id_tuples = new TYPE_ENTITY_LITERAL_ID*[_new_tuples_len];
				//memcpy(_new_id_tuples, _p_id_tuples, sizeof(TYPE_ENTITY_LITERAL_ID*) * _id_tuples_max);
				//delete[] _p_id_tuples;
				//_p_id_tuples = _new_id_tuples;
				//_id_tuples_max = _new_tuples_len;
			//}

			//BETTER: use 3 threads to deal with sub, obj, pre separately
			//However, the cost of new /delete threads may be high
			//We need a thread pool!

			// For subject
			// (all subject is entity, some object is entity, the other is literal)
			string _sub = triple_array[i].getSubject();
			TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_sub);
			if (_sub_id == INVALID_ENTITY_LITERAL_ID)
			//if (_sub_id == -1)
			{
				//_sub_id = this->entity_num;
				_sub_id = this->allocEntityID();
				this->sub_num++;
				//this->entity_num++;
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
				//this->pre_num++;
				(this->kvstore)->setIDByPredicate(_pre, _pre_id);
				(this->kvstore)->setPredicateByID(_pre_id, _pre);
			}

			//  For object
			string _obj = triple_array[i].getObject();
			//int _obj_id = -1;
			TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
			// obj is entity
			if (triple_array[i].isObjEntity())
			{
				_obj_id = (this->kvstore)->getIDByEntity(_obj);
				if (_obj_id == INVALID_ENTITY_LITERAL_ID)
				//if (_obj_id == -1)
				{
					//_obj_id = this->entity_num;
					_obj_id = this->allocEntityID();
					//this->entity_num++;
					(this->kvstore)->setIDByEntity(_obj, _obj_id);
					(this->kvstore)->setEntityByID(_obj_id, _obj);
				}

				//when the predicat is type 
			    
				if(this->checkIsTypePredicate(_pre))
				{
				
				   //umap.insert(_obj,)	
				  auto it=this->umap.find(_obj);
				  if(it!=this->umap.end())
				  {
					it->second=it->second+1;
				  }
				  else{
		
					this->umap.insert(pair<string,unsigned long long>(_obj,1));
				  }
				  //cout<<"the umap size is "<<umap.size()<<endl;
				}
			}
			//obj is literal
			if (triple_array[i].isObjLiteral())
			{
				_obj_id = (this->kvstore)->getIDByLiteral(_obj);
				if (_obj_id == INVALID_ENTITY_LITERAL_ID)
				//if (_obj_id == -1)
				{
					//_obj_id = Util::LITERAL_FIRST_ID + (this->literal_num);
					_obj_id = this->allocLiteralID();
					//this->literal_num++;
					(this->kvstore)->setIDByLiteral(_obj, _obj_id);
					(this->kvstore)->setLiteralByID(_obj_id, _obj);
					//#ifdef DEBUG
					//if(_obj == "\"Bob\"")
					//{
					//cout << "this is id for Bob: " << _obj_id << endl;
					//}
					//cout<<"literal should be bob: " << kvstore->getLiteralByID(_obj_id)<<endl;
					//cout<<"id for bob: "<<kvstore->getIDByLiteral("\"Bob\"")<<endl;
					//#endif
				}
			}

			//NOTICE: we assume that there is no duplicates in the dataset
			//if not, this->triple_num will be not right, and _p_id_tuples will save useless triples
			//However, we can not use exist_triple to detect duplicates here, because it is too time-costly

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
			//fwrite(&_sub_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);
			//fwrite(&_pre_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);
			//fwrite(&_obj_id, sizeof(TYPE_ENTITY_LITERAL_ID), 1, fp);

#ifdef DEBUG_PRECISE
			////  save six tuples
				//_six_tuples_fout << _sub_id << '\t'
					//<< _pre_id << '\t'
					//<< _obj_id << '\t'
					//<< _sub << '\t'
					//<< _pre << '\t'
					//<< _obj << endl;
#endif

			//NOTICE: the memory cost maybe too larger if combine teh below process here
			//we can do below after this function or after all B+trees are built and closed
			//and we can decide the length of signature according to entity num then
			//1. after all b+trees: empty id_tuples and only open id2string, reload rdf file and encode(using string for entity/literal)
			//
			//2. after this function or after all B+trees: close others and only use id_tuples to encode(no need to read file again, which is too costly)
			//not encoded with string but all IDs(not using encode for string regex matching, then this is ok!)
			//Because we encode with ID, then Signature has to be changed(and dynamic sig length)
			//use same encode strategy for entity/literal/predicate, and adjust the rate of the 3 parts according to real case
			//What is more, if the system memory is enough(precisely, the memory you want to assign to gstore - to vstree/entity_sig_array), 
			//we can also set the sig length larger(which should be included in config file)

			//_entity_bitset is used up, double the space
			//if (this->entity_num >= entitybitset_max)
			//{
				////cout<<"to double entity bitset num"<<endl;
				//EntityBitSet** _new_entity_bitset = new EntityBitSet*[entitybitset_max * 2];
				////BETTER?:if use pointer for array, avoid the copy cost of entitybitset, but consumes more mmeory
				////if the triple size is 1-2 billion, then the memory cost will be very large!!!
				//memcpy(_new_entity_bitset, _entity_bitset, sizeof(EntityBitSet*) * entitybitset_max);
				//delete[] _entity_bitset;
				//_entity_bitset = _new_entity_bitset;

				//int tmp = entitybitset_max * 2;
				//for (int i = entitybitset_max; i < tmp; i++)
				//{
					//_entity_bitset[i] = new EntityBitSet();
					//_entity_bitset[i]->reset();
				//}

				//entitybitset_max = tmp;
			//}

			//{
				//_tmp_bitset.reset();
				//Signature::encodePredicate2Entity(_pre_id, _tmp_bitset, Util::EDGE_OUT);
				//Signature::encodeStr2Entity(_obj.c_str(), _tmp_bitset);
				//*_entity_bitset[_sub_id] |= _tmp_bitset;
			//}

			//if (triple_array[i].isObjEntity())
			//{
				//_tmp_bitset.reset();
				//Signature::encodePredicate2Entity(_pre_id, _tmp_bitset, Util::EDGE_IN);
				//Signature::encodeStr2Entity(_sub.c_str(), _tmp_bitset);
				////cout<<"objid: "<<_obj_id <<endl;
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
	//cout<<"==> end while(true)"<<endl;
    
	delete[] triple_array;
	triple_array = NULL;
	_fin.close();
	_six_tuples_fout.close();
	fclose(fp);


	//for (int i = 0; i < entitybitset_max; i++)
	//{
		//delete _entity_bitset[i];
	//}
	//delete[] _entity_bitset;

//{
	//stringstream _ss;
	//_ss << "finish sub2id pre2id obj2id" << endl;
	//_ss << "tripleNum is " << this->triples_num << endl;
	//_ss << "entityNum is " << this->entity_num << endl;
	//_ss << "preNum is " << this->pre_num << endl;
	//_ss << "literalNum is " << this->literal_num << endl;
	//Util::logging(_ss.str());
	//cout << _ss.str() << endl;
//}

	return true;
}

bool
Database::insertTriple(const TripleWithObjType& _triple, vector<unsigned>* _vertices, vector<unsigned>* _predicates, shared_ptr<Transaction> txn)
{
	//cout<<endl<<"the new triple is:"<<endl;
	//cout<<_triple.subject<<endl;
	//cout<<_triple.predicate<<endl;
	//cout<<_triple.object<<endl;

	//int sid1 = this->kvstore->getIDByEntity("<http://www.Department7.University0.edu/UndergraduateStudent394>");
	//int sid2 = this->kvstore->getIDByEntity("<http://www.Department7.University0.edu/UndergraduateStudent395>");
	//int oid1 = this->kvstore->getIDByEntity("<ub:UndergraduateStudent>");
	//int oid2 = this->kvstore->getIDByEntity("<UndergraduateStudent394@Department7.University0.edu>");
	//cout<<sid1<<" "<<sid2<<" "<<oid1<<" "<<oid2<<endl;

	long tv_kv_store_begin = Util::get_cur_time();

	TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
	// if(txn != nullptr)
	// 	cout << "Update in Transaction...................................................." << endl;
	bool _is_new_sub = false;
	//if sub does not exist
	if (_sub_id == INVALID_ENTITY_LITERAL_ID)
	//if (_sub_id == -1)
	{
		_is_new_sub = true;
		//_sub_id = this->entity_num++;
		_sub_id = this->allocEntityID();
		//cout<<"this is a new sub id: "<<_sub_id<<endl;
		this->sub_num++;
		(this->kvstore)->setIDByEntity(_triple.subject, _sub_id);
		(this->kvstore)->setEntityByID(_sub_id, _triple.subject);

		//update the string buffer
		//if (_sub_id < this->entity_buffer_size)
		//{
			//this->entity_buffer->set(_sub_id, _triple.subject);
		//}

		if (_vertices != NULL)
			_vertices->push_back(_sub_id);
	}

	TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
	bool _is_new_pre = false;
	//if pre does not exist
	if (_pre_id == INVALID_PREDICATE_ID)
	//if (_pre_id == -1)
	{
		_is_new_pre = true;
		//_pre_id = this->pre_num++;
		_pre_id = this->allocPredicateID();
		(this->kvstore)->setIDByPredicate(_triple.predicate, _pre_id);
		(this->kvstore)->setPredicateByID(_pre_id, _triple.predicate);

		if (_predicates != NULL)
			_predicates->push_back(_pre_id);
	}

	//object is either entity or literal
	TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
	//int _obj_id = -1;
	bool _is_new_obj = false;
	bool is_obj_entity = _triple.isObjEntity();
	if (is_obj_entity)
	{
		_obj_id = (this->kvstore)->getIDByEntity(_triple.object);

		//if (_obj_id == -1)
		if (_obj_id == INVALID_ENTITY_LITERAL_ID)
		{
			_is_new_obj = true;
			//_obj_id = this->entity_num++;
			_obj_id = this->allocEntityID();
			(this->kvstore)->setIDByEntity(_triple.object, _obj_id);
			(this->kvstore)->setEntityByID(_obj_id, _triple.object);

			//update the string buffer
			//if (_obj_id < this->entity_buffer_size)
			//{
				//this->entity_buffer->set(_obj_id, _triple.object);
			//}

			if (_vertices != NULL)
				_vertices->push_back(_obj_id);
		}
	}
	else
	{
		_obj_id = (this->kvstore)->getIDByLiteral(_triple.object);
		//cout<<"check: "<<_obj_id<<" "<<INVALID_ENTITY_LITERAL_ID<<endl;

		//if (_obj_id == -1)
		if (_obj_id == INVALID_ENTITY_LITERAL_ID)
		{
			_is_new_obj = true;
			//_obj_id = Util::LITERAL_FIRST_ID + this->literal_num;
			_obj_id = this->allocLiteralID();
			//cout<<"this is a new obj id: "<<_obj_id<<endl;
			(this->kvstore)->setIDByLiteral(_triple.object, _obj_id);
			(this->kvstore)->setLiteralByID(_obj_id, _triple.object);
			//cout<<this->kvstore->getLiteralByID(_obj_id)<<endl;
			//cout<<this->kvstore->getIDByLiteral(_triple.object)<<endl;

			//update the string buffer
			//TYPE_ENTITY_LITERAL_ID tid = _obj_id - Util::LITERAL_FIRST_ID;
			//if (tid < this->literal_buffer_size)
			//{
				//this->literal_buffer->set(tid, _triple.object);
			//}

			if (_vertices != NULL)
				_vertices->push_back(_obj_id);
		}
	}

	
	if(txn != nullptr && (this->kvstore)->GetExclusiveLock(_sub_id, _pre_id, _obj_id, txn) == false)
	{
		//conflict
		//abort
		txn->SetState(TransactionState::ABORTED);
		cout << "getExclusiveLocks failed, Abort. TID:" << this_thread::get_id() << endl;
		return false;
	}
	
	//if this is not a new triple, return directly (in case of no transaction)
	if (txn == nullptr)
	{
		bool _triple_exist = false;
		if (!_is_new_sub && !_is_new_pre && !_is_new_obj)
			_triple_exist = this->exist_triple(_sub_id, _pre_id, _obj_id, txn);

		if (_triple_exist)
		{
			cout << "this triple already exist" << endl;
			return false;
		}
		else
		{
			this->triples_num++;	// NOTE: triples_num set here
		}
	}

	//update sp2o op2s s2po o2ps s2o o2s etc.
	bool ret = (this->kvstore)->updateTupleslist_insert(_sub_id, _pre_id, _obj_id, txn);
	if(txn != nullptr)
	{

		if(ret){
			//cout << "WriteSetInsert......." << endl;
			txn->WriteSetInsert(IDTriple(_sub_id, _pre_id, _obj_id));
		}
		else{
			cerr << "insert failed" << endl;
			txn->SetState(TransactionState::ABORTED);
			(this->kvstore)->ReleaseExclusiveLock(_sub_id, _pre_id, _obj_id, txn);
			return false;
		}
	}
	//int* list = NULL;
	//int len = 0;
	//int root = this->kvstore->getIDByEntity("<root>");
	//int contain = this->kvstore->getIDByPredicate("<contain>");
	//this->kvstore->getobjIDlistBysubIDpreID(root, contain, list, len);
	//cout<<root<<" "<<contain<<endl;
	//if(len == 0)
	//{
	//cout<<"no result"<<endl;
	//}
	//for(int i = 0; i < len; ++i)
	//{
	//cout << this->kvstore->getEntityByID(list[i])<<" "<<list[i]<<endl;
	//}

	return true;
	//return updateLen;
}

bool
Database::removeTriple(const TripleWithObjType& _triple, vector<unsigned>* _vertices, vector<unsigned>* _predicates, shared_ptr<Transaction> txn)
{
	long tv_kv_store_begin = Util::get_cur_time();

	TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
	TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
	TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
	if(_triple.isObjEntity())
	{
		_obj_id = (this->kvstore)->getIDByEntity(_triple.object);
	}
	else
	{
		_obj_id = (this->kvstore)->getIDByLiteral(_triple.object);
	}

	//if (_obj_id == -1)
	//if (_obj_id == INVALID_ENTITY_LITERAL_ID)
	//{
		//_obj_id = (this->kvstore)->getIDByLiteral(_triple.object);
	//}

	//if (_sub_id == -1 || _pre_id == -1 || _obj_id == -1)
	if (_sub_id == INVALID_ENTITY_LITERAL_ID || _pre_id == INVALID_PREDICATE_ID || _obj_id == INVALID_ENTITY_LITERAL_ID)
	{
		return false;
	}
	if(txn != nullptr && (this->kvstore)->GetExclusiveLock(_sub_id, _pre_id, _obj_id, txn) == false)
	{
		//conflict
		//abort
		cout << "getExclusiveLocks...................... failed. TID:" << this_thread::get_id() << endl;
		//cout << "getExclusiveLocks failed, Abort. TID:" << this_thread::get_id() << endl;
		txn->SetState(TransactionState::ABORTED);
		return false;
	}
	
	if (txn == nullptr)
	{
		bool _exist_triple = this->exist_triple(_sub_id, _pre_id, _obj_id, txn);
		if (!_exist_triple)
		{
			cout << "triple not exist! " << endl;
			return false;
		}
		else
			this->triples_num--;	// NOTE: triples_num set here
	}

	//cout << "triple existence checked" << endl;

	//remove from sp2o op2s s2po o2ps s2o o2s
	//sub2id, pre2id and obj2id will not be updated
	bool ret = (this->kvstore)->updateTupleslist_remove(_sub_id, _pre_id, _obj_id, txn);
	if(txn != nullptr)
	{
		if(ret)
			txn->WriteSetInsert(IDTriple(_sub_id, _pre_id, _obj_id));
		else{
			cout << " updateTupleslist_remove failed ..............................................." << endl;
			txn->SetState(TransactionState::ABORTED);
			(this->kvstore)->ReleaseExclusiveLock(_sub_id, _pre_id, _obj_id, txn);
			return false;
		}
	}
	//cout << "11 trees updated" << endl;

	long tv_kv_store_end = Util::get_cur_time();
	if(txn == nullptr)
	{
		int sub_degree = (this->kvstore)->getEntityDegree(_sub_id);
		//if subject become an isolated point, remove its corresponding entry
		if (sub_degree == 0)
		{
			//cout<<"to remove entry for sub"<<endl;
			//cout<<_sub_id << " "<<this->kvstore->getEntityByID(_sub_id)<<endl;
			this->kvstore->subEntityByID(_sub_id);
			this->kvstore->subIDByEntity(_triple.subject);
			this->freeEntityID(_sub_id);
			this->sub_num--;
			//update the string buffer
			//if (_sub_id < this->entity_buffer_size)
			//{
				//this->entity_buffer->del(_sub_id);
			//}
			if (_vertices != NULL)
				_vertices->push_back(_sub_id);
		}
		//cout<<"subject dealed"<<endl;
	
		bool is_obj_entity = _triple.isObjEntity();
		int obj_degree;
		if (is_obj_entity)
		{
			obj_degree = this->kvstore->getEntityDegree(_obj_id);
			if (obj_degree == 0)
			{
				//cout<<"to remove entry for obj"<<endl;
				//cout<<_obj_id << " "<<this->kvstore->getEntityByID(_obj_id)<<endl;
				this->kvstore->subEntityByID(_obj_id);
				this->kvstore->subIDByEntity(_triple.object);
				this->freeEntityID(_obj_id);
				//update the string buffer
				//if (_obj_id < this->entity_buffer_size)
				//{
					//this->entity_buffer->del(_obj_id);
				//}
				if (_vertices != NULL)
					_vertices->push_back(_obj_id);
			}
		}
		else
		{
			obj_degree = this->kvstore->getLiteralDegree(_obj_id);
			if (obj_degree == 0)
			{
				this->kvstore->subLiteralByID(_obj_id);
				//cout<<"check after subLiteralByID: "<<_obj_id<<" "<<this->kvstore->getLiteralByID(_obj_id)<<endl;
				this->kvstore->subIDByLiteral(_triple.object);
				this->freeLiteralID(_obj_id);
				//update the string buffer
				//TYPE_ENTITY_LITERAL_ID tid = _obj_id - Util::LITERAL_FIRST_ID;
				//if (tid < this->literal_buffer_size)
				//{
					//this->literal_buffer->del(tid);
				//}
				if (_vertices != NULL)
					_vertices->push_back(_obj_id);
			}
		}
		//cout<<"object dealed"<<endl;

		int pre_degree = this->kvstore->getPredicateDegree(_pre_id);
		if (pre_degree == 0)
		{
			this->kvstore->subPredicateByID(_pre_id);
			this->kvstore->subIDByPredicate(_triple.predicate);
			this->freePredicateID(_pre_id);
			if (_predicates != NULL)
				_predicates->push_back(_pre_id);
		}
		//cout<<"predicate dealed"<<endl;
	}
	
	return true;
}

bool
Database::insert(std::string _rdf_file, bool _is_restore, shared_ptr<Transaction> txn )
{
	bool flag = _is_restore || this->load();
	//bool flag = this->load();
	if (!flag)
	{
		return false;
	}
	cout << "finish loading" << endl;

	long tv_load = Util::get_cur_time();

	TYPE_TRIPLE_NUM success_num = 0;

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		cout << "fail to open : " << _rdf_file << ".@insert_test" << endl;
		//exit(0);
		return false;
	}

	//NOTICE+WARN:we can not load all triples into memory all at once!!!
	//the parameter in build and insert must be the same, because RDF parser also use this
	//for build process, this one can be big enough if memory permits
	//for insert/delete process, this can not be too large, otherwise too costly
	TripleWithObjType* triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
	//parse a file
	RDFParser _parser(_fin);

	TYPE_TRIPLE_NUM triple_num = 0;
#ifdef DEBUG
	Util::logging("==> while(true)");
#endif
	while (true)
	{
		int parse_triple_num = 0;
		_parser.parseFile(triple_array, parse_triple_num);
#ifdef DEBUG
		stringstream _ss;
		//NOTICE:this is not same as others, use parse_triple_num directly
		_ss << "finish rdfparser" << parse_triple_num << endl;
		Util::logging(_ss.str());
		cout << _ss.str() << endl;
#endif
		if (parse_triple_num == 0)
		{
			break;
		}

		//Compress triple here
		//for(int i = 0; i < parse_triple_num; i++)
		//{
			//TripleWithObjType compressed_triple = trie->Compress(triple_array[i], Trie::QUERYMODE);
			//triple_array[i] = compressed_triple;
		//}

		//Process the Triple one by one
		long tv_begin = Util::get_cur_time();
		success_num += this->insert(triple_array, parse_triple_num, _is_restore, txn);
		//success_num += this->batch_insert(triple_array, parse_triple_num, _is_restore, txn);
		long tv_end = Util::get_cur_time();
		cout << "batch insert, used " << (tv_end - tv_begin) << " ms" << endl;
		//some maybe invalid or duplicate
		//triple_num += parse_triple_num;
	}

	delete[] triple_array;
	triple_array = NULL;
	long tv_insert = Util::get_cur_time();
	cout << "after insert, used " << (tv_insert - tv_load) << "ms." << endl;
	//BETTER:update kvstore and vstree separately, to lower the memory cost
	//flag = this->vstree->saveTree();
	//if (!flag)
	//{
		//return false;
	//}
	//flag = this->saveDBInfoFile();
	//if (!flag)
	//{
		//return false;
	//}

	cout << "insert rdf triples done." << endl;
	cout<<"inserted triples num: "<<success_num<<endl;

	//int* list = NULL;
	//int len = 0;
	//int root = this->kvstore->getIDByEntity("<root>");
	//int contain = this->kvstore->getIDByPredicate("<contain>");
	//this->kvstore->getobjIDlistBysubIDpreID(root, contain, list, len);
	//cout<<root<<" "<<contain<<endl;
	//if(len == 0)
	//{
	//cout<<"no result"<<endl;
	//}
	//for(int i = 0; i < len; ++i)
	//{
	//cout << this->kvstore->getEntityByID(list[i])<<" "<<list[i]<<endl;
	//}
	//cout<<endl;
	//int t = this->kvstore->getIDByEntity("<node5>");
	//cout<<t<<endl;
	//cout<<this->kvstore->getEntityByID(0)<<endl;
	this->kvstore->set_if_single_thread(false);
	return true;
}

bool
Database::remove(std::string _rdf_file, bool _is_restore, shared_ptr<Transaction> txn)
{
	bool flag = _is_restore || this->load();
	//bool flag = this->load();
	if (!flag)
	{
		return false;
	}
	cout << "finish loading" << endl;

	long tv_load = Util::get_cur_time();
	TYPE_TRIPLE_NUM success_num = 0;

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		cout << "fail to open : " << _rdf_file << ".@remove_test" << endl;
		return false;
	}

	//NOTICE+WARN:we can not load all triples into memory all at once!!!
	TripleWithObjType* triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
	//parse a file
	RDFParser _parser(_fin);

	//int triple_num = 0;
#ifdef DEBUG
	Util::logging("==> while(true)");
#endif
	while (true)
	{
		int parse_triple_num = 0;
		_parser.parseFile(triple_array, parse_triple_num);
#ifdef DEBUG
		stringstream _ss;
		//NOTICE:this is not same as others, use parse_triple_num directly
		_ss << "finish rdfparser" << parse_triple_num << endl;
		Util::logging(_ss.str());
		cout << _ss.str() << endl;
#endif
		if (parse_triple_num == 0)
		{
			break;
		}

		//Compress triple
		//for(int i = 0; i < parse_triple_num; i++)
		//{
			//TripleWithObjType compressed_triple = trie->Compress(triple_array[i], Trie::QUERYMODE);
			//triple_array[i] = compressed_triple;
		//}

		long tv_begin = Util::get_cur_time();
		success_num += this->remove(triple_array, parse_triple_num, _is_restore, txn);
		long tv_end = Util::get_cur_time();
		cout << "batch remove, used " << (tv_end - tv_begin) << " ms" << endl;
		//some maybe invalid or duplicate
		//triple_num -= parse_triple_num;
	}

	//BETTER: free this just after id_tuples are ok
	//(only when using group insertion/deletion)
	//or reduce the array size
	delete[] triple_array;
	triple_array = NULL;
	long tv_remove = Util::get_cur_time();
	cout << "after remove, used " << (tv_remove - tv_load) << "ms." << endl;

	//flag = this->vstree->saveTree();
	//if (!flag)
	//{
		//return false;
	//}
	//flag = this->saveDBInfoFile();
	//if (!flag)
	//{
		//return false;
	//}

	cout << "remove rdf triples done." << endl;
	cout<<"removed triples num: "<<success_num<<endl;

	//if(this->vstree->isEmpty())
	if(this->triples_num == 0)
	{
		this->resetIDinfo();
	}
	this->kvstore->set_if_single_thread(false);
	return true;
}

unsigned
Database::insert(const TripleWithObjType* _triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore, shared_ptr<Transaction> txn)
{
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	TYPE_TRIPLE_NUM valid_num = 0;
	bool logging = false;
	if (!_is_restore) {
		if(logging)
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

#ifdef USE_GROUP_INSERT
	//NOTICE:this is called by insert(file) or query()(but can not be too large),
	//assume that db is loaded already
	TYPE_ENTITY_LITERAL_ID** id_tuples = new TYPE_ENTITY_LITERAL_ID*[_triple_num];

	//TODO:change the type in this catagolory
	int i = 0;
	//for(i = 0; i < _triple_num; ++i)
	//{
	//id_tuples[i] = new int[3];
	//}
	map<int, EntityBitSet> old_sigmap;
	map<int, EntityBitSet> new_sigmap;
	set<int> new_entity;
	map<int, EntityBitSet>::iterator it;
	EntityBitSet tmpset;
	tmpset.reset();

	int subid, objid, preid;
	bool is_obj_entity;
	for (i = 0; i < _triple_num; ++i)
	{
		bool is_new_sub = false, is_new_pre = false, is_new_obj = false;

		string sub = _triples[i].getSubject();
		subid = this->kvstore->getIDByEntity(sub);
		if (subid == -1)
		{
			is_new_sub = true;
			subid = this->allocEntityID();
#ifdef DEBUG
			//cout << "this is a new subject: " << sub << " " << subid << endl;
#endif
			this->sub_num++;
			this->kvstore->setIDByEntity(sub, subid);
			this->kvstore->setEntityByID(subid, sub);
			new_entity.insert(subid);
			//add info and update buffer
			vertices.push_back(subid);
			//if (subid < this->entity_buffer_size)
			//{
				//this->entity_buffer->set(subid, sub);
			//}
		}

		string pre = _triples[i].getPredicate();
		preid = this->kvstore->getIDByPredicate(pre);
		if (preid == -1)
		{
			is_new_pre = true;
			preid = this->allocPredicateID();
			this->kvstore->setIDByPredicate(pre, preid);
			this->kvstore->setPredicateByID(preid, pre);
			predicates.push_back(preid);
		}

		is_obj_entity = _triples[i].isObjEntity();
		string obj = _triples[i].getObject();
		if (is_obj_entity)
		{
			objid = this->kvstore->getIDByEntity(obj);
			if (objid == -1)
			{
				is_new_obj = true;
				objid = this->allocEntityID();
#ifdef DEBUG
				//cout << "this is a new object: " << obj << " " << objid << endl;
#endif
				//this->obj_num++;
				this->kvstore->setIDByEntity(obj, objid);
				this->kvstore->setEntityByID(objid, obj);
				new_entity.insert(objid);
				//add info and update
				vertices.push_back(objid);
				//if (objid < this->entity_buffer_size)
				//{
					//this->entity_buffer->set(objid, obj);
				//}
			}
		}
		else //isObjLiteral
		{
			objid = this->kvstore->getIDByLiteral(obj);
			if (objid == -1)
			{
				is_new_obj = true;
				objid = this->allocLiteralID();
				//this->obj_num++;
				this->kvstore->setIDByLiteral(obj, objid);
				this->kvstore->setLiteralByID(objid, obj);
				//add info and update
				vertices.push_back(objid);
				//int tid = objid - Util::LITERAL_FIRST_ID;
				//if (tid < this->literal_buffer_size)
				//{
					//this->literal_buffer->set(tid, obj);
				//}
			}
		}

		bool triple_exist = false;
		if (!is_new_sub && !is_new_pre && !is_new_obj)
		{
			triple_exist = this->exist_triple(subid, preid, objid);
		}
		if (triple_exist)
		{
#ifdef DEBUG
			cout << "this triple exist" << endl;
#endif
			continue;
		}
#ifdef DEBUG
		cout << "this triple not exist" << endl;
#endif

		id_tuples[valid_num] = new int[3];
		id_tuples[valid_num][0] = subid;
		id_tuples[valid_num][1] = preid;
		id_tuples[valid_num][2] = objid;
		this->triples_num++;	// NOTE: triples_num set here
		valid_num++;

		tmpset.reset();
		Signature::encodePredicate2Entity(preid, tmpset, Util::EDGE_OUT);
		Signature::encodeStr2Entity(obj.c_str(), tmpset);
		if (new_entity.find(subid) != new_entity.end())
		{
			it = new_sigmap.find(subid);
			if (it != new_sigmap.end())
			{
				it->second |= tmpset;
			}
			else
			{
				new_sigmap[subid] = tmpset;
			}
		}
		else
		{
			it = old_sigmap.find(subid);
			if (it != old_sigmap.end())
			{
				it->second |= tmpset;
			}
			else
			{
				old_sigmap[subid] = tmpset;
			}
		}

		if (is_obj_entity)
		{
			tmpset.reset();
			Signature::encodePredicate2Entity(preid, tmpset, Util::EDGE_IN);
			Signature::encodeStr2Entity(sub.c_str(), tmpset);
			if (new_entity.find(objid) != new_entity.end())
			{
				it = new_sigmap.find(objid);
				if (it != new_sigmap.end())
				{
					it->second |= tmpset;
				}
				else
				{
					new_sigmap[objid] = tmpset;
				}
			}
			else
			{
				it = old_sigmap.find(objid);
				if (it != old_sigmap.end())
				{
					it->second |= tmpset;
				}
				else
				{
					old_sigmap[objid] = tmpset;
				}
			}
		}
	}

#ifdef DEBUG
	cout << "old sigmap size: " << old_sigmap.size() << endl;
	cout << "new sigmap size: " << new_sigmap.size() << endl;
	cout << "valid num: " << valid_num << endl;
#endif

	//NOTICE:need to sort and remove duplicates, update the valid num
	//Notice that duplicates in a group can csuse problem
	//We finish this by spo cmp

	//this->kvstore->updateTupleslist_insert(_sub_id, _pre_id, _obj_id);
	//sort and update kvstore: 11 indexes
	//
	//BETTER:maybe also use int* here with a size to start
	//NOTICE:all kvtrees are opened now, one by one if memory is bottleneck
	//
	//spo cmp: s2p s2o s2po sp2o
	{
#ifdef DEBUG
		cout << "INSRET PROCESS: to spo cmp and update" << endl;
#endif
#ifndef PARALLEL_SORT
		qsort(id_tuples, valid_num, sizeof(int*), KVstore::_spo_cmp);
#else
		omp_set_num_threads(thread_num);
		__gnu_parallel::sort(id_tuples, id_tuples + valid_num, KVstore::parallel_spo_cmp);
#endif
		
		//To remove duplicates
		//int ti = 1, tj = 1;
		//while(tj < valid_num)
		//{
		//if(id_tuples[tj][0] != id_tuples[tj-1][0] || id_tuples[tj][1] != id_tuples[tj-1][1] || id_tuples[tj][2] != id_tuples[tj-1][2])
		//{
		//id_tuples[ti][0] = id_tuples[tj][0];
		//id_tuples[ti][1] = id_tuples[tj][1];
		//id_tuples[ti][2] = id_tuples[tj][2];
		//ti++;
		//}
		//tj++;
		//}
		//for(tj = ti; tj < valid_num; ++tj)
		//{
		//delete[] id_tuples[tj];
		//id_tuples[tj] = NULL;
		//}
		//valid_num = ti;
		//
		//Notice that below already consider duplicates in loop

		vector<int> oidlist_s;
		vector<int> pidlist_s;
		vector<int> oidlist_sp;
		vector<int> pidoidlist_s;

		bool _sub_change = true;
		bool _sub_pre_change = true;
		bool _pre_change = true;

		for (int i = 0; i < valid_num; ++i)
			if (i + 1 == valid_num || (id_tuples[i][0] != id_tuples[i + 1][0] || id_tuples[i][1] != id_tuples[i + 1][1] || id_tuples[i][2] != id_tuples[i + 1][2]))
			{
				int _sub_id = id_tuples[i][0];
				int _pre_id = id_tuples[i][1];
				int _obj_id = id_tuples[i][2];

				oidlist_s.push_back(_obj_id);
				oidlist_sp.push_back(_obj_id);
				pidoidlist_s.push_back(_pre_id);
				pidoidlist_s.push_back(_obj_id);
				pidlist_s.push_back(_pre_id);

				_sub_change = (i + 1 == valid_num) || (id_tuples[i][0] != id_tuples[i + 1][0]);
				_pre_change = (i + 1 == valid_num) || (id_tuples[i][1] != id_tuples[i + 1][1]);
				_sub_pre_change = _sub_change || _pre_change;

				if (_sub_pre_change)
				{
#ifdef DEBUG
					cout << "update sp2o: " << _sub_id << " " << _pre_id << " " << oidlist_sp.size() << endl;
#endif
					cout << this->kvstore->getEntityByID(_sub_id) << endl;
					cout << this->kvstore->getPredicateByID(_pre_id) << endl;
					//this->kvstore->updateInsert_sp2o(_sub_id, _pre_id, oidlist_sp);
					oidlist_sp.clear();
				}

				if (_sub_change)
				{
#ifdef DEBUG
					cout << "update s2p: " << _sub_id << " " << pidlist_s.size() << endl;
#endif
					//this->kvstore->updateInsert_s2p(_sub_id, pidlist_s);
					pidlist_s.clear();

#ifdef DEBUG
					cout << "update s2po: " << _sub_id << " " << pidoidlist_s.size() << endl;
#endif
					this->kvstore->updateInsert_s2values(_sub_id, pidoidlist_s);
					pidoidlist_s.clear();

#ifdef DEBUG
					cout << "update s2o: " << _sub_id << " " << oidlist_s.size() << endl;
#endif
#ifndef PARALLEL_SORT
					sort(oidlist_s.begin(), oidlist_s.end());
#else
					omp_set_num_threads(thread_num);
					__gnu_parallel::sort(oidlist_s.begin(), oidlist_s.end());
#endif
					//this->kvstore->updateInsert_s2o(_sub_id, oidlist_s);
					oidlist_s.clear();
				}

			}
#ifdef DEBUG
		cout << "INSERT PROCESS: OUT s2po..." << endl;
#endif
	}
	//ops cmp: o2p o2s o2ps op2s
	{
#ifdef DEBUG
		cout << "INSRET PROCESS: to ops cmp and update" << endl;
#endif
#ifndef PARALLEL_SORT
		qsort(id_tuples, valid_num, sizeof(int**), KVstore::_ops_cmp);
#else
		omp_set_num_threads(thread_num);
		__gnu_parallel::sort(id_tuples, id_tuples + valid_num, KVstore::parallel_ops_cmp);
#endif
		vector<int> sidlist_o;
		vector<int> sidlist_op;
		vector<int> pidsidlist_o;
		vector<int> pidlist_o;

		bool _obj_change = true;
		bool _pre_change = true;
		bool _obj_pre_change = true;

		for (int i = 0; i < valid_num; ++i)
			if (i + 1 == valid_num || (id_tuples[i][0] != id_tuples[i + 1][0] || id_tuples[i][1] != id_tuples[i + 1][1] || id_tuples[i][2] != id_tuples[i + 1][2]))
			{
				int _sub_id = id_tuples[i][0];
				int _pre_id = id_tuples[i][1];
				int _obj_id = id_tuples[i][2];

				sidlist_o.push_back(_sub_id);
				sidlist_op.push_back(_sub_id);
				pidsidlist_o.push_back(_pre_id);
				pidsidlist_o.push_back(_sub_id);
				pidlist_o.push_back(_pre_id);

				_obj_change = (i + 1 == valid_num) || (id_tuples[i][2] != id_tuples[i + 1][2]);
				_pre_change = (i + 1 == valid_num) || (id_tuples[i][1] != id_tuples[i + 1][1]);
				_obj_pre_change = _obj_change || _pre_change;

				if (_obj_pre_change)
				{
#ifdef DEBUG
					cout << "update op2s: " << _obj_id << " " << _pre_id << " " << sidlist_op.size() << endl;
#endif
					//this->kvstore->updateInsert_op2s(_obj_id, _pre_id, sidlist_op);
					sidlist_op.clear();
				}

				if (_obj_change)
				{
#ifdef DEBUG
					cout << "update o2s: " << _obj_id << " " << sidlist_o.size() << endl;
#endif
#ifndef PARALLEL_SORT
					sort(sidlist_o.begin(), sidlist_o.end());
#else
					omp_set_num_threads(thread_num);
					__gnu_parallel::sort(sidlist_o.begin(), sidlist_o.end());
#endif
					//this->kvstore->updateInsert_o2s(_obj_id, sidlist_o);
					sidlist_o.clear();

#ifdef DEBUG
					cout << "update o2ps: " << _obj_id << " " << pidsidlist_o.size() << endl;
#endif
					this->kvstore->updateInsert_o2values(_obj_id, pidsidlist_o);
					pidsidlist_o.clear();

#ifdef DEBUG
					cout << "update o2p: " << _obj_id << " " << pidlist_o.size() << endl;
#endif
					//this->kvstore->updateInsert_o2p(_obj_id, pidlist_o);
					pidlist_o.clear();
				}

			}
#ifdef DEBUG
		cout << "INSERT PROCESS: OUT o2ps..." << endl;
#endif
	}
	//pso cmp: p2s p2o p2so
	{
#ifdef DEBUG
		cout << "INSRET PROCESS: to pso cmp and update" << endl;
#endif
#ifndef PARALLEL_SORT
		qsort(id_tuples, valid_num, sizeof(int*), KVstore::_pso_cmp);
#else
		omp_set_num_threads(thread_num);
		__gnu_parallel::sort(id_tuples, id_tuples + valid_num,  KVstore::parallel_pso_cmp);
#endif
		vector<int> sidlist_p;
		vector<int> oidlist_p;
		vector<int> sidoidlist_p;

		bool _pre_change = true;
		bool _sub_change = true;
		//bool _pre_sub_change = true;

		for (int i = 0; i < valid_num; i++)
			if (i + 1 == valid_num || (id_tuples[i][0] != id_tuples[i + 1][0] || id_tuples[i][1] != id_tuples[i + 1][1] || id_tuples[i][2] != id_tuples[i + 1][2]))
			{
				int _sub_id = id_tuples[i][0];
				int _pre_id = id_tuples[i][1];
				int _obj_id = id_tuples[i][2];

				oidlist_p.push_back(_obj_id);
				sidoidlist_p.push_back(_sub_id);
				sidoidlist_p.push_back(_obj_id);
				sidlist_p.push_back(_sub_id);

				_pre_change = (i + 1 == valid_num) || (id_tuples[i][1] != id_tuples[i + 1][1]);
				_sub_change = (i + 1 == valid_num) || (id_tuples[i][0] != id_tuples[i + 1][0]);
				//_pre_sub_change = _pre_change || _sub_change;

				if (_pre_change)
				{
#ifdef DEBUG
					cout << "update p2s: " << _pre_id << " " << sidlist_p.size() << endl;
#endif
					//this->kvstore->updateInsert_p2s(_pre_id, sidlist_p);
					sidlist_p.clear();

#ifdef DEBUG
					cout << "update p2o: " << _pre_id << " " << oidlist_p.size() << endl;
#endif
#ifndef PARALLEL_SORT
					sort(oidlist_p.begin(), oidlist_p.end());
#else
					omp_set_num_threads(thread_num);
					__gnu_parallel::sort(oidlist_p.begin(), oidlist_p.end());
#endif
					//this->kvstore->updateInsert_p2o(_pre_id, oidlist_p);
					oidlist_p.clear();

#ifdef DEBUG
					cout << "update p2so: " << _pre_id << " " << sidoidlist_p.size() << endl;
#endif
					this->kvstore->updateInsert_p2values(_pre_id, sidoidlist_p);
					sidoidlist_p.clear();
				}
			}
#ifdef DEBUG
		cout << "INSERT PROCESS: OUT p2so..." << endl;
#endif
	}


	for (int i = 0; i < valid_num; ++i)
	{
		delete[] id_tuples[i];
	}
	delete[] id_tuples;
	id_tuples = NULL;

	//for (it = old_sigmap.begin(); it != old_sigmap.end(); ++it)
	//{
		//this->vstree->updateEntry(it->first, it->second);
	//}
	//for (it = new_sigmap.begin(); it != new_sigmap.end(); ++it)
	//{
		//SigEntry _sig(it->first, it->second);
		//this->vstree->insertEntry(_sig);
	//}
#else //USE_GROUP_INSERT
	//NOTICE:we deal with insertions one by one here
	//Callers should save the vstree(node and info) after calling this function
	for (TYPE_TRIPLE_NUM i = 0; i < _triple_num; ++i)
	{
		bool ret = this->insertTriple(_triples[i], &vertices, &predicates, txn);
		if(ret)
		{
			valid_num++;
		}
	}
#endif //USE_GROUP_INSERT

	//this->stringindex->SetTrie(kvstore->getTrie());
	//update string index
	this->stringindex->change(vertices, *this->kvstore, true);
	this->stringindex->change(predicates, *this->kvstore, false);

	return valid_num;
}

unsigned
Database::remove(const TripleWithObjType* _triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore, shared_ptr<Transaction> txn)
{
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	TYPE_TRIPLE_NUM valid_num = 0;

	if (!_is_restore) {
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

#ifdef USE_GROUP_DELETE
	//NOTICE:this is called by remove(file) or query()(but can not be too large),
	//assume that db is loaded already
	TYPE_ENTITY_LITERAL_ID** id_tuples = new TYPE_ENTITY_LITERAL_ID*[_triple_num];
	TYPE_TRIPLE_NUM i = 0;

	//TODO: change type in this catagolory

	//for(i = 0; i < _triple_num; ++i)
	//{
	//id_tuples[i] = new int[3];
	//}
	//map<int, EntityBitSet> sigmap;
	//map<int, EntityBitSet>::iterator it;
	EntityBitSet tmpset;
	tmpset.reset();

	TYPE_ENTITY_LITERAL_ID subid, objid;
	TYPE_PREDICATE_ID preid;
	bool is_obj_entity;
	for (i = 0; i < _triple_num; ++i)
	{
		string sub = _triples[i].getSubject();
		subid = this->kvstore->getIDByEntity(sub);
		//if(subid == -1)
		if(subid == INVALID_ENTITY_LITERAL_ID)
		{
			continue;
		}

		string pre = _triples[i].getPredicate();
		preid = this->kvstore->getIDByPredicate(pre);
		//if(preid == -1)
		if(preid == INVALID_PREDICATE_ID)
		{
			continue;
		}

		is_obj_entity = _triples[i].isObjEntity();
		string obj = _triples[i].getObject();
		if (is_obj_entity)
		{
			objid = this->kvstore->getIDByEntity(obj);
		}
		else //isObjLiteral
		{
			objid = this->kvstore->getIDByLiteral(obj);
		}
		//if(objid == -1)
		if(objid == INVALID_ENTITY_LITERAL_ID)
		{
			continue;
		}

		//if (subid == -1 || preid == -1 || objid == -1)
		//{
			//continue;
		//}
		bool _exist_triple = this->exist_triple(subid, preid, objid);
		if (!_exist_triple)
		{
			continue;
		}

		id_tuples[valid_num] = new TYPE_ENTITY_LITERAL_ID[3];
		id_tuples[valid_num][0] = subid;
		id_tuples[valid_num][1] = preid;
		id_tuples[valid_num][2] = objid;
		this->triples_num--;	// NOTE: triples_num set here
		valid_num++;
	}

	//NOTICE:sort and remove duplicates, update the valid num
	//Notice that duplicates in a group can cause problem

	int sub_degree, obj_degree, pre_degree;
	string tmpstr;
	//sort and update kvstore: 11 indexes
	//
	//BETTER:maybe also use int* here with a size to start
	//NOTICE:all kvtrees are opened now, one by one if memory is bottleneck
	//
	//spo cmp: s2p s2o s2po sp2o
	{
#ifdef DEBUG
		cout << "INSRET PROCESS: to spo cmp and update" << endl;
#endif
#ifndef PARALLEL_SORT
		qsort(id_tuples, valid_num, sizeof(int*), KVstore::_spo_cmp);
#else
		omp_set_num_threads(thread_num);
		__gnu_parallel::sort(id_tuples, id_tuples + valid_num, KVstore::parallel_spo_cmp);
#endif
		vector<int> oidlist_s;
		vector<int> pidlist_s;
		vector<int> oidlist_sp;
		vector<int> pidoidlist_s;

		bool _sub_change = true;
		bool _sub_pre_change = true;
		bool _pre_change = true;

		for (int i = 0; i < valid_num; ++i)
			if (i + 1 == valid_num || (id_tuples[i][0] != id_tuples[i + 1][0] || id_tuples[i][1] != id_tuples[i + 1][1] || id_tuples[i][2] != id_tuples[i + 1][2]))
			{
				int _sub_id = id_tuples[i][0];
				int _pre_id = id_tuples[i][1];
				int _obj_id = id_tuples[i][2];

				oidlist_s.push_back(_obj_id);
				oidlist_sp.push_back(_obj_id);
				pidoidlist_s.push_back(_pre_id);
				pidoidlist_s.push_back(_obj_id);
				pidlist_s.push_back(_pre_id);

				_sub_change = (i + 1 == valid_num) || (id_tuples[i][0] != id_tuples[i + 1][0]);
				_pre_change = (i + 1 == valid_num) || (id_tuples[i][1] != id_tuples[i + 1][1]);
				_sub_pre_change = _sub_change || _pre_change;

				if (_sub_pre_change)
				{
					this->kvstore->updateRemove_sp2o(_sub_id, _pre_id, oidlist_sp);
					oidlist_sp.clear();
				}

				if (_sub_change)
				{
					this->kvstore->updateRemove_s2p(_sub_id, pidlist_s);
					pidlist_s.clear();
					this->kvstore->updateRemove_s2po(_sub_id, pidoidlist_s);
					pidoidlist_s.clear();

#ifndef PARALLEL_SORT
					sort(oidlist_s.begin(), oidlist_s.end());
#else
					omp_set_num_threads(thread_num);
					__gnu_parallel::sort(oidlist_s.begin(), oidlist_s.end());
#endif
					this->kvstore->updateRemove_s2o(_sub_id, oidlist_s);
					oidlist_s.clear();

					sub_degree = (this->kvstore)->getEntityDegree(_sub_id);
					if (sub_degree == 0)
					{
						tmpstr = this->kvstore->getEntityByID(_sub_id);
						this->kvstore->subEntityByID(_sub_id);
						this->kvstore->subIDByEntity(tmpstr);
						//(this->vstree)->removeEntry(_sub_id);
						this->freeEntityID(_sub_id);
						this->sub_num--;
						//add info and update buffer
						vertices.push_back(_sub_id);
						//if (_sub_id < this->entity_buffer_size)
						//{
							//this->entity_buffer->del(_sub_id);
						//}
					}
					else
					{
						tmpset.reset();
						this->calculateEntityBitSet(_sub_id, tmpset);
						//this->vstree->replaceEntry(_sub_id, tmpset);
					}
				}

			}
#ifdef DEBUG
		cout << "INSERT PROCESS: OUT s2po..." << endl;
#endif
	}
	//ops cmp: o2p o2s o2ps op2s
	{
#ifdef DEBUG
		cout << "INSRET PROCESS: to ops cmp and update" << endl;
#endif
#ifndef PARALLEL_SORT
		qsort(id_tuples, valid_num, sizeof(int**), KVstore::_ops_cmp);
#else
		omp_set_num_threads(thread_num);
		__gnu_parallel::sort(id_tuples, id_tuples + valid_num, KVstore::parallel_ops_cmp);
#endif
		vector<int> sidlist_o;
		vector<int> sidlist_op;
		vector<int> pidsidlist_o;
		vector<int> pidlist_o;

		bool _obj_change = true;
		bool _pre_change = true;
		bool _obj_pre_change = true;

		for (int i = 0; i < valid_num; ++i)
			if (i + 1 == valid_num || (id_tuples[i][0] != id_tuples[i + 1][0] || id_tuples[i][1] != id_tuples[i + 1][1] || id_tuples[i][2] != id_tuples[i + 1][2]))
			{
				int _sub_id = id_tuples[i][0];
				int _pre_id = id_tuples[i][1];
				int _obj_id = id_tuples[i][2];

				sidlist_o.push_back(_sub_id);
				sidlist_op.push_back(_sub_id);
				pidsidlist_o.push_back(_pre_id);
				pidsidlist_o.push_back(_sub_id);
				pidlist_o.push_back(_pre_id);

				_obj_change = (i + 1 == valid_num) || (id_tuples[i][2] != id_tuples[i + 1][2]);
				_pre_change = (i + 1 == valid_num) || (id_tuples[i][1] != id_tuples[i + 1][1]);
				_obj_pre_change = _obj_change || _pre_change;

				if (_obj_pre_change)
				{
					this->kvstore->updateRemove_op2s(_obj_id, _pre_id, sidlist_op);
					sidlist_op.clear();
				}

				if (_obj_change)
				{
#ifndef PARALLEL_SORT
					sort(sidlist_o.begin(), sidlist_o.end());
#else
					omp_set_num_threads(thread_num);
					__gnu_parallel::sort(sidlist_o.begin(), sidlist_o.end());
#endif
					this->kvstore->updateRemove_o2s(_obj_id, sidlist_o);
					sidlist_o.clear();
					this->kvstore->updateRemove_o2ps(_obj_id, pidsidlist_o);
					pidsidlist_o.clear();

					this->kvstore->updateRemove_o2p(_obj_id, pidlist_o);
					pidlist_o.clear();

					is_obj_entity = this->objIDIsEntityID(_obj_id);
					if (is_obj_entity)
					{
						obj_degree = this->kvstore->getEntityDegree(_obj_id);
						if (obj_degree == 0)
						{
							tmpstr = this->kvstore->getEntityByID(_obj_id);
							this->kvstore->subEntityByID(_obj_id);
							this->kvstore->subIDByEntity(tmpstr);
							//(this->vstree)->removeEntry(_obj_id);
							this->freeEntityID(_obj_id);
							//add info and update buffer
							vertices.push_back(_obj_id);
							//if (_obj_id < this->entity_buffer_size)
							//{
								//this->entity_buffer->del(_obj_id);
							//}
						}
						else
						{
							tmpset.reset();
							//this->calculateEntityBitSet(_obj_id, tmpset);
							//this->vstree->replaceEntry(_obj_id, tmpset);
						}
					}
					else
					{
						obj_degree = this->kvstore->getLiteralDegree(_obj_id);
						if (obj_degree == 0)
						{
							tmpstr = this->kvstore->getLiteralByID(_obj_id);
							this->kvstore->subLiteralByID(_obj_id);
							this->kvstore->subIDByLiteral(tmpstr);
							this->freeLiteralID(_obj_id);
							//add info and update buffer
							vertices.push_back(_obj_id);
							//int tid = _obj_id - Util::LITERAL_FIRST_ID;
							//if (tid < this->literal_buffer_size)
							//{
								//this->literal_buffer->del(tid);
							//}
						}
					}
				}

			}
#ifdef DEBUG
		cout << "INSERT PROCESS: OUT o2ps..." << endl;
#endif
	}
	//pso cmp: p2s p2o p2so
	{
#ifdef DEBUG
		cout << "INSRET PROCESS: to pso cmp and update" << endl;
#endif
#ifndef PARALLEL_SORT
		qsort(id_tuples, valid_num, sizeof(int*), KVstore::_pso_cmp);
#else
		omp_set_num_threads(thread_num);
		__gnu_parallel::sort(id_tuples, id_tuples + valid_num, KVstore::parallel_pso_cmp);
#endif
		vector<int> sidlist_p;
		vector<int> oidlist_p;
		vector<int> sidoidlist_p;

		bool _pre_change = true;
		bool _sub_change = true;
		//bool _pre_sub_change = true;

		for (int i = 0; i < valid_num; i++)
			if (i + 1 == valid_num || (id_tuples[i][0] != id_tuples[i + 1][0] || id_tuples[i][1] != id_tuples[i + 1][1] || id_tuples[i][2] != id_tuples[i + 1][2]))
			{
				int _sub_id = id_tuples[i][0];
				int _pre_id = id_tuples[i][1];
				int _obj_id = id_tuples[i][2];

				oidlist_p.push_back(_obj_id);
				sidoidlist_p.push_back(_sub_id);
				sidoidlist_p.push_back(_obj_id);
				sidlist_p.push_back(_sub_id);

				_pre_change = (i + 1 == valid_num) || (id_tuples[i][1] != id_tuples[i + 1][1]);
				_sub_change = (i + 1 == valid_num) || (id_tuples[i][0] != id_tuples[i + 1][0]);
				//_pre_sub_change = _pre_change || _sub_change;

				if (_pre_change)
				{
					this->kvstore->updateRemove_p2s(_pre_id, sidlist_p);
					sidlist_p.clear();

#ifndef PARALLEL_SORT
					sort(oidlist_p.begin(), oidlist_p.end());
#else
					omp_set_num_threads(thread_num);
					__gnu_parallel::sort(oidlist_p.begin(), oidlist_p.end());
#endif
					this->kvstore->updateRemove_p2o(_pre_id, oidlist_p);
					oidlist_p.clear();

					this->kvstore->updateRemove_p2so(_pre_id, sidoidlist_p);
					sidoidlist_p.clear();

					pre_degree = this->kvstore->getPredicateDegree(_pre_id);
					if (pre_degree == 0)
					{
						tmpstr = this->kvstore->getPredicateByID(_pre_id);
						this->kvstore->subPredicateByID(_pre_id);
						this->kvstore->subIDByPredicate(tmpstr);
						this->freePredicateID(_pre_id);
						//add pre info
						predicates.push_back(_pre_id);
					}
				}
			}
#ifdef DEBUG
		cout << "INSERT PROCESS: OUT p2so..." << endl;
#endif
	}


	for (int i = 0; i < valid_num; ++i)
	{
		delete[] id_tuples[i];
	}
	delete[] id_tuples;
	id_tuples = NULL;
#else
	//NOTICE:we deal with deletions one by one here
	//Callers should save the vstree(node and info) after calling this function
	for (TYPE_TRIPLE_NUM i = 0; i < _triple_num; ++i)
	{
		bool ret = this->removeTriple(_triples[i], &vertices, &predicates, txn);
		if(ret)
		{
			valid_num++;
		}
	}
#endif
	if(txn != nullptr)
	{
		this->stringindex->SetTrie(kvstore->getTrie());
		//update string index
		this->stringindex->disable(vertices, true);
		this->stringindex->disable(predicates, false);
	}
	//BETTER+TODO:this will require us to lock all when remove process not ends(in multiple threads cases)
	//An considerable idea is to check if empty after every triple removed
	//if(this->vstree->isEmpty())
	if(this->triples_num == 0)
	{
		this->resetIDinfo();
	}

	return valid_num;
}

unsigned
Database::batch_insert(std::string _rdf_file, bool _is_restore, shared_ptr<Transaction> txn )
{
	bool flag = _is_restore || this->load();
	if (!flag)
	{
		return -1;
	}
	cout << "finish loading" << endl;

	long tv_load = Util::get_cur_time();

	unsigned success_num = 0;

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		cout << "fail to open : " << _rdf_file << ".@insert_test" << endl;
		//exit(0);
		return -1;
	}

	//NOTICE+WARN:we can not load all triples into memory all at once!!!
	//the parameter in build and insert must be the same, because RDF parser also use this
	//for build process, this one can be big enough if memory permits
	//for insert/delete process, this can not be too large, otherwise too costly
	TripleWithObjType* triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
	//parse a file
	RDFParser _parser(_fin);

	TYPE_TRIPLE_NUM triple_num = 0;
	while (true)
	{
		int parse_triple_num = 0;
		_parser.parseFile(triple_array, parse_triple_num);
		if (parse_triple_num == 0)
		{
			break;
		}
		long tv_begin = Util::get_cur_time();
		success_num += this->batch_insert(triple_array, parse_triple_num, _is_restore, txn);
		long tv_end = Util::get_cur_time();
		cout << "batch insert, used " << (tv_end - tv_begin) << " ms" << endl;
	}

	delete[] triple_array;
	triple_array = NULL;
	long tv_insert = Util::get_cur_time();
	cout << "after batch insert, used " << (tv_insert - tv_load) << "ms." << endl;

	cout << "insert rdf triples done." << endl;
	cout<<"inserted triples num: "<<success_num<<endl;

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
	cout << "finish loading" << endl;

	long tv_load = Util::get_cur_time();
	unsigned success_num = 0;

	ifstream _fin(_rdf_file.c_str());
	if (!_fin)
	{
		cout << "fail to open : " << _rdf_file << ".@remove_test" << endl;
		return -1;
	}

	//NOTICE+WARN:we can not load all triples into memory all at once!!!
	TripleWithObjType* triple_array = new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP];
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
		cout << "batch remove, used " << (tv_end - tv_begin) << " ms" << endl;

	}

	delete[] triple_array;
	triple_array = NULL;
	long tv_remove = Util::get_cur_time();
	cout << "after batch remove, used " << (tv_remove - tv_load) << "ms." << endl;
	cout << "remove rdf triples done." << endl;
	cout<<"removed triples num: "<<success_num<<endl;

	if(this->triples_num == 0)
	{
		this->resetIDinfo();
	}
	return success_num;
}

//WARNING: TRANSACTIONAL batch insert is not completed yet!
unsigned
Database::batch_insert(const TripleWithObjType* _triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore, shared_ptr<Transaction> txn)
{
	if(_triple_num == 0) return 0;
	TYPE_TRIPLE_NUM valid_num = 0;
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	unsigned update_num_s = 0;
	unsigned update_num_p = 0;
	unsigned update_num_o = 0;
	unsigned update_num_subject=0;

	if (!_is_restore) {
		write_update_log(_triples, _triple_num, 1, txn);
	}
	bool is_obj_entity;
	vector<ID_TUPLE> id_tuples(_triple_num);
	for (int i = 0; i < _triple_num; ++i)
	{
		TripleWithObjType _triple = _triples[i];
		TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
		if (_sub_id == INVALID_ENTITY_LITERAL_ID)
		{
			_sub_id = this->allocEntityID();
			update_num_subject=update_num_subject+1;
			(this->kvstore)->setIDByEntity(_triple.subject, _sub_id);
			(this->kvstore)->setEntityByID(_sub_id, _triple.subject);
			vertices.push_back(_sub_id);
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
	if(txn != nullptr)
	{
		unordered_set<TYPE_ENTITY_LITERAL_ID> sids, oids;
		unordered_set<TYPE_PREDICATE_ID> pids;
		for(auto tuple: id_tuples)
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
		if(ret == false){
			return -1;
		}
	}
	//po inserts
	//sub_batch_update(id_tuples, valid_num, update_num_s, UPDATE_TYPE::SUBJECT_INSERT, txn);
	thread sub_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_s), UPDATE_TYPE::SUBJECT_INSERT, txn );

	//so inserts
	//sub_batch_update(id_tuples, valid_num, update_num_p, UPDATE_TYPE::PREDICATE_INSERT, txn);
	thread pre_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_p), UPDATE_TYPE::PREDICATE_INSERT, txn );

	//ps inserts
	//sub_batch_update(id_tuples, valid_num, update_num_o, UPDATE_TYPE::OBJECT_INSERT, txn);
	thread obj_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_o), UPDATE_TYPE::OBJECT_INSERT, txn);


	sub_t.join();
	pre_t.join();
	obj_t.join();

	unsigned update_num_triple=update_num_s;
	if(update_num_triple<update_num_p)
	update_num_triple=update_num_p;
	if(update_num_triple<update_num_o)
	update_num_triple=update_num_o;

	cout <<"update_num_triple:"<<update_num_triple<<",update_num_s:"<< update_num_s << ",update_num_p:" << update_num_p << ",update_num_o:" << update_num_o << endl;
	// assert(update_num_o == update_num_p);
	// assert(update_num_s == update_num_o);

    this->triples_num=this->triples_num+update_num_triple;
	this->sub_num=this->sub_num+update_num_subject;
	this->stringindex->SetTrie(kvstore->getTrie());
	//update string index
	this->stringindex->change(vertices, *this->kvstore, true);
	this->stringindex->change(predicates, *this->kvstore, false);

	return update_num_s;
}

//WARNING: TRANSACTIONAL batch remove is not completed yet!
unsigned
Database::batch_remove(const TripleWithObjType* _triples, TYPE_TRIPLE_NUM _triple_num, bool _is_restore, shared_ptr<Transaction> txn)
{
	if(_triple_num == 0) return 0;
	TYPE_TRIPLE_NUM valid_num = 0;
	unsigned update_num_s = 0;
	unsigned update_num_p = 0;
	unsigned update_num_o = 0;
    unsigned update_num_subject=0;
	unsigned update_num_triple=0;
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	set<unsigned> sub_ids, pre_ids, obj_ids;
	if (!_is_restore) {
		write_update_log(_triples, _triple_num, 0, txn);
	}

	vector<ID_TUPLE> id_tuples(_triple_num);
	for (int i = 0; i < _triple_num; ++i)
	{
		TripleWithObjType _triple = _triples[i];
		TYPE_ENTITY_LITERAL_ID _sub_id = (this->kvstore)->getIDByEntity(_triple.subject);
		if(_sub_id == INVALID_ENTITY_LITERAL_ID) continue;
		TYPE_PREDICATE_ID _pre_id = (this->kvstore)->getIDByPredicate(_triple.predicate);
		if(_pre_id == INVALID_PREDICATE_ID) continue;
		TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
		if(_triple.isObjEntity())
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

	//po inserts
	//sub_batch_update(id_tuples, valid_num, update_num_s, UPDATE_TYPE::SUBJECT_INSERT, txn);
	thread sub_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_s), UPDATE_TYPE::SUBJECT_REMOVE, txn );

	//so inserts
	//sub_batch_update(id_tuples, valid_num, update_num_p, UPDATE_TYPE::PREDICATE_INSERT, txn);
	thread pre_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_p), UPDATE_TYPE::PREDICATE_REMOVE, txn );

	//ps inserts
	//sub_batch_update(id_tuples, valid_num, update_num_o, UPDATE_TYPE::OBJECT_INSERT, txn);
	thread obj_t = thread(&Database::sub_batch_update, this, id_tuples, valid_num, ref(update_num_o), UPDATE_TYPE::OBJECT_REMOVE, txn );
	//cout << update_num_s << " " << update_num_p << " " << update_num_o << endl;

	sub_t.join();
	pre_t.join();
	obj_t.join();
	//cout << update_num_s << " " << update_num_p << " " << update_num_o << endl;
	// assert(update_num_o == update_num_p);
	// assert(update_num_s == update_num_o);

	if(update_num_triple<update_num_p)
	update_num_triple=update_num_p;
	if(update_num_triple<update_num_o)
	update_num_triple=update_num_o;

	cout <<"update_num_triple:"<<update_num_triple<<",update_num_s:"<< update_num_s << ",update_num_p:" << update_num_p << ",update_num_o:" << update_num_o << endl;
	if(txn == nullptr)
	{
		for(auto _sub_id: sub_ids)
		{
			int sub_degree = (this->kvstore)->getEntityDegree(_sub_id);
			if (sub_degree == 0)
			{
				string subject = this->kvstore->getEntityByID(_sub_id);
				if(subject == "") continue;
				this->kvstore->subEntityByID(_sub_id);
				this->kvstore->subIDByEntity(subject);
				this->freeEntityID(_sub_id);
				this->sub_num--;
				vertices.push_back(_sub_id);
			}
		}

		for(auto _obj_id: obj_ids)
		{
			bool is_obj_entity = Util::is_entity_ele(_obj_id);
			int obj_degree;
			if (is_obj_entity)
			{
				obj_degree = this->kvstore->getEntityDegree(_obj_id);
				if (obj_degree == 0)
				{
					string object = this->kvstore->getEntityByID(_obj_id);
					if(object == "") continue;
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
					string object = this->kvstore->getEntityByID(_obj_id);
					if(object == "") continue;
					this->kvstore->subLiteralByID(_obj_id);
					this->kvstore->subIDByLiteral(object);
					this->freeLiteralID(_obj_id);
					vertices.push_back(_obj_id);
				}
			}
		}
		for(auto _pre_id: pre_ids)
		{
			int pre_degree = this->kvstore->getPredicateDegree(_pre_id);
			if (pre_degree == 0)
			{
				string predicate = this->kvstore->getPredicateByID(_pre_id);
				if(predicate == "") continue;
				this->kvstore->subPredicateByID(_pre_id);
				this->kvstore->subIDByPredicate(predicate);
				this->freePredicateID(_pre_id);
				predicates.push_back(_pre_id);
			}
		}
        this->triples_num=this->triples_num-update_num_triple;
		this->stringindex->SetTrie(kvstore->getTrie());
		//update string index
		this->stringindex->disable(vertices, true);
		this->stringindex->disable(predicates, false);
	}
	return update_num_s;
}

void
Database::sub_batch_update(vector<ID_TUPLE> id_tuples, TYPE_TRIPLE_NUM _triple_num, unsigned &update_num, UPDATE_TYPE type, shared_ptr<Transaction> txn)
{
	vector<unsigned> data;
	if(type == UPDATE_TYPE::SUBJECT_INSERT || type == UPDATE_TYPE::SUBJECT_REMOVE)
	{
		TYPE_ENTITY_LITERAL_ID sub_id = id_tuples[0].subid;
		for(int i = 0; i < _triple_num; i++)
		{
			if(id_tuples[i].subid == sub_id){
				data.push_back(id_tuples[i].preid);
				data.push_back(id_tuples[i].objid);
			}
			else
			{
				if(txn == nullptr){
					if(type == UPDATE_TYPE::SUBJECT_INSERT)
						update_num += this->kvstore->updateInsert_s2values(sub_id, data);
					else if(type == UPDATE_TYPE::SUBJECT_REMOVE)
						update_num += this->kvstore->updateRemove_s2values(sub_id, data);
				}
				else{

				}
				data.clear();
				sub_id = id_tuples[i].subid;
				data.push_back(id_tuples[i].preid);
				data.push_back(id_tuples[i].objid);
			}
		}
		if(txn == nullptr){
			if(type == UPDATE_TYPE::SUBJECT_INSERT)
				update_num += this->kvstore->updateInsert_s2values(sub_id, data);
			else if(type == UPDATE_TYPE::SUBJECT_REMOVE)
				update_num += this->kvstore->updateRemove_s2values(sub_id, data);
		}
		else{

		}
	}
	else if(type == UPDATE_TYPE::PREDICATE_INSERT || type == UPDATE_TYPE::PREDICATE_REMOVE)
	{
		sort(id_tuples.begin(), id_tuples.end(), Util::pso_cmp_idtuple);
		TYPE_PREDICATE_ID pre_id = id_tuples[0].preid;
		for(int i = 0; i < _triple_num; i++)
		{
			if(id_tuples[i].preid == pre_id){
				data.push_back(id_tuples[i].subid);
				data.push_back(id_tuples[i].objid);
			}
			else
			{
				if(txn == nullptr){
					if(type == UPDATE_TYPE::PREDICATE_INSERT)
						update_num += this->kvstore->updateInsert_p2values(pre_id, data);
					else if(type == UPDATE_TYPE::PREDICATE_REMOVE)
						update_num += this->kvstore->updateRemove_p2values(pre_id, data);
				}
				else{

				}
				data.clear();
				pre_id = id_tuples[i].preid;
				data.push_back(id_tuples[i].subid);
				data.push_back(id_tuples[i].objid);
			}
		}
		if(txn == nullptr){
			if(type == UPDATE_TYPE::PREDICATE_INSERT)
				update_num += this->kvstore->updateInsert_p2values(pre_id, data);
			else if(type == UPDATE_TYPE::PREDICATE_REMOVE)
				update_num += this->kvstore->updateRemove_p2values(pre_id, data);
		}
		else{

		}
	}
	else if(type == UPDATE_TYPE::OBJECT_INSERT || type == UPDATE_TYPE::OBJECT_REMOVE)
	{
		sort(id_tuples.begin(), id_tuples.end(), Util::ops_cmp_idtuple);
		TYPE_ENTITY_LITERAL_ID obj_id = id_tuples[0].objid;
		for(int i = 0; i < _triple_num; i++)
		{
			if(id_tuples[i].objid == obj_id){
				data.push_back(id_tuples[i].preid);
				data.push_back(id_tuples[i].subid);
			}
			else
			{
				if(txn == nullptr){
					if(type == UPDATE_TYPE::OBJECT_INSERT)
						update_num += this->kvstore->updateInsert_o2values(obj_id, data);
					else if(type == UPDATE_TYPE::OBJECT_REMOVE)
						update_num += this->kvstore->updateRemove_o2values(obj_id, data);
				}
				else{

				}
				data.clear();
				obj_id = id_tuples[i].objid;
				data.push_back(id_tuples[i].preid);
				data.push_back(id_tuples[i].subid);
			}
		}
		if(txn == nullptr){
			if(type == UPDATE_TYPE::OBJECT_INSERT)
				update_num += this->kvstore->updateInsert_o2values(obj_id, data);
			else if(type == UPDATE_TYPE::OBJECT_REMOVE)
				update_num += this->kvstore->updateRemove_o2values(obj_id, data);
		}
		else{

		}
	}
}

bool
Database::backup() 
{
	if (!Util::dir_exist(Util::backup_path)) 
	{
		Util::create_dir(Util::backup_path);
	}
	string backup_path = Util::backup_path + this->store_path;

	cout << "Beginning backup." << endl;

	string sys_cmd;
	if (Util::dir_exist(backup_path)) 
	{
		sys_cmd = "rm -rf " + backup_path;
		system(sys_cmd.c_str());
	}
	sys_cmd = "cp -r " + this->store_path + ' ' + backup_path;
	system(sys_cmd.c_str());
	sys_cmd = "rm " + backup_path + '/' + this->update_log;
	system(sys_cmd.c_str());

	//this->vstree->saveTree();
	this->kvstore->flush();

	this->clear_update_log();
	string update_log_path = this->store_path + '/' + this->update_log_since_backup;
	sys_cmd = "rm " + update_log_path;
	system(sys_cmd.c_str());
	Util::create_file(update_log_path);

	cout << "Backup completed!" << endl;
	return true;
}

bool 
Database::restore() 
{
	cout << "Begining restore." << endl;
	string sys_cmd;

	multiset<string> insertions;
	multiset<string> removals;

	int num_update = 0;
	if (!this->load()) 
	{
		this->clear();

		string backup_path = Util::backup_path + this->store_path;
		if (!Util::dir_exist(Util::backup_path)) 
		{
			cerr << "Failed to restore!" << endl;
			return false;
		}

		num_update += Database::read_update_log(this->store_path + '/' + this->update_log_since_backup, insertions, removals);

		cout << "Failed to restore from original db file, trying to restore from backup file." << endl;
		cout << "Your old db file will be stored at " << this->store_path << ".bad" << endl;

		sys_cmd = "rm -rf " + this->store_path + ".bad";
		system(sys_cmd.c_str());
		sys_cmd = "cp -r " + this->store_path + ' ' + this->store_path + ".bad";
		system(sys_cmd.c_str());
		sys_cmd = "rm -rf " + this->store_path;
		system(sys_cmd.c_str());
		sys_cmd = "cp -r " + backup_path + ' ' + this->store_path;
		system(sys_cmd.c_str());
		Util::create_file(this->store_path + '/' + this->update_log);

		if (!this->load()) 
		{
			this->clear();
			cerr << "Failed to restore from backup file." << endl;
			return false;
		}

		num_update += Database::read_update_log(this->store_path + '/' + this->update_log_since_backup, insertions, removals);
	}
	else 
	{
		num_update += Database::read_update_log(this->store_path + '/' + this->update_log, insertions, removals);
	}

	cout << "Restoring " << num_update << " updates." << endl;

	if (!this->restore_update(insertions, removals)) 
	{
		cerr << "Failed to restore updates" << endl;
		return false;
	}

	cout << "Restore completed." << endl;

	return true;
}

int 
Database::read_update_log(const string _path, multiset<string>& _i, multiset<string>& _r) 
{
	ifstream in;
#ifdef DEBUG
	cout<<_path<<endl;
#endif
	in.open(_path.c_str(), ios::in);
	if (!in) {
		cerr << "Failed to read update log." << endl;
		return 0;
	}
	
	int ret = 0;
	int buffer_size = 1024 + 2;
	char buffer[buffer_size];
	in.getline(buffer, buffer_size);
	while (!in.eof() && buffer[0]) {
		string triple;
		switch (buffer[0]) {
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
			cerr << "Bad line in update log!" << endl;
		}
		in.getline(buffer, buffer_size);
	}

	return ret;
}

bool 
Database::restore_update(multiset<string>& _i, multiset<string>& _r) 
{
	multiset<string>::iterator pos;
	multiset<string>::iterator it_to_erase = _r.end();
	for (multiset<string>::iterator it = _r.begin(); it != _r.end(); it++) 
	{
		//NOTICE: check the intersect of insert_set and remove_set
		if (it_to_erase != _r.end()) {
			_r.erase(it_to_erase);
		}
		pos = _i.find(*it);
		if (pos != _i.end()) {
			_i.erase(pos);
			it_to_erase = it;
		}
		else {
			it_to_erase = _r.end();
		}
	}
	if (it_to_erase != _r.end()) {
		_r.erase(it_to_erase);
	}

	string tmp_path = this->store_path + "/.update_tmp";

	ofstream out_i;
	out_i.open(tmp_path.c_str(), ios::out);
	if (!out_i) {
		cerr << "Failed to open temp file, restore failed!" << endl;
		return false;
	}
	for (multiset<string>::iterator it = _i.begin(); it != _i.end(); it++) {
		out_i << *it << endl;
	}
	out_i.close();
	if (!this->insert(tmp_path, true)) 
	//if (!this->remove(tmp_path, true)) 
	{
		return false;
	}

	ofstream out_r;
	out_r.open(tmp_path.c_str(), ios::out);
	if (!out_r) {
		cerr << "Failed to open temp file!" << endl;
		return false;
	}
	for (multiset<string>::iterator it = _r.begin(); it != _r.end(); it++) {
		out_r << *it << endl;
	}
	out_r.close();
	if (!this->remove(tmp_path, true)) 
	//if (!this->insert(tmp_path, true)) 
	{
		return false;
	}

	string cmd = "rm " + tmp_path;
	system(cmd.c_str());
	return true;
}

void 
Database::clear_update_log() 
{
	string path = this->getStorePath() + '/' + this->update_log;
	ofstream out;
	out.open(path.c_str(), ios::out);
	out.close();
}

bool
Database::write_update_log(const TripleWithObjType* _triples, TYPE_TRIPLE_NUM _triple_num, int type, shared_ptr<Transaction> txn)
{
	log_lock.lock();
	string path = this->getStorePath() + '/' + this->update_log;
	string path_all = this->getStorePath() + '/' + this->update_log_since_backup;
	ofstream out;
	ofstream out_all;
	out.open(path.c_str(), ios::out | ios::app);
	out_all.open(path_all.c_str(), ios::out | ios::app);
	if (!out || !out_all) {
		cerr << "Failed to open update log. Insertion aborted." << endl;
		log_lock.unlock();
		return false;
	}

	for (int i = 0; i < _triple_num; i++) {
		// if (type == 1 && exist_triple(_triples[i], txn)) {
		//	continue;
		// }
		// else if(type == 0 && !exist_triple(_triples[i], txn))
		// {
		//	continue;
		// }
		stringstream ss;
		if(type == 1){
			ss << "I\t" << Util::node2string(_triples[i].getSubject().c_str()) << '\t';
		}
		else{
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

bool
Database::objIDIsEntityID(TYPE_ENTITY_LITERAL_ID _id)
{
	return _id < Util::LITERAL_FIRST_ID;
}

bool
Database::getFinalResult(SPARQLquery& _sparql_q, ResultSet& _result_set)
{
#ifdef DEBUG_PRECISE
	printf("getFinalResult:begins\n");
#endif
	// this is only selected var num
	int _var_num = _sparql_q.getQueryVarNum();
	_result_set.setVar(_sparql_q.getQueryVar());
	vector<BasicQuery*>& query_vec = _sparql_q.getBasicQueryVec();

	//sum the answer number
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
	_result_set.answer = new string*[_ans_num];
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
	//map int ans into string ans
	//union every basic result into total result
	for (unsigned i = 0; i < query_vec.size(); i++)
	{
		vector<unsigned*>& tmp_vec = query_vec[i]->getResultList();
		//ensure the spo order is right, but the triple order is still reversed
		//for every result group in resultlist
		//for(vector<int*>::reverse_iterator itr = tmp_vec.rbegin(); itr != tmp_vec.rend(); ++itr)
		for (vector<unsigned*>::iterator itr = tmp_vec.begin(); itr != tmp_vec.end(); ++itr)
		{
			//to ensure the order so do reversely in two nested loops
#ifndef STREAM_ON
			_result_set.answer[tmp_ans_count] = new string[_var_num];
#endif
#ifdef DEBUG_PRECISE
			printf("getFinalResult:before map loop\n");
#endif
			//NOTICE: in new join method only selec_var_num columns,
			//but before in shenxuchuan's join method, not like this.
			//though there is all graph_var_num columns in result_list,
			//we only consider the former selected vars
			//map every ans_id into ans_str
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

//void
//Database::printIDlist(int _i, int* _list, int _len, string _log)
//{
	//stringstream _ss;
	//_ss << "[" << _i << "] ";
	//for (int i = 0; i < _len; i++) {
		//_ss << _list[i] << "\t";
	//}
	//Util::logging("==" + _log + ":");
	//Util::logging(_ss.str());
//}

//void
//Database::printPairList(int _i, int* _list, int _len, string _log)
//{
	//stringstream _ss;
	//_ss << "[" << _i << "] ";
	//for (int i = 0; i < _len; i += 2) {
		//_ss << "[" << _list[i] << "," << _list[i + 1] << "]\t";
	//}
	//Util::logging("==" + _log + ":");
	//Util::logging(_ss.str());
//}

//void
//Database::test()
//{
	//int subNum = 9, preNum = 20, objNum = 90;

	//int* _id_list = NULL;
	//int _list_len = 0;
	//{   [> x2ylist <]
		//for (int i = 0; i < subNum; i++)
		//{

			//(this->kvstore)->getobjIDlistBysubID(i, _id_list, _list_len);
			//if (_list_len != 0)
			//{
				//stringstream _ss;
				//this->printIDlist(i, _id_list, _list_len, "s2olist[" + _ss.str() + "]");
				//delete[] _id_list;
			//}

			//[> o2slist <]
			//(this->kvstore)->getsubIDlistByobjID(i, _id_list, _list_len);
			//if (_list_len != 0)
			//{
				//stringstream _ss;
				//this->printIDlist(i, _id_list, _list_len, "o(sub)2slist[" + _ss.str() + "]");
				//delete[] _id_list;
			//}
		//}

		//for (int i = 0; i < objNum; i++)
		//{
			//int _i = Util::LITERAL_FIRST_ID + i;
			//(this->kvstore)->getsubIDlistByobjID(_i, _id_list, _list_len);
			//if (_list_len != 0)
			//{
				//stringstream _ss;
				//this->printIDlist(_i, _id_list, _list_len, "o(literal)2slist[" + _ss.str() + "]");
				//delete[] _id_list;
			//}
		//}
	//}
	//{   [> xy2zlist <]
		//for (int i = 0; i < subNum; i++)
		//{
			//for (int j = 0; j < preNum; j++)
			//{
				//(this->kvstore)->getobjIDlistBysubIDpreID(i, j, _id_list,
					//_list_len);
				//if (_list_len != 0)
				//{
					//stringstream _ss;
					//_ss << "preid:" << j;
					//this->printIDlist(i, _id_list, _list_len, "sp2olist[" + _ss.str() + "]");
					//delete[] _id_list;
				//}

				//(this->kvstore)->getsubIDlistByobjIDpreID(i, j, _id_list,
					//_list_len);
				//if (_list_len != 0)
				//{
					//stringstream _ss;
					//_ss << "preid:" << j;
					//this->printIDlist(i, _id_list, _list_len, "o(sub)p2slist[" + _ss.str() + "]");
					//delete[] _id_list;
				//}
			//}
		//}

		//for (int i = 0; i < objNum; i++)
		//{
			//int _i = Util::LITERAL_FIRST_ID + i;
			//for (int j = 0; j < preNum; j++)
			//{
				//(this->kvstore)->getsubIDlistByobjIDpreID(_i, j, _id_list,
					//_list_len);
				//if (_list_len != 0)
				//{
					//stringstream _ss;
					//_ss << "preid:" << j;
					//this->printIDlist(_i, _id_list, _list_len,
						//"*o(literal)p2slist[" + _ss.str() + "]");
					//delete[] _id_list;
				//}
			//}
		//}
	//}
	//{   [> x2yzlist <]
		//for (int i = 0; i < subNum; i++)
		//{
			//(this->kvstore)->getpreIDobjIDlistBysubID(i, _id_list, _list_len);
			//if (_list_len != 0)
			//{
				//this->printPairList(i, _id_list, _list_len, "s2polist");
				//delete[] _id_list;
				//_list_len = 0;
			//}
		//}

		//for (int i = 0; i < subNum; i++)
		//{
			//(this->kvstore)->getpreIDsubIDlistByobjID(i, _id_list, _list_len);
			//if (_list_len != 0)
			//{
				//this->printPairList(i, _id_list, _list_len, "o(sub)2pslist");
				//delete[] _id_list;
			//}
		//}

		//for (int i = 0; i < objNum; i++)
		//{
			//int _i = Util::LITERAL_FIRST_ID + i;
			//(this->kvstore)->getpreIDsubIDlistByobjID(_i, _id_list, _list_len);
			//if (_list_len != 0)
			//{
				//this->printPairList(_i, _id_list, _list_len,
					//"o(literal)2pslist");
				//delete[] _id_list;
			//}
		//}
	//}
//}

//void
//Database::test_build_sig()
//{
	//BasicQuery* _bq = new BasicQuery("");
	/*
	* <!!!>	y:created	<!!!_(album)>.
	*  <!!!>	y:created	<Louden_Up_Now>.
	*  <!!!_(album)>	y:hasSuccessor	<Louden_Up_Now>
	* <!!!_(album)>	rdf:type	<wordnet_album_106591815>
	*
	* id of <!!!> is 0
	* id of <!!!_(album)> is 2
	*
	*
	* ?x1	y:created	?x2.
	*  ?x1	y:created	<Louden_Up_Now>.
	*  ?x2	y:hasSuccessor	<Louden_Up_Now>.
	* ?x2	rdf:type	<wordnet_album_106591815>
	*/
	//{
		//Triple _triple("?x1", "y:created", "?x2");
		//_bq->addTriple(_triple);
	//}
	//{
		//Triple _triple("?x1", "y:created", "<Louden_Up_Now>");
		//_bq->addTriple(_triple);
	//}
	//{
		//Triple _triple("?x2", "y:hasSuccessor", "<Louden_Up_Now>");
		//_bq->addTriple(_triple);
	//}
	//{
		//Triple _triple("?x2", "rdf:type", "<wordnet_album_106591815>");
		//_bq->addTriple(_triple);
	//}
	//vector<string> _v;
	//_v.push_back("?x1");
	//_v.push_back("?x2");

	//_bq->encodeBasicQuery(this->kvstore, _v);
	//Util::logging(_bq->to_str());
	//SPARQLquery _q;
	//_q.addBasicQuery(_bq);

	//(this->vstree)->retrieve(_q);

	//Util::logging("\n\n");
	//Util::logging("candidate:\n\n" + _q.candidate_str());
//}

//void
//Database::test_join()
//{
//BasicQuery* _bq = new BasicQuery("");
//
//* <!!!>	y:created	<!!!_(album)>.
//*  <!!!>	y:created	<Louden_Up_Now>.
//*  <!!!_(album)>	y:hasSuccessor	<Louden_Up_Now>
//* <!!!_(album)>	rdf:type	<wordnet_album_106591815>
//*
//* id of <!!!> is 0
//* id of <!!!_(album)> is 2
//*
//*
//* ?x1	y:created	?x2.
//*  ?x1	y:created	<Louden_Up_Now>.
//*  ?x2	y:hasSuccessor	<Louden_Up_Now>.
//* ?x2	rdf:type	<wordnet_album_106591815>
//
//{
////Triple _triple("?x1", "y:created", "?x2");
//_bq->addTriple(_triple);
//}
//{
//Triple _triple("?x1", "y:created", "<Louden_Up_Now>");
//_bq->addTriple(_triple);
//}
//{
//Triple _triple("?x2", "y:hasSuccessor", "<Louden_Up_Now>");
//_bq->addTriple(_triple);
//}
//{
//Triple _triple("?x2", "rdf:type", "<wordnet_album_106591815>");
//_bq->addTriple(_triple);
//}
//vector<string> _v;
//_v.push_back("?x1");
//_v.push_back("?x2");
//_bq->encodeBasicQuery(this->kvstore, _v);
//Util::logging(_bq->to_str());
//SPARQLquery _q;
//_q.addBasicQuery(_bq);
//(this->vstree)->retrieve(_q);
//Util::logging("\n\n");
//Util::logging("candidate:\n\n"+_q.candidate_str());
//_q.print(cout);
//this->join(_q);
//ResultSet _rs;
//this->getFinalResult(_q, _rs);
//cout << _rs.to_str() << endl;
//}


//garbage clean 
void
Database::VersionClean(vector<unsigned> &sub_ids ,vector<unsigned>& obj_ids, vector<unsigned>& obj_literal_ids, vector<unsigned> &pre_ids)
{
	//vector<unsigned> sub_ids , obj_ids, obj_literal_ids, pre_ids;
	(this->kvstore)->IVArrayVacuum(sub_ids, obj_ids, obj_literal_ids, pre_ids);
	vector<TYPE_ENTITY_LITERAL_ID> vertices, predicates;
	//update
	int sub_degree, obj_degree, pre_degree;
	for(auto &_sub_id: sub_ids)
	{
		sub_degree = (this->kvstore)->getEntityDegree(_sub_id);
		//if subject become an isolated point, remove its corresponding entry
		if (sub_degree == 0)
		{
			//cout<<"to remove entry for sub"<<endl;
			//cout<<_sub_id << " "<<this->kvstore->getEntityByID(_sub_id)<<endl;
			string sub_str = this->kvstore->getEntityByID(_sub_id);
			//cerr << "sub_str" << sub_str << endl;
			if(sub_str == "") continue;
			this->kvstore->subIDByEntity(sub_str);
			this->kvstore->subEntityByID(_sub_id);			
			this->freeEntityID(_sub_id);
			this->sub_num--;
			//update the string buffer
			//if (_sub_id < this->entity_buffer_size)
			//{
				//this->entity_buffer->del(_sub_id);
			//}
			vertices.push_back(_sub_id);
		}
		//cout<<"subject dealed"<<endl;
	}
	
	for(auto &_obj_id: obj_ids)
	{
		obj_degree = this->kvstore->getEntityDegree(_obj_id);
		if (obj_degree == 0)
		{
			//cout<<"to remove entry for obj"<<endl;
			//cout<<_obj_id << " "<<this->kvstore->getEntityByID(_obj_id)<<endl;
			string obj_str = this->kvstore->getEntityByID(_obj_id);
			//cerr << "obj_str" << obj_str << endl;
			if(obj_str == "") continue;
			this->kvstore->subIDByEntity(obj_str);
			this->kvstore->subEntityByID(_obj_id);			
			this->freeEntityID(_obj_id);
			//update the string buffer
			//if (_obj_id < this->entity_buffer_size)
			//{
				//this->entity_buffer->del(_obj_id);
			//}
			vertices.push_back(_obj_id);
		}
	}
	
	for(auto &_obj_id: obj_literal_ids)
	{
		//cerr << _obj_id << endl;
		obj_degree = this->kvstore->getLiteralDegree(_obj_id);
		if (obj_degree == 0)
		{
			string obj_str = this->kvstore->getLiteralByID(_obj_id);
			cerr << "obj_str" <<  obj_str << "     _obj_id" << _obj_id << endl;
			if(obj_str == "") continue;

			this->kvstore->subLiteralByID(_obj_id);
			this->kvstore->subIDByLiteral(obj_str);
			//cout<<"check after subLiteralByID: "<<_obj_id<<" "<<this->kvstore->getLiteralByID(_obj_id)<<endl;
			this->freeLiteralID(_obj_id);
			//update the string buffer
			//TYPE_ENTITY_LITERAL_ID tid = _obj_id - Util::LITERAL_FIRST_ID;
			//if (tid < this->literal_buffer_size)
			//{
			//this->literal_buffer->del(tid);
			//}
			vertices.push_back(_obj_id);
		}
	}
		//cout<<"object dealed"<<endl;
	for(auto &_pre_id: pre_ids)
	{
		pre_degree = this->kvstore->getPredicateDegree(_pre_id);
		if (pre_degree == 0)
		{
			string pre_str = this->kvstore->getPredicateByID(_pre_id);
			//cerr << "pre_str" << pre_str << endl;
			this->kvstore->subIDByPredicate(pre_str);
			this->kvstore->subPredicateByID(_pre_id);
			this->freePredicateID(_pre_id);
			predicates.push_back(_pre_id);
		}
		//cout<<"predicate dealed"<<endl;
	}
	
	//cerr << "vertices.size()" << vertices.size() << endl;
	//cerr << "predicates.size()" << predicates.size() << endl;
	this->stringindex->SetTrie(kvstore->getTrie());
	//update string index
	this->stringindex->disable(vertices, true);
	this->stringindex->disable(predicates, false);
	
}

void 
Database::TransactionRollback(shared_ptr<Transaction> txn)
{
	if((this->kvstore)->TransactionInvalid(txn) == false)
	{
		cerr << "WARNING: transaction rollback exception! " << endl;
		cerr << "Please REBOOT service!" << endl;
	}
}

void 
Database::TransactionCommit(shared_ptr<Transaction> txn)
{
	//cout << "transaction_commit ........" << endl;
	if((this->kvstore)->ReleaseAllLocks(txn) == false)
	{
		cerr << "WARNING: not all latches get unlatched! " << endl;
		cerr << "Please REBOOT service!" << endl;
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

void Database::load_statistics() {
    this->statistics = new Statistics(this->getStorePath(), this->getlimitID_predicate());
    this->statistics->load_Statistics();
}

bool Database::saveStatisticsInfoFile()
{
     ofstream file;
	 string filepath=this->getStorePath() + "/" + this->statistics_info_file;
	 if(Util::file_exist(filepath)==false)
	 {
		cout<<"the statistics file is not exist"<<endl;
		Util::create_file(filepath);
	 }
	 file.open(filepath);
	 int i=0;
	 for(auto& kv:this->umap){
		//cout<<"first:"<<kv.first<<",second:"<<kv.second<<endl;
        file<<kv.first<<"@@"<<kv.second<<endl;
		i++;
    }
	file.flush();
	file.close();
	cout<<"save the statistics file successfully! total "<<i<<" records have been saved!" <<endl;
	return true;
}

bool Database::loadStatisticsInfoFile()
{
 
   string filepath=this->getStorePath() + "/" + this->statistics_info_file;
   if(Util::file_exist(filepath)==false)
   {
		cout<<"the statistics file is not exist."<<endl;
		cout<<"load the statistics file failure!"<<endl;
		return false;
   }
   cout<<"load the file: "<<filepath<<endl;
   ifstream file(filepath,ios::in);
   string line;
   vector<string> lines;

   stringstream str;
   unsigned long long value;
   if(file)
   {
	   this->umap.clear();
   while(getline(file,line))
   {
	 
	  Util::split(line,"@@",lines);
	  if(lines.size()==2)
	  {
		str<<lines[1];
		str>>value;
		this->umap.insert(pair<string,unsigned long long>(lines[0],value));
	  }
    }
	 return true;
   }
   else
   return false;
   
  
	
}