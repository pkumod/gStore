/*=============================================================================
# Filename: KVstore.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-23 14:25
# Description:
=============================================================================*/

#include "KVstore.h"

using namespace std;

int
KVstore::getEntityDegree(int _entity_id)
{
	int _in_degree = this->getEntityInDegree(_entity_id);
	int _out_degree = this->getEntityOutDegree(_entity_id);
	return _in_degree + _out_degree;
}

int
KVstore::getEntityOutDegree(int _entity_id)
{
	int* _plist = NULL;
	int _list_len = 0;
	this->getpreIDlistBysubID(_entity_id, _plist, _list_len);
	return _list_len;
}
int
KVstore::getEntityInDegree(int _entity_id)
{
	int* _plist = NULL;
	int _list_len = 0;
	this->getpreIDlistByobjID(_entity_id, _plist, _list_len);
	return _list_len;
}

int
KVstore::getLiteralDegree(int _literal_id)
{
	int* _plist = NULL;
	int _list_len = 0;
	this->getpreIDlistByobjID(_literal_id, _plist, _list_len);
	return _list_len;
}

int
KVstore::getPredicateDegree(int _predicate_id)
{
	int* _slist = NULL;
	int _list_len = 0;
	this->getsubIDlistBypreID(_predicate_id, _slist, _list_len);
	return _list_len;
}


//BETTER:implement these more efficiently? (not copy value)
int
KVstore::getSubjectPredicateDegree(int _subid, int _preid)
{
	int* olist = NULL;
	int len = 0;
	this->getobjIDlistBysubIDpreID(_subid, _preid, olist, len);
	return len;
}

int
KVstore::getObjectPredicateDegree(int _objid, int _preid)
{
	int* slist = NULL;
	int len = 0;
	this->getsubIDlistByobjIDpreID(_objid, _preid, slist, len);
	return len;
}

//================================================================================================


//QUERY:better to insert/remove in ordered list(merge join?)
bool
KVstore::updateInsert_sp2o(int _subid, int _preid, const std::vector<int>& _oidlist)
{
	int* _sp2olist = NULL;
	int _sp2o_len = 0;
	this->getobjIDlistBysubIDpreID(_subid, _preid, _sp2olist, _sp2o_len);
	bool insert = this->insertList_x(_sp2olist, _sp2o_len, _oidlist);

	if (insert)
	{
		this->setobjIDlistBysubIDpreID(_subid, _preid, _sp2olist, _sp2o_len);
	}
	else
	{
		this->addobjIDlistBysubIDpreID(_subid, _preid, _sp2olist, _sp2o_len);
	}
	delete[] _sp2olist;
	_sp2olist = NULL;
	_sp2o_len = 0;

	return true;
}

bool
KVstore::updateRemove_sp2o(int _subid, int _preid, const std::vector<int>& _oidlist)
{
	int* _sp2olist = NULL;
	int _sp2o_len = 0;
	this->getobjIDlistBysubIDpreID(_subid, _preid, _sp2olist, _sp2o_len);
	bool remove = this->removeList_x(_sp2olist, _sp2o_len, _oidlist);

	if (remove)
	{
		this->setobjIDlistBysubIDpreID(_subid, _preid, _sp2olist, _sp2o_len);
	}
	else
	{
		//this->subobjIDlistBysubIDpreID(_subid, _preid, _sp2olist, _sp2o_len);
		int* sp = new int[2];
		sp[0] = _subid;
		sp[1] = _preid;
		this->removeKey(this->subIDpreID2objIDlist, (char*)sp, sizeof(int) * 2);
		delete[] sp;
	}
	delete[] _sp2olist;
	_sp2olist = NULL;
	_sp2o_len = 0;

	return true;
}

bool
KVstore::updateInsert_s2po(int _subid, const std::vector<int>& _pidoidlist)
{
	int* _s2polist = NULL;
	int _s2po_len = 0;
	this->getpreIDobjIDlistBysubID(_subid, _s2polist, _s2po_len);
	bool insert = this->insertList_xy(_s2polist, _s2po_len, _pidoidlist);

	if (insert)
	{
		this->setpreIDobjIDlistBysubID(_subid, _s2polist, _s2po_len);
	}
	else
	{
		this->addpreIDobjIDlistBysubID(_subid, _s2polist, _s2po_len);
	}
	delete[] _s2polist;
	_s2polist = NULL;
	_s2po_len = 0;

	return true;
}

bool
KVstore::updateRemove_s2po(int _subid, const std::vector<int>& _pidoidlist)
{
	int* _s2polist = NULL;
	int _s2po_len = 0;
	this->getpreIDobjIDlistBysubID(_subid, _s2polist, _s2po_len);
	bool remove = this->removeList_xy(_s2polist, _s2po_len, _pidoidlist);

	if (remove)
	{
		this->setpreIDobjIDlistBysubID(_subid, _s2polist, _s2po_len);
	}
	else
	{
		//this->subpreIDobjIDlistBysubID(_subid, _s2polist, _s2po_len);
		this->removeKey(this->subID2preIDobjIDlist, _subid);
	}
	delete[] _s2polist;
	_s2polist = NULL;
	_s2po_len = 0;

	return true;
}

bool
KVstore::updateInsert_s2p(int _subid, const std::vector<int>& _pidlist)
{
	int* _s2plist = NULL;
	int _s2p_len = 0;
	this->getpreIDlistBysubID(_subid, _s2plist, _s2p_len);
	bool insert = this->insertList_x(_s2plist, _s2p_len, _pidlist);

	if (insert)
	{
		this->setpreIDlistBysubID(_subid, _s2plist, _s2p_len);
	}
	else
	{
		this->addpreIDlistBysubID(_subid, _s2plist, _s2p_len);
	}
	delete[] _s2plist;
	_s2plist = NULL;
	_s2p_len = 0;

	return true;
}

bool
KVstore::updateRemove_s2p(int _subid, const std::vector<int>& _pidlist)
{
	int* _s2plist = NULL;
	int _s2p_len = 0;
	this->getpreIDlistBysubID(_subid, _s2plist, _s2p_len);
	bool remove = this->removeList_x(_s2plist, _s2p_len, _pidlist);

	if (remove)
	{
		this->setpreIDlistBysubID(_subid, _s2plist, _s2p_len);
	}
	else
	{
		//this->subpreIDlistBysubID(_subid, _s2plist, _s2p_len);
		this->removeKey(this->subID2preIDlist, _subid);
	}
	delete[] _s2plist;
	_s2plist = NULL;
	_s2p_len = 0;

	return true;

}

bool
KVstore::updateInsert_s2o(int _subid, const std::vector<int>& _oidlist)
{
	int* _s2olist = NULL;
	int _s2o_len = 0;
	this->getobjIDlistBysubID(_subid, _s2olist, _s2o_len);
	bool insert = this->insertList_x(_s2olist, _s2o_len, _oidlist);

	if (insert)
	{
		this->setobjIDlistBysubID(_subid, _s2olist, _s2o_len);
	}
	else
	{
		this->addobjIDlistBysubID(_subid, _s2olist, _s2o_len);
	}
	delete[] _s2olist;
	_s2olist = NULL;
	_s2o_len = 0;

	return true;

}

bool
KVstore::updateRemove_s2o(int _subid, const std::vector<int>& _oidlist)
{
	int* _s2olist = NULL;
	int _s2o_len = 0;
	this->getobjIDlistBysubID(_subid, _s2olist, _s2o_len);
	bool remove = this->removeList_x(_s2olist, _s2o_len, _oidlist);

	if (remove)
	{
		this->setobjIDlistBysubID(_subid, _s2olist, _s2o_len);
	}
	else
	{
		//this->subobjIDlistBysubID(_subid, _s2olist, _s2o_len);
		this->removeKey(this->subID2objIDlist, _subid);
	}
	delete[] _s2olist;
	_s2olist = NULL;
	_s2o_len = 0;

	return true;
}

bool
KVstore::updateInsert_op2s(int _objid, int _preid, const std::vector<int>& _sidlist)
{
	int* _op2slist = NULL;
	int _op2s_len = 0;
	this->getsubIDlistByobjIDpreID(_objid, _preid, _op2slist, _op2s_len);
	bool insert = this->insertList_x(_op2slist, _op2s_len, _sidlist);

	if (insert)
	{
		this->setsubIDlistByobjIDpreID(_objid, _preid, _op2slist, _op2s_len);
	}
	else
	{
		this->addsubIDlistByobjIDpreID(_objid, _preid, _op2slist, _op2s_len);
	}
	delete[] _op2slist;
	_op2slist = NULL;
	_op2s_len = 0;

	return true;
}

bool
KVstore::updateRemove_op2s(int _objid, int _preid, const std::vector<int>& _sidlist)
{
	int* _op2slist = NULL;
	int _op2s_len = 0;
	this->getsubIDlistByobjIDpreID(_objid, _preid, _op2slist, _op2s_len);
	bool remove = this->removeList_x(_op2slist, _op2s_len, _sidlist);

	if (remove)
	{
		this->setsubIDlistByobjIDpreID(_objid, _preid, _op2slist, _op2s_len);
	}
	else
	{
		//this->subsubIDlistByobjIDpreID(_objid, _preid, _op2slist, _op2s_len);
		int* op = new int[2];
		op[0] = _objid;
		op[1] = _preid;
		this->removeKey(this->objIDpreID2subIDlist, (char*)op, sizeof(int) * 2);
		delete[] op;
	}
	delete[] _op2slist;
	_op2slist = NULL;
	_op2s_len = 0;

	return true;
}

bool
KVstore::updateInsert_o2ps(int _objid, const std::vector<int>& _pidsidlist)
{
	int* _o2pslist = NULL;
	int _o2ps_len = 0;
	this->getpreIDsubIDlistByobjID(_objid, _o2pslist, _o2ps_len);
	bool insert = this->insertList_xy(_o2pslist, _o2ps_len, _pidsidlist);

	if (insert)
	{
		this->setpreIDsubIDlistByobjID(_objid, _o2pslist, _o2ps_len);
	}
	else
	{
		this->addpreIDsubIDlistByobjID(_objid, _o2pslist, _o2ps_len);
	}
	delete[] _o2pslist;
	_o2pslist = NULL;
	_o2ps_len = 0;

	return true;

}

bool
KVstore::updateRemove_o2ps(int _objid, const std::vector<int>& _pidsidlist)
{
	int* _o2pslist = NULL;
	int _o2ps_len = 0;
	this->getpreIDsubIDlistByobjID(_objid, _o2pslist, _o2ps_len);
	bool remove = this->removeList_xy(_o2pslist, _o2ps_len, _pidsidlist);

	if (remove)
	{
		this->setpreIDsubIDlistByobjID(_objid, _o2pslist, _o2ps_len);
	}
	else
	{
		//this->subpreIDsubIDlistByobjID(_objid, _o2pslist, _o2ps_len);
		this->removeKey(this->objID2preIDsubIDlist, _objid);
	}
	delete[] _o2pslist;
	_o2pslist = NULL;
	_o2ps_len = 0;

	return true;
}

