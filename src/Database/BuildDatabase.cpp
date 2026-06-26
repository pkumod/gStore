#include "Database.h"

bool Database::build(const std::vector<std::string>&_rdf_files)
{
	SLOG_CORE("---------Begin to Build Database `" << name << "`---------");
	std::vector<std::string> rets;
	for (const auto& m : _rdf_files)
		rets.push_back(Util::getExactPath(m.c_str()));
		
	InitEmptyDB();
	string error_log = this->store_path + "/parse_error.log";
	FILE *fp = fopen(error_log.c_str(), "a");
	for (auto& ret : rets)
	{
		string log_msg = "Info " + gs::TimeUtil::now(NORM_DATETIME_PATTERN) + " build parser info, file path " + ret + "\n";
		fputs(log_msg.c_str(), fp);
	}
	fclose(fp);
	SLOG_CORE("Begin encode RDF from : " << rets.size() << " ...");

	//    this->encodeRDF(ret);
	if (!this->encodeRDF_new(rets, error_log)) //<-- this->kvstore->id2* trees are closed
	{
		return false;
	}
	SLOG_CORE("finish encode.");

	this->kvstore.reset();

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
	addTripleUpdateNum(this->triples_num);
	return true;
}

bool Database::encodeRDF_new(const std::vector<std::string> &_rdf_files, const string _error_log)
{
	indicators::ProgressBar bar{
			indicators::option::BarWidth{50},
			indicators::option::Start{"["},
			indicators::option::Fill{"="},
			indicators::option::Lead{">"},
			indicators::option::Remainder{" "},
			indicators::option::End{"]"},
			indicators::option::PostfixText{"Parsing triples 0/5"},
			indicators::option::ForegroundColor{indicators::Color::green},
			indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

	int64_t t1 = gs::TimeUtil::timestamp();
	SLOG_CORE("Begin to parse triples ......");
	setProgress(Progress_RDFParse);
	std::map<std::string, std::set<std::string>> id_tuples;
	if (!this->sub2id_pre2id_obj2id_RDFintoSignature(_rdf_files, _error_log, bar, id_tuples))
	{
		return false;
	}
	id_tuples.clear();
	int64_t t2 = gs::TimeUtil::timestamp();
	SLOG_CORE("Finish parsing, used " + to_string(t2 - t1) + "ms.");
	
	bar.set_option(indicators::option::PostfixText{"building stringIndex 1/5"});
	bar.set_progress(60);
	SLOG_CORE("Begin to save StringIndex ......");
	this->stringindex->setNum(StringIndexFile::Entity, this->entity_num);
	this->stringindex->setNum(StringIndexFile::Literal, this->literal_num);
	this->stringindex->setNum(StringIndexFile::Predicate, this->pre_num);
	setProgress(Progress_SavingStringIndex);
	this->stringindex->save(*this->kvstore);

	t1 = gs::TimeUtil::timestamp();
	SLOG_CORE("Saving StringIndex, used " + to_string(t1 - t2) + "ms.");
	bar.set_option(indicators::option::PostfixText{"building id2string and string2id 2/5"});
	bar.set_progress(61);
	buildCloseToSaveMemory();
	
	t2 = gs::TimeUtil::timestamp();
	SLOG_CORE("Finish saving id2string and string2id, used " + to_string(t2 - t1) + "ms.");
	bar.set_option(indicators::option::PostfixText{"building spo2values 3/5"});
	bar.set_progress(80);
	// after closing the 6 trees, read the id tuples again, and remove the file     given num, a dimension,return a pointer
	// NOTICE: the file can also be used for debugging, and a program can start just from the id tuples file
	//(if copy the 6 id2string trees, no need to parse each time)
	std::shared_ptr<ID_TUPLE[]> _p_id_tuples(new ID_TUPLE[this->triples_num], std::default_delete<ID_TUPLE[]>());
	this->readIDTuples(_p_id_tuples);

	// NOTICE: we can also build the signature when we are reading triples, and
	// update to the corresponding position in the signature file
	// However, this may be costly due to frequent read/write

	t1 = gs::TimeUtil::timestamp();
	SLOG_CORE("id tuples read, used " + to_string(t1 - t2) + "ms.");

	// TODO: how to set the buffer of trees is a big question, fully utilize the availiable memory
	SLOG_CORE("Begin to build spo2values ......");

	// remove duplicates from the id tables
	#ifndef PARALLEL_SORT
	std::sort(_p_id_tuples.get(), _p_id_tuples.get() + this->triples_num, Util::spo_cmp_idtuple);
	#else
	omp_set_num_threads(thread_num);
	__gnu_parallel::sort(_p_id_tuples.get(), _p_id_tuples.get() + this->triples_num, Util::ops_cmp_idtuple);
	#endif
	t2 = gs::TimeUtil::timestamp();
	SLOG_CORE("Finish sorting id tuples, used " + to_string(t2 - t1) + "ms.");
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
	t1 = gs::TimeUtil::timestamp();
	SLOG_CORE("Finish removing duplicate tuples, used " + to_string(t1 - t2) + "ms.");
	if (Util::getConfigureValue("build_multi_thread") == "off")
	{
		t2 = gs::TimeUtil::timestamp();
		build_s2xx(_p_id_tuples);
		build_o2xx(_p_id_tuples);
		build_p2xx(_p_id_tuples);
		t1 = gs::TimeUtil::timestamp();
		SLOG_CORE("Finish building spo2values, used " + to_string(t1 - t2) + "ms.");
		bar.set_option(indicators::option::PostfixText{"Saving database info 4/5"});
		bar.set_progress(99);
		// WARN:we must free the memory for id_tuples array
		_p_id_tuples.reset();
	}
	else
	{
		// copy the id tuples to the other two arrays
		ID_TUPLE* tmp_array_1 = new ID_TUPLE[this->triples_num];
		ID_TUPLE* tmp_array_2 = new ID_TUPLE[this->triples_num];
		std::copy(&_p_id_tuples[0], &_p_id_tuples[this->triples_num], tmp_array_1);
		std::copy(&_p_id_tuples[0], &_p_id_tuples[this->triples_num], tmp_array_2);
		std::shared_ptr<ID_TUPLE[]> _p_id_tuples_1(tmp_array_1, std::default_delete<ID_TUPLE[]>());
		std::shared_ptr<ID_TUPLE[]> _p_id_tuples_2(tmp_array_2, std::default_delete<ID_TUPLE[]>());
		t2 = gs::TimeUtil::timestamp();
		SLOG_CORE("Finish copying id tuples, used " + to_string(t2 - t1) + "ms.");

		thread build_s2value_thread(&Database::build_s2xx, this, _p_id_tuples);
		thread build_o2value_thread(&Database::build_o2xx, this, _p_id_tuples_1);
		thread build_p2value_thread(&Database::build_p2xx, this, _p_id_tuples_2);
		build_s2value_thread.join();
		build_o2value_thread.join();
		build_p2value_thread.join();
		t1 = gs::TimeUtil::timestamp();
		SLOG_CORE("Finish building spo2values, used " + to_string(t1 - t2) + "ms.");
		bar.set_option(indicators::option::PostfixText{"Saving database info 4/5"});
		bar.set_progress(99);
		// WARN:we must free the memory for id_tuples array
		_p_id_tuples.reset();
		_p_id_tuples_1.reset();
		_p_id_tuples_2.reset();
	}

	bool flag = this->saveDBInfoFile();
	if (!flag)
	{
		return false;
	}
	t2 = gs::TimeUtil::timestamp();
	SLOG_CORE("Finish saving DBInfo, used " + to_string(t2 - t1) + "ms.");

	flag = this->saveStatisticsInfoFile();
	if (!flag)
	{
		SLOG_ERROR("the statistics info file of db saved failure!");
	}
	bar.set_option(indicators::option::PostfixText{"Build RDF database done 5/5"});
	bar.set_progress(100);
	return true;
}

bool Database::sub2id_pre2id_obj2id_RDFintoSignature(const std::vector<std::string> &_rdf_files, const string _error_log, indicators::ProgressBar& bar, std::map<string, std::set<std::string>>& id_tuples)
{
	string fname = this->getIDTuplesFile();
	FILE *fp = fopen(fname.c_str(), "wb");
	if (fp == NULL)
	{
		SLOG_ERROR("error in Database::sub2id_pre2id_obj2id() -- unable to open file to write " << fname);
		return false;
	}
	ID_TUPLE tmp_id_tuple;
	{
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

	unsigned cur_file = 0;
	unsigned file_count = _rdf_files.size();
	SLOG_CORE("Finish initial sub2id_pre2id_obj2id");
	ifstream _fin;
	for (unsigned int i = 0; i<file_count; i++)
	{
		_fin.open(_rdf_files[i].c_str());
		if (_fin)
		{
			cur_file = i;
			break;
		}
		if (i == file_count-1)
		{
			SLOG_ERROR("sub2id&pre2id&obj2id: Fail to rdf open : " << _rdf_files[cur_file]);
			return false;
		}
	}

	string _six_tuples_file = this->getSixTuplesFile();
	ofstream _six_tuples_fout(_six_tuples_file.c_str());
	if (!_six_tuples_fout)
	{
		SLOG_ERROR("sub2id&pre2id&obj2id: Fail to tuples open: " << _six_tuples_file);
		return false;
	}

	std::shared_ptr<TripleWithObjType[]> triple_array(new TripleWithObjType[RDFParser::TRIPLE_NUM_PER_GROUP], std::default_delete<TripleWithObjType[]>());

	SLOG_CORE("Begin to build Trie ......");
	int num_lines = 0;
	RDFParser _parser(_fin); // RDFParser is actually invoked twice, see above
	// Enable strict N-Triples mode for .nt files:
	// reject <<...>> without () and reject triple terms as subject
	{
		std::string fpath = _rdf_files[cur_file];
		if (fpath.length() >= 3) {
			std::string ext = fpath.substr(fpath.length() - 3);
			if (ext == ".nt" || ext == ".NT")
				_parser.setNTriplesMode(true);
		}
	}

	num_lines = 0;
	SLOG_CORE("this type predicate name is " << StringUtil::join(this->type_predicate_name, "@@"));
	this->umap.clear();

	int batch_count = 0;
	unordered_set<TYPE_ENTITY_LITERAL_ID> sub_lists;
	std::shared_ptr<ID_TUPLE[]> tmp_id_tuples(new ID_TUPLE[RDFParser::TRIPLE_NUM_PER_GROUP], std::default_delete<ID_TUPLE[]>());
	float progress_unit = 60 / 50 / 10;
	while (true)
	{
		++batch_count;
		int parse_triple_num = 0;
		int64_t t1 = gs::TimeUtil::timestamp();
		int curr_lines = _parser.parseFile(triple_array, parse_triple_num, _error_log, num_lines);
		num_lines = curr_lines;

		if (parse_triple_num == 0)
		{
			if (cur_file >= file_count -1 )
			{
				bar.set_progress(59);
                _fin.close();
				break;	
			}
			cur_file++;
			for (unsigned int i = cur_file; i<file_count; i++)
			{
                _fin.close();
				_fin.open(_rdf_files[i].c_str());
				if (_fin)
				{
                    num_lines = 0;
					cur_file = i;
					break;
				}
			}
			continue;
		}
		for (int i = 0; i < parse_triple_num; i++)
		{
			#ifdef SHOW_PROGRESS
			++bar_tmp;
			if (bar_tmp == one_percent_num)
			{
				bar.tick();
				bar_tmp = 0;
			}
			#endif
            this->triples_num++; // NOTE: triples_num set here
			TripleWithObjType triple_for_spo = triple_array[i];
			string _sub = triple_for_spo.getSubject();
			TYPE_ENTITY_LITERAL_ID _sub_id = INVALID_ENTITY_LITERAL_ID;
			subject2id_RDFintoSignature(_sub, _sub_id, sub_lists);
			string _pre = triple_for_spo.getPredicate();
			TYPE_PREDICATE_ID _pre_id = INVALID_PREDICATE_ID;
			predicate2id_RDFintoSignature(_pre, _pre_id);
			string _obj = triple_for_spo.getObject();
			TYPE_ENTITY_LITERAL_ID _obj_id = INVALID_ENTITY_LITERAL_ID;
			literal2id_RDFintoSignature(_obj, _obj_id, triple_for_spo);

			tmp_id_tuple.subid = _sub_id;
			tmp_id_tuple.preid = _pre_id;
			tmp_id_tuple.objid = _obj_id;
			if (triple_for_spo.isObjEntity() && this->checkIsTypePredicate(_pre))
				id_tuples[_obj].insert(_sub);
				
			tmp_id_tuples[i] = tmp_id_tuple;
		}
		fwrite(tmp_id_tuples.get(), sizeof(ID_TUPLE), parse_triple_num, fp);
		int64_t t2 = gs::TimeUtil::timestamp();
		SLOG_CORE("Alloc ID for triple batch " + to_string(batch_count) + ", batch size = " + to_string(parse_triple_num) + ", use " + to_string(t2-t1) + "ms");
		bar.set_progress(int(batch_count * progress_unit));
	}
	for (const auto& m: id_tuples)
	{
		std::string obj_v = m.first;
		if (obj_v.empty() || m.second.size() == 0)
			continue;
		this->umap.insert(pair<string, unsigned long long>(obj_v, m.second.size()));
	}

	this->kvstore->set_if_single_thread(false);

	triple_array.reset();
	tmp_id_tuples.reset();
	_six_tuples_fout.close();
	fclose(fp);

	return true;
}