bool
KVstore::updateInsert_o2p(int _objid, const std::vector<int>& _pidlist)
{
	int* _o2plist = NULL;
	int _o2p_len = 0;
	this->getpreIDlistByobjID(_objid, _o2plist, _o2p_len);
	bool insert = this->insertList_x(_o2plist, _o2p_len, _pidlist);

	if (insert)
	{
		this->setpreIDlistByobjID(_objid, _o2plist, _o2p_len);
	}
	else
	{
		this->addpreIDlistByobjID(_objid, _o2plist, _o2p_len);
	}
	delete[] _o2plist;
	_o2plist = NULL;
	_o2p_len = 0;

	return true;
}

bool
KVstore::updateRemove_o2p(int _objid, const std::vector<int>& _pidlist)
{
	int* _o2plist = NULL;
	int _o2p_len = 0;
	this->getpreIDlistByobjID(_objid, _o2plist, _o2p_len);
	bool remove = this->removeList_x(_o2plist, _o2p_len, _pidlist);

	if (remove)
	{
		this->setpreIDlistByobjID(_objid, _o2plist, _o2p_len);
	}
	else
	{
		//this->subpreIDlistByobjID(_objid, _o2plist, _o2p_len);
		this->removeKey(this->objID2preIDlist, _objid);
	}
	delete[] _o2plist;
	_o2plist = NULL;
	_o2p_len = 0;

	return true;
}

bool
KVstore::updateInsert_o2s(int _objid, const std::vector<int>& _sidlist)
{
	int* _o2slist = NULL;
	int _o2s_len = 0;
	this->getsubIDlistByobjID(_objid, _o2slist, _o2s_len);
	bool insert = this->insertList_x(_o2slist, _o2s_len, _sidlist);

	if (insert)
	{
		this->setsubIDlistByobjID(_objid, _o2slist, _o2s_len);
	}
	else
	{
		this->addsubIDlistByobjID(_objid, _o2slist, _o2s_len);
	}
	delete[] _o2slist;
	_o2slist = NULL;
	_o2s_len = 0;

	return true;
}

bool
KVstore::updateRemove_o2s(int _objid, const std::vector<int>& _sidlist)
{
	int* _o2slist = NULL;
	int _o2s_len = 0;
	this->getsubIDlistByobjID(_objid, _o2slist, _o2s_len);
	bool remove = this->removeList_x(_o2slist, _o2s_len, _sidlist);

	if (remove)
	{
		this->setsubIDlistByobjID(_objid, _o2slist, _o2s_len);
	}
	else
	{
		//this->subsubIDlistByobjID(_objid, _o2slist, _o2s_len);
		this->removeKey(this->objID2subIDlist, _objid);
	}
	delete[] _o2slist;
	_o2slist = NULL;
	_o2s_len = 0;

	return true;
}

bool
KVstore::updateInsert_p2so(int _preid, const std::vector<int>& _sidoidlist)
{
	int* _p2solist = NULL;
	int _p2so_len = 0;
	this->getsubIDobjIDlistBypreID(_preid, _p2solist, _p2so_len);
	bool insert = this->insertList_x(_p2solist, _p2so_len, _sidoidlist);

	if (insert)
	{
		this->setsubIDobjIDlistBypreID(_preid, _p2solist, _p2so_len);
	}
	else
	{
		this->addsubIDobjIDlistBypreID(_preid, _p2solist, _p2so_len);
	}
	delete[] _p2solist;
	_p2solist = NULL;
	_p2so_len = 0;

	return true;
}

bool
KVstore::updateRemove_p2so(int _preid, const std::vector<int>& _sidoidlist)
{
	int* _p2solist = NULL;
	int _p2so_len = 0;
	this->getsubIDobjIDlistBypreID(_preid, _p2solist, _p2so_len);
	bool remove = this->removeList_x(_p2solist, _p2so_len, _sidoidlist);

	if (remove)
	{
		this->setsubIDobjIDlistBypreID(_preid, _p2solist, _p2so_len);
	}
	else
	{
		//this->subsubIDobjIDlistBypreID(_preid, _p2solist, _p2so_len);
		this->removeKey(this->preID2subIDobjIDlist, _preid);
	}
	delete[] _p2solist;
	_p2solist = NULL;
	_p2so_len = 0;

	return true;
}

bool
KVstore::updateInsert_p2s(int _preid, const std::vector<int>& _sidlist)
{
	int* _p2slist = NULL;
	int _p2s_len = 0;
	this->getsubIDlistBypreID(_preid, _p2slist, _p2s_len);
	bool insert = this->insertList_x(_p2slist, _p2s_len, _sidlist);

	if (insert)
	{
		this->setsubIDlistBypreID(_preid, _p2slist, _p2s_len);
	}
	else
	{
		this->addsubIDlistBypreID(_preid, _p2slist, _p2s_len);
	}
	delete[] _p2slist;
	_p2slist = NULL;
	_p2s_len = 0;

	return true;
}

bool
KVstore::updateRemove_p2s(int _preid, const std::vector<int>& _sidlist)
{
	int* _p2slist = NULL;
	int _p2s_len = 0;
	this->getsubIDlistBypreID(_preid, _p2slist, _p2s_len);
	bool remove = this->removeList_x(_p2slist, _p2s_len, _sidlist);

	if (remove)
	{
		this->setsubIDlistBypreID(_preid, _p2slist, _p2s_len);
	}
	else
	{
		//this->subsubIDlistBypreID(_preid, _p2slist, _p2s_len);
		this->removeKey(this->preID2subIDlist, _preid);
	}
	delete[] _p2slist;
	_p2slist = NULL;
	_p2s_len = 0;

	return true;
}

bool
KVstore::updateInsert_p2o(int _preid, const std::vector<int>& _oidlist)
{
	int* _p2olist = NULL;
	int _p2o_len = 0;
	this->getobjIDlistBypreID(_preid, _p2olist, _p2o_len);
	bool insert = this->insertList_x(_p2olist, _p2o_len, _oidlist);

	if (insert)
	{
		this->setsubIDlistBypreID(_preid, _p2olist, _p2o_len);
	}
	else
	{
		this->addsubIDlistBypreID(_preid, _p2olist, _p2o_len);
	}
	delete[] _p2olist;
	_p2olist = NULL;
	_p2o_len = 0;

	return true;
}

bool
KVstore::updateRemove_p2o(int _preid, const std::vector<int>& _oidlist)
{
	int* _p2olist = NULL;
	int _p2o_len = 0;
	this->getobjIDlistBypreID(_preid, _p2olist, _p2o_len);
	bool remove = this->removeList_x(_p2olist, _p2o_len, _oidlist);

	if (remove)
	{
		this->setsubIDlistBypreID(_preid, _p2olist, _p2o_len);
	}
	else
	{
		//this->subsubIDlistBypreID(_preid, _p2olist, _p2o_len);
		this->removeKey(this->preID2objIDlist, _preid);
	}
	delete[] _p2olist;
	_p2olist = NULL;
	_p2o_len = 0;

	return true;
}

//============================================================================================================

//there are two situation when we need to update tuples list: s2o o2s sp2o op2s s2po o2ps s2p p2s o2p p2o so2p p2so
//1. insert triple(finished in this function)
//2. remove triple
//before call this function, we were sure that this triple did not exist
int
KVstore::updateTupleslist_insert(int _sub_id, int _pre_id, int _obj_id)
{
#ifdef DEBUG_PRECISE
	stringstream _ss;
	_ss << "updateTupleslist_insert: " << _sub_id << " " << _pre_id << " " << _obj_id << endl;
	Util::logging(_ss.str());
#endif

	int updateListLen = 0;
	//bool ret = true;
	bool insert = true;

	//update sp2o
	{
		//cout<<"this for sp2o in update_insert"<<endl;
		int* _sp2olist = NULL;
		int _sp2o_len = 0;
		this->getobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);

		//cout<<"original sp2o len:"<<_sp2o_len<<endl;

		//if no duplication, _insert will be true
		//this->setXXX function will override the previous value
		insert = this->insert_x(_sp2olist, _sp2o_len, _obj_id);

		//cout<<"current sp2o len:"<<_sp2o_len<<endl;
		//cout<<_sp2olist[0]<<this->getEntityByID(_sp2olist[0])<<endl;
		//cout<<_sub_id<<" "<<_pre_id<<endl;

		if (insert)
		{
			//if(ret)
			//{
			this->setobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);
			//}
		}
		else
		{
			this->addobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);
		}

		//int* list = NULL;
		//int len = 0;
		//int root = this->getIDByEntity("<root>");
		//int contain = this->getIDByPredicate("<contain>");
		//this->getobjIDlistBysubIDpreID(root, contain, list, len);
		//cout<<root<<" "<<contain<<endl;
		//if(len == 0)
		//{
		//cout<<"no result"<<endl;
		//}
		//for(int i = 0; i < len; ++i)
		//{
		//cout << this->getEntityByID(list[i])<<" "<<list[i]<<endl;
		//}

		//updateListLen += _sp2o_len;
		delete[] _sp2olist;
		_sp2olist = NULL;
		_sp2o_len = 0;

	}

	//  Util::logging("update sp2o done.");

	//update op2s
	{
		int* _op2slist = NULL;
		int _op2s_len = 0;
		this->getsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);

		//if no duplication, _insert will be true
		//this->setXXX function will override the previous value
		insert = this->insert_x(_op2slist, _op2s_len, _sub_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);
			//}
		}
		else //key not exist
		{
			this->addsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);
		}

		//updateListLen += _op2s_len;
		delete[] _op2slist;
		_op2slist = NULL;
		_op2s_len = 0;
	}

	//  Util::logging("update op2s done.");

	//update s2po
	{
		int* _s2polist = NULL;
		int _s2po_len = 0;
		this->getpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);

		//if no duplication, _insert will be true
		//this->setXXX function will override the previous value
		insert = this->insert_xy(_s2polist, _s2po_len, _pre_id, _obj_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);
			//}
		}
		else
		{
			this->addpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);
		}

		//updateListLen += _s2po_len;
		delete[] _s2polist;
		_s2polist = NULL;
		_s2po_len = 0;
	}

	//  Util::logging("update s2po done.");

	//update o2ps
	{
		int* _o2pslist = NULL;
		int _o2ps_len = 0;
		this->getpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);

		//if no duplication, _insert will be true
		//this->setXXX function will override the previous value
		insert = this->insert_xy(_o2pslist, _o2ps_len, _pre_id, _sub_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);
			//}
		}
		else
		{
			this->addpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);
		}

		//updateListLen += _o2ps_len;
		delete[] _o2pslist;
		_o2pslist = NULL;
		_o2ps_len = 0;
	}

	//  Util::logging("update o2ps done.");

	//update s2o
	{
		int* _s2olist = NULL;
		int _s2o_len = 0;
		this->getobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
		//if no duplication, _insert will be true
		//this->setXXX function will override the previous value
		insert = this->insert_x(_s2olist, _s2o_len, _obj_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
			//}
		}
		else
		{
			this->addobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
		}

		//updateListLen += _s2o_len;
		delete[] _s2olist;
		_s2olist = NULL;
		_s2o_len = 0;
	}

	//    Util::logging("update s2o done.");

	//update o2s
	{
		int* _o2slist = NULL;
		int _o2s_len = 0;
		this->getsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);
		//if no duplication, _insert will be true
		//this->setXXX function will override the previous value
		insert = this->insert_x(_o2slist, _o2s_len, _sub_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);
			//}
		}
		else
		{
			this->addsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);
		}
		//updateListLen += _o2s_len;
		delete[] _o2slist;
		_o2slist = NULL;
		_o2s_len = 0;
	}

	//update s2p
	{
		int* _s2plist = NULL;
		int _s2p_len = 0;
		this->getpreIDlistBysubID(_sub_id, _s2plist, _s2p_len);
		insert = this->insert_x(_s2plist, _s2p_len, _pre_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setpreIDlistBysubID(_sub_id, _s2plist, _s2p_len);
			//}
		}
		else
		{
			this->addpreIDlistBysubID(_sub_id, _s2plist, _s2p_len);
		}
		//updateListLen += _s2p_len;
		delete[] _s2plist;
		_s2plist = NULL;
		_s2p_len = 0;
	}

	//update p2s
	{
		int* _p2slist = NULL;
		int _p2s_len = 0;
		this->getsubIDlistBypreID(_pre_id, _p2slist, _p2s_len);
		insert = this->insert_x(_p2slist, _p2s_len, _sub_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setsubIDlistBypreID(_pre_id, _p2slist, _p2s_len);
			//}
		}
		else
		{
			this->addsubIDlistBypreID(_pre_id, _p2slist, _p2s_len);
		}
		//updateListLen += _p2s_len;
		delete[] _p2slist;
		_p2slist = NULL;
		_p2s_len = 0;
	}

	//update o2p
	{
		int* _o2plist = NULL;
		int _o2p_len = 0;
		this->getpreIDlistByobjID(_obj_id, _o2plist, _o2p_len);
		insert = this->insert_x(_o2plist, _o2p_len, _pre_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setpreIDlistByobjID(_obj_id, _o2plist, _o2p_len);
			//}
		}
		else
		{
			this->addpreIDlistByobjID(_obj_id, _o2plist, _o2p_len);
		}
		//updateListLen += _o2p_len;
		delete[] _o2plist;
		_o2plist = NULL;
		_o2p_len = 0;
	}

	//update p2o
	{
		int* _p2olist = NULL;
		int _p2o_len = 0;
		this->getobjIDlistBypreID(_pre_id, _p2olist, _p2o_len);
		insert = this->insert_x(_p2olist, _p2o_len, _obj_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setobjIDlistBypreID(_pre_id, _p2olist, _p2o_len);
			//}
		}
		else
		{
			this->addobjIDlistBypreID(_pre_id, _p2olist, _p2o_len);
		}
		//updateListLen += _p2o_len;
		delete[] _p2olist;
		_p2olist = NULL;
		_p2o_len = 0;
	}

	//update so2p
	//{
	//	int* _so2plist = NULL;
	//	int _so2p_len = 0;
	//	ret = this->getpreIDlistBysubIDobjID(_sub_id, _obj_id, _so2plist, _so2p_len);
	//	bool _insert = this->insert_x(_so2plist, _so2p_len, _pre_id);
	//	if(_insert)
	//	{
	////	if(ret)
	//		this->setpreIDlistBysubIDobjID(_sub_id, _obj_id, _so2plist, _so2p_len);
	//	}
	//	else
	//		this->addpreIDlistBysubIDobjID(_sub_id, _obj_id, _so2plist, _so2p_len);
	//	updateListLen += _so2p_len;
	//	delete[] _so2plist;
	//	_so2plist = NULL;
	//	_so2p_len = 0;
	//}

	//update p2so
	{
		//cout<<"this for p2so in update_insert"<<endl;
		int* _p2solist = NULL;
		int _p2so_len = 0;
		this->getsubIDobjIDlistBypreID(_pre_id, _p2solist, _p2so_len);
		insert = this->insert_xy(_p2solist, _p2so_len, _sub_id, _obj_id);
		if (insert)
		{
			//if(ret)
			//{
			this->setsubIDobjIDlistBypreID(_pre_id, _p2solist, _p2so_len);
			//}
		}
		else
		{
			this->addsubIDobjIDlistBypreID(_pre_id, _p2solist, _p2so_len);
		}
		//updateListLen += _p2so_len;
		delete[] _p2solist;
		_p2solist = NULL;
		_p2so_len = 0;
	}

	return updateListLen;

	//   Util::logging("update o2s done.");
}

//there are two situation when we need to update tuples list: s2o o2s sp2o op2s s2po o2ps s2p p2s o2p p2o so2p p2so
//1. insert triple
//2. remove triple(finished in this function)
//before call this function, we were sure that this triple did exist
void
KVstore::updateTupleslist_remove(int _sub_id, int _pre_id, int _obj_id)
{
	//bool ret = true;
	bool remove = true;

	//update sp2o
	{
		int* _sp2olist = NULL;
		int _sp2o_len = 0;
		this->getobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);
		remove = this->remove_x(_sp2olist, _sp2o_len, _obj_id);
		if (remove)
		{
			this->setobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);
		}
		//if(_sp2o_len == 0)
		else   //already empty
		{
			int* _sp = new int[2];
			_sp[0] = _sub_id;
			_sp[1] = _pre_id;
			this->removeKey(this->subIDpreID2objIDlist, (char*)_sp, sizeof(int) * 2);
			delete[] _sp;
		}
		delete[] _sp2olist;
	}
	//cout<<"remove in sp2o"<<endl;

	//update op2s
	{
		int* _op2slist = NULL;
		int _op2s_len = 0;
		this->getsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);
		//cout <<"tag 1"<<endl;
		remove = this->remove_x(_op2slist, _op2s_len, _sub_id);
		//cout <<"tag 2"<<endl;
		if (remove)
		{
		//cout <<"tag 3"<<endl;
		//cout<<_op2s_len<<" "<<_op2slist[0]<<endl;
		//cout<<this->getEntityByID(_sub_id)<<endl;
		//cout<<this->getPredicateByID(_pre_id)<<endl;
	//cout<<this->getEntityByID(_obj_id)<<endl;
		//cout<<this->getEntityByID(_op2slist[0])<<endl;
			this->setsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);
		}
		//if(_op2s_len == 0)
		else
		{
		//cout <<"tag 4"<<endl;
			int* _sp = new int[2];
			_sp[0] = _obj_id;
			_sp[1] = _pre_id;
			this->removeKey(this->objIDpreID2subIDlist, (char*)_sp, sizeof(int) * 2);
			delete[] _sp;
		}
		//cout <<"tag 5"<<endl;
		delete[] _op2slist;
	}
	//cout<<"remove in op2s"<<endl;

	//update s2po
	{
		int* _s2polist = NULL;
		int _s2po_len = 0;
		this->getpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);
		remove = this->remove_xy(_s2polist, _s2po_len, _pre_id, _obj_id);
		if (remove)
		{
			this->setpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);
		}
		//if(_s2po_len == 0)
		else
		{
			this->removeKey(this->subID2preIDobjIDlist, _sub_id);
		}
		delete[] _s2polist;
	}
	//cout<<"remove in s2po"<<endl;

	//update o2ps
	{
		int* _o2pslist = NULL;
		int _o2ps_len = 0;
		this->getpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);
		remove = this->remove_xy(_o2pslist, _o2ps_len, _pre_id, _sub_id);
		if (remove)
		{
			this->setpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);
		}
		//if(_o2ps_len == 0)
		else
		{
			this->removeKey(this->objID2preIDsubIDlist, _obj_id);
		}
		delete[] _o2pslist;
	}
	//cout<<"remove in o2ps"<<endl;

	//update s2o
	{
		int* _s2olist = NULL;
		int _s2o_len = 0;
		this->getobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
		remove = this->remove_x(_s2olist, _s2o_len, _obj_id);
		if (remove)
		{
			this->setobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
		}
		//if(_s2o_len == 0)
		else
		{
			this->removeKey(this->subID2objIDlist, _sub_id);
		}
		delete[] _s2olist;
	}
	//cout<<"remove in s2o"<<endl;

	//update o2s
	{
		int* _o2slist = NULL;
		int _o2s_len = 0;
		this->getsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);
		remove = this->remove_x(_o2slist, _o2s_len, _sub_id);
		if (remove)
		{
			this->setsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);
		}
		//if(_o2s_len == 0)
		else
		{
			this->removeKey(this->objID2subIDlist, _obj_id);
		}
		delete[] _o2slist;
	}
	//cout<<"remove in o2s"<<endl;

	//update s2p
	{
		int* _s2plist = NULL;
		int _s2p_len = 0;
		this->getpreIDlistBysubID(_sub_id, _s2plist, _s2p_len);
		remove = this->remove_x(_s2plist, _s2p_len, _pre_id);
		if (remove)
		{
			this->setpreIDlistBysubID(_sub_id, _s2plist, _s2p_len);
		}
		//if(_s2p_len == 0)
		else
		{
			this->removeKey(this->subID2preIDlist, _sub_id);
		}
		delete[] _s2plist;
	}
	//cout<<"remove in s2p"<<endl;

	//update p2s
	{
		int* _p2slist = NULL;
		int _p2s_len = 0;
		this->getsubIDlistBypreID(_pre_id, _p2slist, _p2s_len);
		if(_p2slist == NULL)
		{
			cout<<"null list in updating p2s: "<<_p2s_len<<endl;
		}
		remove = this->remove_x(_p2slist, _p2s_len, _sub_id);
		//cout<<"after remove"<<endl;
		if (remove)
		{
			//cout<<"to modify in p2s: "<<_p2s_len<<endl;
			this->setsubIDlistBypreID(_pre_id, _p2slist, _p2s_len);
			//cout<<"modify ok"<<endl;
		}
		//if(_p2s_len == 0)
		else
		{
			//cout<<"to remove key in p2s"<<endl;
			this->removeKey(this->preID2subIDlist, _pre_id);
		}
		//cout<<"to delete"<<endl;
		delete[] _p2slist;
		//cout<<"delete ok"<<endl;
	}
	//cout<<"remove in p2s"<<endl;

	//update o2p
	{
		int* _o2plist = NULL;
		int _o2p_len = 0;
		this->getpreIDlistByobjID(_obj_id, _o2plist, _o2p_len);
		remove = this->remove_x(_o2plist, _o2p_len, _pre_id);
		if (remove)
		{
			this->setpreIDlistByobjID(_obj_id, _o2plist, _o2p_len);
		}
		//if(_o2p_len == 0)
		else
		{
			this->removeKey(this->objID2preIDlist, _obj_id);
		}
		delete[] _o2plist;
	}
	//cout<<"remove in o2p"<<endl;

	//update p2o
	{
		int* _p2olist = NULL;
		int _p2o_len = 0;
		this->getobjIDlistBypreID(_pre_id, _p2olist, _p2o_len);
		remove = this->remove_x(_p2olist, _p2o_len, _obj_id);
		if (remove)
		{
			this->setobjIDlistBypreID(_pre_id, _p2olist, _p2o_len);
		}
		//if(_p2o_len == 0)
		else
		{
			this->removeKey(this->preID2objIDlist, _pre_id);
		}
		delete[] _p2olist;
	}
	//cout<<"remove in p2o"<<endl;

	//update so2p
	//{
	//	int* _so2plist = NULL;
	//	int _so2p_len = 0;
	//	this->getpreIDlistBysubIDobjID(_sub_id, _obj_id, _so2plist, _so2p_len);
	//	bool _remove = this->remove_x(_so2plist, _so2p_len, _pre_id);
	//	if(_so2p_len == 0)
	//	{
	//		int* _so = new int[2];
	//		_so[0] = _sub_id;
	//		_so[1] = _obj_id;
	//		this->removeKey(this->subIDobjID2preIDlist, (char*)_so, sizeof(int)*2 );
	//		delete[] _so;
	//	}
	//	else if(_remove)
	//	{
	//		this->setpreIDlistBysubIDobjID(_sub_id, _obj_id, _so2plist, _so2p_len);
	//	}
	//	delete[] _so2plist;
	//}

	//update p2so
	{
		int* _p2solist = NULL;
		int _p2so_len = 0;
		this->getsubIDobjIDlistBypreID(_pre_id, _p2solist, _p2so_len);
		remove = this->remove_xy(_p2solist, _p2so_len, _sub_id, _obj_id);
		if (remove)
		{
			this->setsubIDobjIDlistBypreID(_pre_id, _p2solist, _p2so_len);
		}
		//if(_p2so_len == 0)
		else
		{
			this->removeKey(this->preID2subIDobjIDlist, _pre_id);
		}

		delete[] _p2solist;
	}
	//cout<<"remove in p2so"<<endl;
}


//============================================================================================================


//insert <_x_id, _y_id> into _xylist(keep _xylist(<x,y>) in ascending order)
bool
KVstore::insert_xy(int*& _xylist, int& _list_len, int _x_id, int _y_id)
{
	//NOTICE:to keep remove working well, duplicates are needed
	//check duplication
	//for(int i = 0; i < _list_len; i += 2)
	//{
	//if(_xylist[i] == _x_id &&
	//_xylist[i+1] == _y_id )
	//{
	//return false;
	//}
	//}

	int _new_list_len = _list_len + 2;
	int* _new_xylist = new int[_new_list_len];

	//if _xylist does not exist
	if (_xylist == NULL)
	{
		_new_xylist[0] = _x_id;
		_new_xylist[1] = _y_id;
		_xylist = _new_xylist;
		_list_len = _new_list_len;
		//return true;
		return false;
	}

	bool _insert_head = (_xylist[0] > _x_id) || (_xylist[0] == _x_id && _xylist[1] >= _y_id);
	if (_insert_head)
	{
		_new_xylist[0] = _x_id;
		_new_xylist[1] = _y_id;
		memcpy(_new_xylist + 2, _xylist, _list_len*(sizeof(int)));
		delete[] _xylist;
		_xylist = _new_xylist;
		_list_len = _new_list_len;
	}
	else
	{
		_new_xylist[0] = _xylist[0];
		_new_xylist[1] = _xylist[1];
		int j = 2;
		int i = 2;

		bool _insert_xyid = false;
		bool found = false;
		while (i < _list_len)
		{
			if(!found)
			{
				bool _gt_previous = (_xylist[i - 2] < _x_id) || (_xylist[i - 2] == _x_id && _xylist[i - 1] <= _y_id);
				bool _lt_current = (_x_id < _xylist[i]) || (_x_id == _xylist[i] && _y_id <= _xylist[i + 1]);
				_insert_xyid = _gt_previous && _lt_current;
				if (_insert_xyid)
				{
					//insert the new pair.
					_new_xylist[j] = _x_id;
					_new_xylist[j + 1] = _y_id;
					j += 2;
					found = true;
				}
			}

			//copy the ith old pair to the new list.
			_new_xylist[j] = _xylist[i];
			_new_xylist[j + 1] = _xylist[i + 1];
			j += 2;
			i += 2;
		}

		bool _insert_tail = (j == _list_len);
		if (_insert_tail)
		{
			_new_xylist[j] = _x_id;
			_new_xylist[j + 1] = _y_id;
			j += 2;
		}

		delete[] _xylist;
		_xylist = _new_xylist;
		_list_len = _new_list_len;
	}

	return true;
}

//insert _x_id into _xlist(keep _xlist in ascending order)
bool
KVstore::insert_x(int*& _xlist, int& _list_len, int _x_id)
{
	//NOTICE:to keep remove working well, duplicates are needed
	//check duplication
	//for(int i = 0; i < _list_len; i ++)
	//{
	//if(_xlist[i] == _x_id)
	//{
	//return false;
	//}
	//}

	int _new_list_len = _list_len + 1;
	int* _new_xlist = new int[_new_list_len];
	
	//DEBUG
	//cerr<<"new list len"<<_new_list_len<<endl;

	//if _xlist does not exist before
	if (_xlist == NULL)
	{
		//DEBUG!!!
		//cerr<<"empty list!!!"<<endl;

		_new_xlist[0] = _x_id;
		_xlist = _new_xlist;
		_list_len = _new_list_len;
		//return true;
		return false;
	}

	bool _insert_head = _x_id <= _xlist[0];
	if (_insert_head)
	{
		//cerr<<"insert in head"<<endl;

		_new_xlist[0] = _x_id;
		memcpy(_new_xlist + 1, _xlist, _list_len*(sizeof(int)));
		delete[] _xlist;
		_xlist = _new_xlist;
		_list_len = _new_list_len;
	}
	else
	{
		_new_xlist[0] = _xlist[0];
		int j = 1;
		int i = 1;
		bool _insert_xid = false;
		bool found = false;
		while (i < _list_len)
		{
			if(!found)
			{
				_insert_xid = (_xlist[i - 1] <= _x_id) && (_x_id <= _xlist[i]);
				//insert the new element.
				if (_insert_xid)
				{
					//cerr<<"insert now: "<<i <<" "<<j<<endl;
					_new_xlist[j] = _x_id;
					j++;
					found = true;
				}
			}

			//cerr<<"current pos: "<<i<<" "<<j<<endl;

			//copy the ith old element to the new list.
			_new_xlist[j] = _xlist[i];
			j++;
			i++;
		}

		bool _insert_tail = (j == _list_len);
		if (_insert_tail)
		{
			//cerr<<"to insert in tail"<<endl;
			_new_xlist[j] = _x_id;
			j++;
		}

		delete[] _xlist;
		_xlist = _new_xlist;
		_list_len = _new_list_len;
	}
	return true;
}

//NOTICE:below two functions(remove_x/remove_xy) find the first matching one and remove
//not all removed(duplicates needed sometimes)

bool
KVstore::remove_x(int*& _xlist, int& _list_len, int _x_id)
{
	if(_xlist == NULL)
	{
		return false;
	}

	for (int i = 0; i < _list_len; i++)
	{
		if (_xlist[i] == _x_id)
		{
			//move the latter ones backward
			for (int j = i + 1; j < _list_len; j++)
			{
				_xlist[j - 1] = _xlist[j];
			}
			_xlist[_list_len - 1] = 0;
			_list_len--;
			if (_list_len > 0)
				return true;
			else
			{
				//_xlist = NULL;
				return false;
			}
		}
	}

	return true;
}

bool
KVstore::remove_xy(int*& _xylist, int& _list_len, int _x_id, int _y_id)
{
	if(_xylist == NULL)
	{
		return false;
	}

	for (int i = 0; i < _list_len; i += 2)
	{
		bool _remove = (_xylist[i] == _x_id) && (_xylist[i + 1] == _y_id);
		if (_remove)
		{
			//move the latter pairs backward
			for (int j = i + 2; j < _list_len; j += 2)
			{
				_xylist[j - 2] = _xylist[j];
				_xylist[j - 1] = _xylist[j + 1];
			}
			_xylist[_list_len - 2] = 0;
			_xylist[_list_len - 1] = 0;
			_list_len -= 2;
			if (_list_len > 0)
				return true;
			else
				return false;
		}
	}
	return true;
}

bool
KVstore::insertList_x(int*& _xlist, int& _list_len, const std::vector<int>& _list2)
{
	int num = _list2.size();
	int _new_list_len = _list_len + num;
	int* _new_xlist = new int[_new_list_len];
	int i, j, k;

	//if _xlist does not exist before
	if (_xlist == NULL)
	{
		for (i = 0; i < num; ++i)
		{
			_new_xlist[i] = _list2[i];
		}
		_xlist = _new_xlist;
		_list_len = _new_list_len;
		return false;
	}

	//To Merge
	i = 0;
	j = 0;
	k = 0;
	bool select = false; //choose the first list
	while (i < _list_len || j < num) //or k < _new_list_len
	{
		if (i >= _list_len)
		{
			select = true;
		}
		else if (j >= num)
		{
			select = false;
		}
		else
		{
			if (_xlist[i] <= _list2[j])
			{
				select = false;
			}
			else
			{
				select = true;
			}
		}

		if (!select)
		{
			_new_xlist[k] = _xlist[i];
			i++;
		}
		else
		{
			_new_xlist[k] = _list2[j];
			j++;
		}
		k++;
	}

	delete[] _xlist;
	_xlist = _new_xlist;
	_list_len = _new_list_len;

	return true;
}

//NOTICE:there can not be same po pair in xy-list! (different from x list)
//not already exist in list
//
bool
KVstore::insertList_xy(int*& _xylist, int& _list_len, const std::vector<int>& _list2)
{
	int num = _list2.size();
	int _new_list_len = _list_len + num;
	int* _new_xylist = new int[_new_list_len];
	int i, j, k;

	//if _xylist does not exist
	if (_xylist == NULL)
	{
		for (i = 0; i < num; ++i)
		{
			_new_xylist[i] = _list2[i];
		}
		_xylist = _new_xylist;
		_list_len = _new_list_len;
		return false;
	}

	//To merge, sort on key1 and key2
	i = 0;
	j = 0;
	k = 0;
	bool select = false; //choose the first list
	int comp = 0;
	while (i < _list_len || j < num) //or k < _new_list_len
	{
		if (i >= _list_len)
		{
			select = true;
		}
		else if (j >= num)
		{
			select = false;
		}
		else
		{
			comp = Util::compIIpair(_xylist[i], _xylist[i + 1], _list2[j], _list2[j + 1]);
			//if(_xylist[i] < _list2[j] || (_xylist[i] == _list2[j] && _xylist[i+1] <= _list2[j+1]))
			if (comp == -1)
			{
				select = false;
			}
			else
			{
				select = true;
			}
		}

		if (!select)
		{
			_new_xylist[k] = _xylist[i];
			_new_xylist[k + 1] = _xylist[i + 1];
			i += 2;
		}
		else
		{
			_new_xylist[k] = _list2[j];
			_new_xylist[k + 1] = _list2[j + 1];
			j += 2;
		}
		k += 2;
	}

	delete[] _xylist;
	_xylist = _new_xylist;
	_list_len = _new_list_len;

	return true;
}

//NOTICE: In Database, we ensure that only when spo exist in db, remove can be valid
//So the xlist can not be NULL possible
//the eles in _list2 all exist in _xlist, no need to judge here(already judged)
//each ele in _list2 should only remove once in _xlist
//
bool
KVstore::removeList_x(int*& _xlist, int& _list_len, const std::vector<int>& _list2)
{
	int num = _list2.size();
	int _new_list_len = _list_len - num;

	if (_new_list_len == 0)
	{
		delete[] _xlist;
		_xlist = NULL;
		_list_len = 0;
		return false;
	}

	int* _new_xlist = new int[_new_list_len];
	int i, j, k;

	i = 0;
	j = 0;
	k = 0;
	while (k < _new_list_len)
	{
		//NOTICE:i = _list_len can not occur first due to existence of _list2 in _xlist
		if (j >= num || _xlist[i] < _list2[j])
		{
			_new_xlist[k] = _xlist[i];
			k++;
			i++;
		}
		else if (_xlist[i] > _list2[j])
		{
			j++;
		}
		else //==
		{
			i++;
			j++;
		}
	}

	delete[] _xlist;
	_xlist = _new_xlist;
	_list_len = _new_list_len;

	return true;
}

bool
KVstore::removeList_xy(int*& _xylist, int& _list_len, const std::vector<int>& _list2)
{
	int num = _list2.size();
	int _new_list_len = _list_len - num;

	if (_new_list_len == 0)
	{
		delete[] _xylist;
		_xylist = NULL;
		_list_len = 0;
		return false;
	}

	int* _new_xylist = new int[_new_list_len];
	int i, j, k;

	//To merge, sort on key1 and key2
	i = 0;
	j = 0;
	k = 0;
	int comp = 0;
	while (k < _new_list_len)
	{
		if (j >= num)
		{
			_new_xylist[k] = _xylist[i];
			_new_xylist[k + 1] = _xylist[i + 1];
			k += 2;
			i += 2;
		}
		else
		{
			comp = Util::compIIpair(_xylist[i], _xylist[i + 1], _list2[j], _list2[j + 1]);
			if (comp == -1)
			{
				_new_xylist[k] = _xylist[i];
				_new_xylist[k + 1] = _xylist[i + 1];
				k += 2;
				i += 2;
			}
			else if (comp == 1)
			{
				j += 2;
			}
			else // = 0
			{
				i += 2;
				j += 2;
			}
		}
	}

	delete[] _xylist;
	_xylist = _new_xylist;
	_list_len = _new_list_len;

	return true;
}



//==========================================================================================================



//for entity2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_entity2id(int _mode)
{
	return this->open(this->entity2id, KVstore::s_entity2id, _mode);
}

bool
KVstore::close_entity2id()
{
	if (this->entity2id == NULL)
	{
		return true;
	}
	this->entity2id->save();
	delete this->entity2id;
	this->entity2id = NULL;
	return true;
}

bool
KVstore::subIDByEntity(string _entity)
{
	return this->entity2id->remove(_entity.c_str(), _entity.length());
}

int
KVstore::getIDByEntity(string _entity)
{
	return this->getIDByStr(this->entity2id, _entity.c_str(), _entity.length());
}

bool
KVstore::setIDByEntity(string _entity, int _id)
{
	bool _set = this->addValueByKey(this->entity2id,
		_entity.c_str(), _entity.length(), _id);
	{
		if (!_set)
		{
			return false;
		}
	}

	return true;
}

//for id2entity
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_id2entity(int _mode)
{
	return this->open(this->id2entity, KVstore::s_id2entity, _mode);
}

bool
KVstore::close_id2entity()
{
	if (this->id2entity == NULL)
	{
		return true;
	}
	this->id2entity->save();
	delete this->id2entity;
	this->id2entity = NULL;
	return true;
}

bool
KVstore::subEntityByID(int _id)
{
	return this->id2entity->remove(_id);
}

string
KVstore::getEntityByID(int _id)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->id2entity, _id, _tmp, _len);
	{
		if (!_get)
		{
			return "";
		}
	}
	string _ret = string(_tmp);
	//delete[] _tmp;	DEBUG

	return _ret;
}

bool
KVstore::setEntityByID(int _id, string _entity) {
	bool _set = this->addValueByKey(this->id2entity,
		_id, _entity.c_str(), _entity.length());
	{
		if (!_set)
		{
			return false;
		}
	}

	return true;
}


//for predicate2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_predicate2id(int _mode)
{
	return this->open(this->predicate2id, KVstore::s_predicate2id, _mode);
}

bool
KVstore::close_predicate2id()
{
	if (this->predicate2id == NULL)
	{
		return true;
	}
	this->predicate2id->save();
	delete this->predicate2id;
	this->predicate2id = NULL;
	return true;
}

bool
KVstore::subIDByPredicate(string _predicate)
{
	return this->predicate2id->remove(_predicate.c_str(), _predicate.length());
}

int
KVstore::getIDByPredicate(string _predicate)
{
	return this->getIDByStr(this->predicate2id, _predicate.c_str(), _predicate.length());
}

bool
KVstore::setIDByPredicate(string _predicate, int _id) {
	bool _set = this->addValueByKey(this->predicate2id,
		_predicate.c_str(), _predicate.length(), _id);
	{
		if (!_set)
		{
			return false;
		}
	}

	return true;
}

//for id2predicate
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_id2predicate(int _mode)
{
	return this->open(this->id2predicate, KVstore::s_id2predicate, _mode);
}

bool
KVstore::close_id2predicate()
{
	if (this->id2predicate == NULL)
	{
		return true;
	}
	this->id2predicate->save();
	delete this->id2predicate;
	this->id2predicate = NULL;
	return true;
}

bool
KVstore::subPredicateByID(int _id)
{
	return this->id2predicate->remove(_id);
}

string
KVstore::getPredicateByID(int _id)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->id2predicate, _id, _tmp, _len);
	{
		if (!_get)
		{
			return "";
		}
	}
	string _ret = string(_tmp);
	//delete[] _tmp;

	return _ret;
}

bool
KVstore::setPredicateByID(int _id, string _predicate)
{
	bool _set = this->addValueByKey(this->id2predicate,
		_id, _predicate.c_str(), _predicate.length());
	{
		if (!_set)
		{
			return false;
		}
	}

	return true;
}


//for literal2id
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_literal2id(int _mode)
{
	return this->open(this->literal2id, KVstore::s_literal2id, _mode);
}

bool
KVstore::close_literal2id()
{
	if (this->literal2id == NULL)
	{
		return true;
	}
	this->literal2id->save();
	delete this->literal2id;
	this->literal2id = NULL;
	return true;
}

bool
KVstore::subIDByLiteral(string _literal)
{
	return this->literal2id->remove(_literal.c_str(), _literal.length());
}

int
KVstore::getIDByLiteral(string _literal)
{
	return this->getIDByStr(this->literal2id, _literal.c_str(), _literal.length());
}

bool
KVstore::setIDByLiteral(string _literal, int _id)
{
	bool _set = this->addValueByKey(this->literal2id,
		_literal.c_str(), _literal.length(), _id);
	{
		if (!_set)
		{
			return false;
		}
	}

	return true;
}

//for id2literal
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_id2literal(int _mode)
{
	return this->open(this->id2literal, KVstore::s_id2literal, _mode);
}

bool
KVstore::close_id2literal()
{
	if (this->id2literal == NULL)
	{
		return true;
	}
	this->id2literal->save();
	delete this->id2literal;
	this->id2literal = NULL;
	return true;
}

bool
KVstore::subLiteralByID(int _id)
{
	return this->id2literal->remove(_id);
}

string
KVstore::getLiteralByID(int _id)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->id2literal, _id, _tmp, _len);
	{
		if (!_get)
		{
			return "";
		}
	}
	string _ret = string(_tmp);
	//	delete[] _tmp;

	return _ret;
}

bool
KVstore::setLiteralByID(int _id, string _literal)
{
	bool _set = this->addValueByKey(this->id2literal, _id, _literal.c_str(), _literal.length());
	{
		if (!_set)
		{
			return false;
		}
	}

	return true;
}

//for subID2objIDlist
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_subID2objIDlist(int _mode)
{
	return this->open(this->subID2objIDlist, KVstore::s_sID2oIDlist, _mode);
}

bool
KVstore::close_subID2objIDlist()
{
	if (this->subID2objIDlist == NULL)
	{
		return true;
	}
	this->subID2objIDlist->save();
	delete this->subID2objIDlist;
	this->subID2objIDlist = NULL;
	return true;
}

bool
KVstore::getobjIDlistBysubID(int _subid, int*& _objidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2objIDlist, _subid, _tmp, _len);
	{
		if (!_get)
		{
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_objidlist = new int[_list_len];
		memcpy((char*)_objidlist, _tmp, sizeof(int)*_list_len);
	}
	//	delete[] _tmp;

	return true;
}

bool
KVstore::addobjIDlistBysubID(int _subid, const int* _objidlist, int _list_len)
{
	return this->addValueByKey
		(this->subID2objIDlist, _subid, (char*)_objidlist, _list_len * sizeof(int));
}

bool
KVstore::setobjIDlistBysubID(int _subid, const int* _objidlist, int _list_len)
{
	return this->setValueByKey
		(this->subID2objIDlist, _subid, (char*)_objidlist, _list_len * sizeof(int));
}

//for objID2subIDlist
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_objID2subIDlist(int _mode)
{
	return this->open(this->objID2subIDlist, KVstore::s_oID2sIDlist, _mode);
}

bool
KVstore::close_objID2subIDlist()
{
	if (this->objID2subIDlist == NULL)
	{
		return true;
	}
	this->objID2subIDlist->save();
	delete this->objID2subIDlist;
	this->objID2subIDlist = NULL;
	return true;
}

bool
KVstore::getsubIDlistByobjID(int _objid, int*& _subidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2subIDlist, _objid, _tmp, _len);
	{
		if (!_get)
		{
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_subidlist = new int[_list_len];
		memcpy((char*)_subidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addsubIDlistByobjID(int _objid, const int* _subidlist, int _list_len)
{
	return this->addValueByKey
		(this->objID2subIDlist, _objid, (char*)_subidlist, _list_len * sizeof(int));
}

bool
KVstore::setsubIDlistByobjID(int _objid, const int* _subidlist, int _list_len)
{
	return this->setValueByKey
		(this->objID2subIDlist, _objid, (char*)_subidlist, _list_len * sizeof(int));
}

//for subID&preID2objIDlist
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_subIDpreID2objIDlist(int _mode)
{
	return this->open(this->subIDpreID2objIDlist, KVstore::s_sIDpID2oIDlist, _mode);
}

bool
KVstore::close_subIDpreID2objIDlist()
{
	if (this->subIDpreID2objIDlist == NULL)
	{
		return true;
	}
	this->subIDpreID2objIDlist->save();
	delete this->subIDpreID2objIDlist;
	this->subIDpreID2objIDlist = NULL;
	return true;
}

bool
KVstore::getobjIDlistBysubIDpreID(int _subid, int _preid, int*& _objidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	int* _sp = new int[2];
	_sp[0] = _subid;
	_sp[1] = _preid;
	bool _get = this->getValueByKey(this->subIDpreID2objIDlist, (char*)_sp, sizeof(int) * 2, _tmp, _len);
	delete[] _sp;
	{
		if (!_get)
		{
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_objidlist = new int[_list_len];
		memcpy((char*)_objidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addobjIDlistBysubIDpreID(int _subid, int _preid, const int* _objidlist, int _list_len)
{
	int* _sp = new int[2];
	_sp[0] = _subid;
	_sp[1] = _preid;

	bool _set = this->addValueByKey
		(this->subIDpreID2objIDlist, (char*)_sp, sizeof(int) * 2, (char*)_objidlist, _list_len * sizeof(int));

	//if(_set)
	//cout<<"set sp2o true"<<endl;
	//else
	//cout<<"set sp2o false"<<endl;

	delete[] _sp;

	return _set;
}

bool
KVstore::setobjIDlistBysubIDpreID(int _subid, int _preid, const int* _objidlist, int _list_len)
{
	int* _sp = new int[2];
	_sp[0] = _subid;
	_sp[1] = _preid;

	bool _set = this->setValueByKey
		(this->subIDpreID2objIDlist, (char*)_sp, sizeof(int) * 2, (char*)_objidlist, _list_len * sizeof(int));

	//if(_set)
	//cout<<"set sp2o true"<<endl;
	//else
	//cout<<"set sp2o false"<<endl;

	delete[] _sp;

	return _set;
}

//for objID&preID2subIDlist
//_mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_objIDpreID2subIDlist(int _mode)
{
	return this->open(this->objIDpreID2subIDlist, KVstore::s_oIDpID2sIDlist, _mode);
}

bool
KVstore::close_objIDpreID2subIDlist()
{
	if (this->objIDpreID2subIDlist == NULL)
	{
		return true;
	}
	this->objIDpreID2subIDlist->save();
	delete this->objIDpreID2subIDlist;
	this->objIDpreID2subIDlist = NULL;
	return true;
}

bool
KVstore::getsubIDlistByobjIDpreID(int _objid, int _preid, int*& _subidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	int* _sp = new int[2];
	_sp[0] = _objid;
	_sp[1] = _preid;

	bool _get = this->getValueByKey(this->objIDpreID2subIDlist, (char*)_sp, sizeof(int) * 2, _tmp, _len);

	delete[] _sp;
	{
		if (!_get)
		{
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_subidlist = new int[_list_len];
		memcpy((char*)_subidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addsubIDlistByobjIDpreID(int _objid, int _preid, const int* _subidlist, int _list_len)
{
	int* _sp = new int[2];
	_sp[0] = _objid;
	_sp[1] = _preid;

	bool _set = this->addValueByKey
		(this->objIDpreID2subIDlist, (char*)_sp, sizeof(int) * 2, (char*)_subidlist, _list_len * sizeof(int));

	delete[] _sp;

	return _set;
}

bool
KVstore::setsubIDlistByobjIDpreID(int _objid, int _preid, const int* _subidlist, int _list_len)
{
	int* _sp = new int[2];
	_sp[0] = _objid;
	_sp[1] = _preid;

	bool _set = this->setValueByKey
		(this->objIDpreID2subIDlist, (char*)_sp, sizeof(int) * 2, (char*)_subidlist, _list_len * sizeof(int));
	//if(_set)
	//{
		//cout<<"tag 10"<<endl;
	//}
	//else
	//{
		//cout<<"tage 11"<<endl;
	//}

	delete[] _sp;

	return _set;
}

//for subID 2 preID&objIDlist 
bool
KVstore::open_subID2preIDobjIDlist(int _mode)
{
	return this->open(this->subID2preIDobjIDlist, KVstore::s_sID2pIDoIDlist, _mode);
}

bool
KVstore::close_subID2preIDobjIDlist()
{
	if (this->subID2preIDobjIDlist == NULL)
	{
		return true;
	}
	this->subID2preIDobjIDlist->save();
	delete this->subID2preIDobjIDlist;
	this->subID2preIDobjIDlist = NULL;
	return true;
}

bool
KVstore::getpreIDobjIDlistBysubID(int _subid, int*& _preid_objidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2preIDobjIDlist, _subid, _tmp, _len);
	{
		if (!_get)
		{
			_preid_objidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_preid_objidlist = new int[_list_len];
		memcpy((char*)_preid_objidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addpreIDobjIDlistBysubID(int _subid, const int* _preid_objidlist, int _list_len)
{
	return this->addValueByKey
		(this->subID2preIDobjIDlist, _subid, (char*)_preid_objidlist, _list_len * sizeof(int));
}

//NOTICE: the list is ordered by each two (pre, sub), and the whole is viewed as a string
bool
KVstore::setpreIDobjIDlistBysubID(int _subid, const int* _preid_objidlist, int _list_len)
{
	return this->setValueByKey
		(this->subID2preIDobjIDlist, _subid, (char*)_preid_objidlist, _list_len * sizeof(int));
}

//for objID 2 preID&subIDlist 
bool
KVstore::open_objID2preIDsubIDlist(int _mode)
{
	return this->open(this->objID2preIDsubIDlist, KVstore::s_oID2pIDsIDlist, _mode);
}

bool
KVstore::close_objID2preIDsubIDlist()
{
	if (this->objID2preIDsubIDlist == NULL)
	{
		return true;
	}
	this->objID2preIDsubIDlist->save();
	delete this->objID2preIDsubIDlist;
	this->objID2preIDsubIDlist = NULL;
	return true;
}

bool
KVstore::getpreIDsubIDlistByobjID(int _objid, int*& _preid_subidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2preIDsubIDlist, _objid, _tmp, _len);
	{
		if (!_get)
		{
			_preid_subidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_preid_subidlist = new int[_list_len];
		memcpy((char*)_preid_subidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addpreIDsubIDlistByobjID(int _objid, const int* _preid_subidlist, int _list_len)
{
	return this->addValueByKey
		(this->objID2preIDsubIDlist, _objid, (char*)_preid_subidlist, _list_len * sizeof(int));
}

bool
KVstore::setpreIDsubIDlistByobjID(int _objid, const int* _preid_subidlist, int _list_len)
{
	return this->setValueByKey
		(this->objID2preIDsubIDlist, _objid, (char*)_preid_subidlist, _list_len * sizeof(int));
}

//for subID 2 preIDlist
bool
KVstore::open_subID2preIDlist(int _mode)
{
	return this->open(this->subID2preIDlist, KVstore::s_sID2pIDlist, _mode);
}

bool
KVstore::close_subID2preIDlist()
{
	if (this->subID2preIDlist == NULL)
	{
		return true;
	}
	this->subID2preIDlist->save();
	delete this->subID2preIDlist;
	this->subID2preIDlist = NULL;
	return true;
}

bool
KVstore::getpreIDlistBysubID(int _subid, int*& _preidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->subID2preIDlist, _subid, _tmp, _len);
	{
		if (!_get)
		{
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_preidlist = new int[_list_len];
		memcpy((char*)_preidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addpreIDlistBysubID(int _subid, const int* _preidlist, int _list_len)
{
	return this->addValueByKey
		(this->subID2preIDlist, _subid, (char*)_preidlist, _list_len * sizeof(int));
}

bool
KVstore::setpreIDlistBysubID(int _subid, const int* _preidlist, int _list_len)
{
	return this->setValueByKey
		(this->subID2preIDlist, _subid, (char*)_preidlist, _list_len * sizeof(int));
}

//for preID 2 subIDlist
bool
KVstore::open_preID2subIDlist(int _mode)
{
	return this->open(this->preID2subIDlist, KVstore::s_pID2sIDlist, _mode);
}

bool
KVstore::close_preID2subIDlist()
{
	if (this->preID2subIDlist == NULL)
	{
		return true;
	}
	this->preID2subIDlist->save();
	delete this->preID2subIDlist;
	this->preID2subIDlist = NULL;
	return true;
}

bool
KVstore::getsubIDlistBypreID(int _preid, int*& _subidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->preID2subIDlist, _preid, _tmp, _len);
	{
		if (!_get)
		{
			_subidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_subidlist = new int[_list_len];
		memcpy((char*)_subidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addsubIDlistBypreID(int _preid, const int* _subidlist, int _list_len)
{
	return this->addValueByKey
		(this->preID2subIDlist, _preid, (char*)_subidlist, _list_len * sizeof(int));
}

bool
KVstore::setsubIDlistBypreID(int _preid, const int* _subidlist, int _list_len)
{
	return this->setValueByKey
		(this->preID2subIDlist, _preid, (char*)_subidlist, _list_len * sizeof(int));
}

//for objID 2 preIDlist
bool
KVstore::open_objID2preIDlist(int _mode)
{
	return this->open(this->objID2preIDlist, KVstore::s_oID2pIDlist, _mode);
}

bool
KVstore::close_objID2preIDlist()
{
	if (this->objID2preIDlist == NULL)
	{
		return true;
	}
	this->objID2preIDlist->save();
	delete this->objID2preIDlist;
	this->objID2preIDlist = NULL;
	return true;
}

bool
KVstore::getpreIDlistByobjID(int _objid, int*& _preidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->objID2preIDlist, _objid, _tmp, _len);
	{
		if (!_get)
		{
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_preidlist = new int[_list_len];
		memcpy((char*)_preidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addpreIDlistByobjID(int _objid, const int* _preidlist, int _list_len)
{
	return this->addValueByKey
		(this->objID2preIDlist, _objid, (char*)_preidlist, _list_len * sizeof(int));
}

bool
KVstore::setpreIDlistByobjID(int _objid, const int* _preidlist, int _list_len)
{
	return this->setValueByKey
		(this->objID2preIDlist, _objid, (char*)_preidlist, _list_len * sizeof(int));
}

//for preID 2 objIDlist
bool
KVstore::open_preID2objIDlist(int _mode)
{
	return this->open(this->preID2objIDlist, KVstore::s_pID2oIDlist, _mode);
}

bool
KVstore::close_preID2objIDlist()
{
	if (this->preID2objIDlist == NULL)
	{
		return true;
	}
	this->preID2objIDlist->save();
	delete this->preID2objIDlist;
	this->preID2objIDlist = NULL;
	return true;
}

bool
KVstore::getobjIDlistBypreID(int _preid, int*& _objidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->preID2objIDlist, _preid, _tmp, _len);
	{
		if (!_get)
		{
			_objidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_objidlist = new int[_list_len];
		memcpy((char*)_objidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addobjIDlistBypreID(int _preid, const int* _objidlist, int _list_len)
{
	return this->addValueByKey
		(this->preID2objIDlist, _preid, (char*)_objidlist, _list_len * sizeof(int));
}

bool
KVstore::setobjIDlistBypreID(int _preid, const int* _objidlist, int _list_len)
{
	return this->setValueByKey
		(this->preID2objIDlist, _preid, (char*)_objidlist, _list_len * sizeof(int));
}

//for subID&objID2preIDlist  _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
bool
KVstore::open_subIDobjID2preIDlist(int _mode)
{
	return this->open(this->subIDobjID2preIDlist, KVstore::s_sIDoID2pIDlist, _mode);
}

bool
KVstore::getpreIDlistBysubIDobjID(int _subid, int _objid, int*& _preidlist, int& _list_len)
{
#ifdef SO2P
	char* _tmp = NULL;
	int _len = 0;
	int* _sp = new int[2];
	_sp[0] = _subid;
	_sp[1] = _objid;
	bool _get = this->getValueByKey(this->subIDobjID2preIDlist, (char*)_sp, sizeof(int) * 2, _tmp, _len);
	delete[] _sp;
	{
		if (!_get)
		{
			_preidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_preidlist = new int[_list_len];
		memcpy((char*)_preidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;
#else
	//WARN+TODO:this maybe not correct
	//s p o2 and s2 p o
	//s2p intersect o2p can get p, but this is not right
	int *list1 = NULL, *list2 = NULL;
	int len1 = 0, len2 = 0;
	this->getpreIDlistBysubID(_subid, list1, len1);
	this->getpreIDlistByobjID(_objid, list2, len2);
	Util::intersect(_preidlist, _list_len, list1, len1, list2, len2);
#endif

	return true;
}

bool
KVstore::addpreIDlistBysubIDobjID(int _subid, int _objid, const int* _preidlist, int _list_len)
{
	int* _sp = new int[2];
	_sp[0] = _subid;
	_sp[1] = _objid;

	bool _set = this->addValueByKey
		(this->subIDobjID2preIDlist, (char*)_sp, sizeof(int) * 2, (char*)_preidlist, _list_len * sizeof(int));

	delete[] _sp;

	return _set;
}

//NOTICE:this is not used because so2p is implemented by s2p and o2p now
bool
KVstore::setpreIDlistBysubIDobjID(int _subid, int _objid, const int* _preidlist, int _list_len)
{
	int* _sp = new int[2];
	_sp[0] = _subid;
	_sp[1] = _objid;

	bool _set = this->setValueByKey
		(this->subIDobjID2preIDlist, (char*)_sp, sizeof(int) * 2, (char*)_preidlist, _list_len * sizeof(int));

	delete[] _sp;

	return _set;
}

//preID2subID&objIDlist
bool
KVstore::open_preID2subIDobjIDlist(int _mode)
{
	return this->open(this->preID2subIDobjIDlist, KVstore::s_pID2sIDoIDlist, _mode);
}

bool
KVstore::close_preID2subIDobjIDlist()
{
	if (this->preID2subIDobjIDlist == NULL)
	{
		return true;
	}
	this->preID2subIDobjIDlist->save();
	delete this->preID2subIDobjIDlist;
	this->preID2subIDobjIDlist = NULL;
	return true;
}

bool
KVstore::getsubIDobjIDlistBypreID(int _preid, int*& _subid_objidlist, int& _list_len)
{
	char* _tmp = NULL;
	int _len = 0;
	bool _get = this->getValueByKey(this->preID2subIDobjIDlist, _preid, _tmp, _len);
	{
		if (!_get)
		{
			_subid_objidlist = NULL;
			_list_len = 0;
			return false;
		}
	}
	{
		_list_len = _len / sizeof(int);
		_subid_objidlist = new int[_list_len];
		memcpy((char*)_subid_objidlist, _tmp, sizeof(int)*_list_len);
	}
	//delete[] _tmp;

	return true;
}

bool
KVstore::addsubIDobjIDlistBypreID(int _preid, const int* _subid_objidlist, int _list_len)
{
	return this->addValueByKey
		(this->preID2subIDobjIDlist, _preid, (char*)_subid_objidlist, _list_len * sizeof(int));
}

bool
KVstore::setsubIDobjIDlistBypreID(int _preid, const int* _subid_objidlist, int _list_len)
{
	return this->setValueByKey
		(this->preID2subIDobjIDlist, _preid, (char*)_subid_objidlist, _list_len * sizeof(int));
}

//====================================================================================================================================
//Below are for s/p/o2tripleNum, wasteful to implement unique trees
//However, if using original tree to get num, sometimes maybe very costly?
//NOTICE:not change to II_TREE now

//preID2num
bool
KVstore::open_preID2num(int _mode)
{
	return this->open(this->preID2num, KVstore::s_pID2num, _mode);
}

int
KVstore::getNumBypreID(int _preid)
{
	//TODO
	int tmp = 0;
	//bool _get = this->getValueByKey(this->preID2num, _preid, &tmp);
	//if(!_get)
	//{
	//return 0;
	//}

	//return _tmp[0];
	return tmp;
}

bool
KVstore::setNumBypreID(int _preid, int _tripleNum)
{
	//TODO
	//return this->setValueByKey(this->preID2num, (char*)&_preid, sizeof(int), (char*)&_tripleNum, sizeof(int));
	return false;
}

//subIDpreID2num
bool
KVstore::open_subIDpreID2num(int _mode)
{
	return this->open(this->subIDpreID2num, KVstore::s_sIDpID2num, _mode);
}

int
KVstore::getNumBysubIDpreID(int _subid, int _preid)
{
	//TODO
	//int* _key = new int[2];
	//_key[0] = _subid;
	//_key[1] = _preid;
	//char* _tmp = NULL;
	//int _len = 0;
	//bool _get = this->getValueByKey(this->subIDpreID2num, (char*)_key, 2 * sizeof(int), _tmp, _len);

	//if(!_get)
	//{
	//	return 0;
	//}

	//delete[] _key;
	//return _tmp[0];
	return 0;
}

bool
KVstore::setNumBysubIDpreID(int _subid, int _preid, int _tripleNum)
{
	//TODO
	//int* _key = new int[2];
	//_key[0] = _subid;
	//_key[1] = _preid;
	//bool ret = this->setValueByKey(this->subIDpreID2num, (char*)_key, 2 * sizeof(int), (char*)&_tripleNum, sizeof(int));

	//delete[] _key;
	//return ret;
	return true;
}

//objIDpreID2num
bool
KVstore::open_objIDpreID2num(int _mode)
{
	return this->open(this->objIDpreID2num, KVstore::s_oIDpID2num, _mode);
}

int
KVstore::getNumByobjIDpreID(int _objid, int _preid)
{
	//TODO
	//int* _key = new int[2];
	//_key[0] = _objid;
	//_key[1] = _preid;
	//char* _tmp = NULL;
	//int _len = 0;
	//bool _get = this->getValueByKey(this->objIDpreID2num, (char*)_key, 2 * sizeof(int), _tmp, _len);

	//if(!_get)
	//{
	//	return 0;
	//}

	//delete[] _key;
	//return _tmp[0];
	return 0;
}

bool
KVstore::setNumByobjIDpreID(int _objid, int _preid, int _tripleNum)
{
	//TODO
	//int* _key = new int[2];
	//_key[0] = _objid;
	//_key[1] = _preid;
	//bool ret = this->setValueByKey(this->objIDpreID2num, (char*)_key, 2 * sizeof(int), (char*)&_tripleNum, sizeof(int));

	//delete[] _key;
	//return ret;
	return true;
}

//==========================================================================================================================


//set the store_path as the root dir of this KVstore
//initial all Tree pointer as NULL
KVstore::KVstore(string _store_path) {
	this->store_path = _store_path;

	this->entity2id = NULL;
	this->id2entity = NULL;

	this->predicate2id = NULL;
	this->id2predicate = NULL;

	this->literal2id = NULL;
	this->id2literal = NULL;

	this->objID2subIDlist = NULL;
	this->subID2objIDlist = NULL;

	this->objIDpreID2subIDlist = NULL;
	this->subIDpreID2objIDlist = NULL;

	this->subID2preIDobjIDlist = NULL;
	this->objID2preIDsubIDlist = NULL;

	this->subID2preIDlist = NULL;
	this->preID2subIDlist = NULL;

	this->objID2preIDlist = NULL;
	this->preID2objIDlist = NULL;

	this->subIDobjID2preIDlist = NULL;
	this->preID2subIDobjIDlist = NULL;

	this->preID2num = NULL;
	this->subIDpreID2num = NULL;
	this->objIDpreID2num = NULL;
}

//release all the memory used in this KVstore
//before destruction
KVstore::~KVstore()
{
	this->flush();
	this->release();
}

void
KVstore::release()
{
	delete this->entity2id;
	this->entity2id = NULL;
	delete this->id2entity;
	this->id2entity = NULL;

	delete this->literal2id;
	this->literal2id = NULL;
	delete this->id2literal;
	this->id2literal = NULL;

	delete this->predicate2id;
	this->predicate2id = NULL;
	delete this->id2predicate;
	this->id2predicate = NULL;

	delete this->objID2subIDlist;
	this->objID2subIDlist = NULL;
	delete this->subID2objIDlist;
	this->subID2objIDlist = NULL;

	delete this->objIDpreID2subIDlist;
	this->objIDpreID2subIDlist = NULL;
	delete this->subIDpreID2objIDlist;
	this->subIDpreID2objIDlist = NULL;

	delete this->subID2preIDobjIDlist;
	this->subID2preIDobjIDlist = NULL;
	delete this->objID2preIDsubIDlist;
	this->objID2preIDsubIDlist = NULL;

	delete this->subID2preIDlist;
	this->subID2preIDlist = NULL;
	delete this->preID2subIDlist;
	this->preID2subIDlist = NULL;
	delete this->objID2preIDlist;
	this->objID2preIDlist = NULL;
	delete this->preID2objIDlist;
	this->preID2objIDlist = NULL;
	delete this->subIDobjID2preIDlist;
	this->subIDobjID2preIDlist = NULL;
	delete this->preID2subIDobjIDlist;
	this->preID2subIDobjIDlist = NULL;

	//delete this->preID2num;
	//delete this->subIDpreID2num;
	//delete this->objIDpreID2num;
}

//just flush all modified part into disk
//will not release any memory at all
//any Tree pointer that is null or
//has not been modified will do nothing
void
KVstore::flush()
{
	this->flush(this->entity2id);
	this->flush(this->id2entity);

	this->flush(this->literal2id);
	this->flush(this->id2literal);

	this->flush(this->predicate2id);
	this->flush(this->id2predicate);

	this->flush(this->objID2subIDlist);
	this->flush(this->subID2objIDlist);

	this->flush(this->objIDpreID2subIDlist);
	this->flush(this->subIDpreID2objIDlist);

	this->flush(this->subID2preIDobjIDlist);
	this->flush(this->objID2preIDsubIDlist);

	this->flush(this->subID2preIDlist);
	this->flush(this->preID2subIDlist);
	this->flush(this->objID2preIDlist);
	this->flush(this->preID2objIDlist);
	this->flush(this->subIDobjID2preIDlist);
	this->flush(preID2subIDobjIDlist);

	//this->flush(this->preID2num);
	//this->flush(this->subIDpreID2num);
	//this->flush(this->objIDpreID2num);
}

//================================================================================================================

void
KVstore::flush(Tree* _p_btree)
{
	if (_p_btree != NULL)
	{
		_p_btree->save();
	}
}

void
KVstore::flush(SITree* _p_btree)
{
	if (_p_btree != NULL)
	{
		_p_btree->save();
	}
}

void
KVstore::flush(ISTree* _p_btree)
{
	if (_p_btree != NULL)
	{
		_p_btree->save();
	}
}

//================================================================================================================

void
KVstore::open()
{
#ifdef DEBUG
	//cout << "open KVstore" << endl;
#endif

	this->open(this->entity2id, KVstore::s_entity2id, KVstore::READ_WRITE_MODE);
	this->open(this->id2entity, KVstore::s_id2entity, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"entity-id opened"<<endl;
#endif

	this->open(this->literal2id, KVstore::s_literal2id, KVstore::READ_WRITE_MODE);
	this->open(this->id2literal, KVstore::s_id2literal, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"literal-id opened"<<endl;
#endif

	this->open(this->predicate2id, KVstore::s_predicate2id, KVstore::READ_WRITE_MODE);
	this->open(this->id2predicate, KVstore::s_id2predicate, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"predicate-id opened"<<endl;
#endif

	this->open(this->objID2subIDlist, KVstore::s_oID2sIDlist, KVstore::READ_WRITE_MODE);
	this->open(this->subID2objIDlist, KVstore::s_sID2oIDlist, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"o-s opened"<<endl;
#endif

	this->open(this->objIDpreID2subIDlist, KVstore::s_oIDpID2sIDlist, KVstore::READ_WRITE_MODE);
	this->open(this->subIDpreID2objIDlist, KVstore::s_sIDpID2oIDlist, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"op-s opened"<<endl;
#endif

	this->open(this->subID2preIDobjIDlist, KVstore::s_sID2pIDoIDlist, KVstore::READ_WRITE_MODE);
	this->open(this->objID2preIDsubIDlist, KVstore::s_oID2pIDsIDlist, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"s-po opened"<<endl;
#endif

	this->open(this->subID2preIDlist, KVstore::s_sID2pIDlist, KVstore::READ_WRITE_MODE);
	this->open(this->preID2subIDlist, KVstore::s_pID2sIDlist, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"s-p opened"<<endl;
#endif

	this->open(this->objID2preIDlist, KVstore::s_oID2pIDlist, KVstore::READ_WRITE_MODE);
	this->open(this->preID2objIDlist, KVstore::s_pID2oIDlist, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"o-p opened"<<endl;
#endif

#ifdef SO2P
	this->open(this->subIDobjID2preIDlist, KVstore::s_sIDoID2pIDlist, KVstore::READ_WRITE_MODE);
#endif

	this->open(this->preID2subIDobjIDlist, KVstore::s_pID2sIDoIDlist, KVstore::READ_WRITE_MODE);
#ifdef DEBUG
	//cout<<"p2so opened"<<endl;
#endif

	//this->open(this->preID2num, KVstore::s_pID2num, KVstore::READ_WRITE_MODE);
	//this->open(this->subIDpreID2num, KVstore::s_sIDpID2num, KVstore::READ_WRITE_MODE);
	//this->open(this->objIDpreID2num, KVstore::s_oIDpID2num, KVstore::READ_WRITE_MODE);
}

//Open a btree according the mode
//CREATE_MODE: 		build a new btree and delete if exist
//READ_WRITE_MODE: 	open a btree, btree must exist
bool
KVstore::open(Tree*& _p_btree, string _tree_name, int _mode)
{
	if (_p_btree != NULL)
	{
		return false;
	}

	string smode;
	if (_mode == KVstore::CREATE_MODE)
	{
		smode = "build";
	}
	else if (_mode == KVstore::READ_WRITE_MODE)
	{
		smode = "open";
	}
	else
	{
		cout << "bug in open mode of : " << _tree_name << " with mode=" << _mode << endl;
		return false;
	}

	_p_btree = new Tree(this->store_path, _tree_name, smode);
	return true;
}

bool
KVstore::open(SITree*& _p_btree, string _tree_name, int _mode)
{
	if (_p_btree != NULL)
	{
		return false;
	}

	string smode;
	if (_mode == KVstore::CREATE_MODE)
	{
		smode = "build";
	}
	else if (_mode == KVstore::READ_WRITE_MODE)
	{
		smode = "open";
	}
	else
	{
		cout << "bug in open mode of : " << _tree_name << " with mode=" << _mode << endl;
		return false;
	}

	_p_btree = new SITree(this->store_path, _tree_name, smode);
	return true;
}

bool
KVstore::open(ISTree*& _p_btree, string _tree_name, int _mode)
{
	if (_p_btree != NULL)
	{
		return false;
	}

	string smode;
	if (_mode == KVstore::CREATE_MODE)
	{
		smode = "build";
	}
	else if (_mode == KVstore::READ_WRITE_MODE)
	{
		smode = "open";
	}
	else
	{
		cout << "bug in open mode of : " << _tree_name << " with mode=" << _mode << endl;
		return false;
	}

	_p_btree = new ISTree(this->store_path, _tree_name, smode);
	return true;
}

//==========================================================================================================

bool
KVstore::addValueByKey(Tree* _p_btree, const char* _key, int _klen, const char* _val, int _vlen)
{
	return _p_btree->insert(_key, _klen, _val, _vlen);
}

bool
KVstore::addValueByKey(SITree* _p_btree, const char* _key, int _klen, int _val)
{
	return _p_btree->insert(_key, _klen, _val);
}

bool
KVstore::addValueByKey(ISTree* _p_btree, int _key, const char* _val, int _vlen)
{
	return _p_btree->insert(_key, _val, _vlen);
}

//==========================================================================================================

bool
KVstore::setValueByKey(Tree* _p_btree, const char* _key, int _klen, const char* _val, int _vlen)
{
	return _p_btree->modify(_key, _klen, _val, _vlen);
}

bool
KVstore::setValueByKey(SITree* _p_btree, const char* _key, int _klen, int _val)
{
	return _p_btree->modify(_key, _klen, _val);
}

bool
KVstore::setValueByKey(ISTree* _p_btree, int _key, const char* _val, int _vlen)
{
	return _p_btree->modify(_key, _val, _vlen);
}

//==========================================================================================================

bool
KVstore::getValueByKey(Tree* _p_btree, const char* _key, int _klen, char*& _val, int& _vlen)
{
	return _p_btree->search(_key, _klen, _val, _vlen);
}

bool
KVstore::getValueByKey(SITree* _p_btree, const char* _key, int _klen, int* _val)
{
	return _p_btree->search(_key, _klen, _val);
}

bool
KVstore::getValueByKey(ISTree* _p_btree, int _key, char*& _val, int& _vlen)
{
	return _p_btree->search(_key, _val, _vlen);
}

//==========================================================================================================

int
KVstore::getIDByStr(SITree* _p_btree, const char* _key, int _klen)
{
	int val = 0;
	bool ret = _p_btree->search(_key, _klen, &val);
	if (!ret)	//QUERY: if need to check vlen?
	{
		return -1;
	}

	return val;
}

//==========================================================================================================

bool
KVstore::removeKey(Tree* _p_btree, const char* _key, int _klen)
{
	return _p_btree->remove(_key, _klen);
}

bool
KVstore::removeKey(SITree* _p_btree, const char* _key, int _klen)
{
	return _p_btree->remove(_key, _klen);
}

bool
KVstore::removeKey(ISTree* _p_btree, int _key)
{
	return _p_btree->remove(_key);
}

//==========================================================================================================





string KVstore::s_entity2id = "s_entity2id";
string KVstore::s_id2entity = "s_id2entity";

string KVstore::s_predicate2id = "s_predicate2id";
string KVstore::s_id2predicate = "s_id2predicate";

string KVstore::s_literal2id = "s_literal2id";
string KVstore::s_id2literal = "s_id2literal";


string KVstore::s_sID2oIDlist = "s_sID2oIDlist";
string KVstore::s_oID2sIDlist = "s_oID2sIDlist";

string KVstore::s_sIDpID2oIDlist = "s_sIDpID2oIDlist";
string KVstore::s_oIDpID2sIDlist = "s_oIDpID2sIDlist";

string KVstore::s_sID2pIDoIDlist = "s_sID2pIDoIDlist";
string KVstore::s_oID2pIDsIDlist = "s_oID2pIDsIDlist";

string KVstore::s_sID2pIDlist = "s_sID2pIDlist";
string KVstore::s_pID2sIDlist = "s_pID2sIDlist";

string KVstore::s_oID2pIDlist = "s_oID2pIDlist";
string KVstore::s_pID2oIDlist = "s_pID2oIDlist";

string KVstore::s_sIDoID2pIDlist = "s_sIDoID2pIDlist";
string KVstore::s_pID2sIDoIDlist = "s_pID2sIDoIDlist";

string KVstore::s_pID2num = "s_pID2num";
string KVstore::s_sIDpID2num = "s_sIDpID2num";
string KVstore::s_oIDpID2num = "s_oIDpID2num";
