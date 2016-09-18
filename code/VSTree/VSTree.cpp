/*
 * VSTREE.cpp
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#include"VSTree.h"
#include<stdio.h>
#include<queue>
#include"../Database/Database.h"
#include"../Signature/Signature.h"
#include<algorithm>
#include<vector>
#include<iostream>
using namespace std;

string VSTree::tree_file_foler_path;
string VSTree::tree_node_file_path;  // to be determine
string VSTree::tree_info_file_path;  // to be determine

VSTree::VSTree(std::string _store_path)
{
    this->height = 0;
    this->node_num = 0;
    this->entry_num = 0;
    this->root_file_line = 0;
    this->entry_buffer = NULL;
    this->node_buffer = NULL;
    /* set the store path */
    VSTree::tree_file_foler_path = _store_path;
    VSTree::tree_node_file_path = VSTree::tree_file_foler_path + "/tree_node_file.dat";
    VSTree::tree_info_file_path = VSTree::tree_file_foler_path + "/tree_info_file.dat";
}

VSTree::~VSTree()
{
    delete this->node_buffer;
    delete this->entry_buffer;
}

int VSTree::getHeight()const
{
    return this->height;
}

/* get the tree's root node pointer. */
VNode* VSTree::getRoot()
{
    return (this->node_buffer)->get(this->root_file_line);
}

/* get the node pointer by its file line. */
VNode* VSTree::getNode(int _line)
{
    if (_line >= this->node_num)
    {
        cerr << "error, the parameter:_line is bigger than the tree node_num. @VSTree::getNode" << endl;
        return NULL;
    }

    return this->node_buffer->get(_line);
}

/* retrieve candidate result set by the var_sig in the _query. */
void VSTree::retrieve(SPARQLquery& _query)
{
	Database::log("IN retrieve");

	//debug
//	{
//	    VNode* temp_ptr = this->getLeafNodeByEntityID(473738);
//	    stringstream _ss;
//
//	    for (int i=0;i<temp_ptr->getChildNum();i++)
//	        if (temp_ptr->getChildEntry(i).getEntityId() == 473738)
//	        {
//	            _ss << "entity id=473738 entry sig:" << endl;
//	            _ss << "entity id=473738 leaf node line: " << temp_ptr->getFileLine() << endl;
//	            _ss << Signature::BitSet2str(temp_ptr->getChildEntry(i).getEntitySig().entityBitSet) << endl;
//	            break;
//	        }
//
//	    _ss << "leaf node sig:" << endl;
//	    _ss << Signature::BitSet2str(temp_ptr->getEntry().getEntitySig().entityBitSet) << endl;
//
//	    temp_ptr = temp_ptr->getFather(*(this->node_buffer));
//	    while (temp_ptr != NULL)
//	    {
//	        _ss << "line=" << temp_ptr->getFileLine() << endl;
//	        _ss << Signature::BitSet2str(temp_ptr->getEntry().getEntitySig().entityBitSet) << endl;
//	        temp_ptr = temp_ptr->getFather(*(this->node_buffer));
//	    }
//	    Database::log(_ss.str());
//	}

    vector<BasicQuery*>& queryList = _query.getBasicQueryVec();
    // enumerate each BasicQuery and retrieve their variables' mapping entity in the VSTree.
    vector<BasicQuery*>::iterator iter=queryList.begin();
    for(; iter != queryList.end(); iter++)
    {
        int varNum = (*iter)->getVarNum();
        for (int i=0;i<varNum;i++)
        {
            //debug
        	{
        		std::stringstream _ss;
        		_ss << "retrieve of var: " << i << endl;
        		Database::log(_ss.str());
        	}
            const EntityBitSet& entityBitSet = (*iter)->getVarBitSet(i);
            IDList* idListPtr = &( (*iter)->getCandidateList(i) );
            this->retrieveEntity(entityBitSet, idListPtr);

            //debug
//            {
//                std::stringstream _ss;
//                _ss << "candidate num: " << idListPtr->size() << endl;
//                _ss << endl;
//                _ss << "isExist 473738: " << (idListPtr->isExistID(473738)?"true":"false") <<endl;
//                _ss << "isExist 473472: " << (idListPtr->isExistID(473472)?"true":"false") <<endl;
//                _ss << "isExist 473473: " << (idListPtr->isExistID(473473)?"true":"false") <<endl;
//                Database::log(_ss.str());
//            }

        }
    }
	Database::log("OUT retrieve");
}

/* retrieve candidate result set by the var_sig in the _query. */
void VSTree::retrieve(SPARQLquery& _query, char* internal_vertices_arr)
{
	Database::log("IN retrieve");

	//debug
//	{
//	    VNode* temp_ptr = this->getLeafNodeByEntityID(473738);
//	    stringstream _ss;
//
//	    for (int i=0;i<temp_ptr->getChildNum();i++)
//	        if (temp_ptr->getChildEntry(i).getEntityId() == 473738)
//	        {
//	            _ss << "entity id=473738 entry sig:" << endl;
//	            _ss << "entity id=473738 leaf node line: " << temp_ptr->getFileLine() << endl;
//	            _ss << Signature::BitSet2str(temp_ptr->getChildEntry(i).getEntitySig().entityBitSet) << endl;
//	            break;
//	        }
//
//	    _ss << "leaf node sig:" << endl;
//	    _ss << Signature::BitSet2str(temp_ptr->getEntry().getEntitySig().entityBitSet) << endl;
//
//	    temp_ptr = temp_ptr->getFather(*(this->node_buffer));
//	    while (temp_ptr != NULL)
//	    {
//	        _ss << "line=" << temp_ptr->getFileLine() << endl;
//	        _ss << Signature::BitSet2str(temp_ptr->getEntry().getEntitySig().entityBitSet) << endl;
//	        temp_ptr = temp_ptr->getFather(*(this->node_buffer));
//	    }
//	    Database::log(_ss.str());
//	}

    vector<BasicQuery*>& queryList = _query.getBasicQueryVec();
    // enumerate each BasicQuery and retrieve their variables' mapping entity in the VSTree.
    vector<BasicQuery*>::iterator iter=queryList.begin();
    for(; iter != queryList.end(); iter++)
    {
        int varNum = (*iter)->getVarNum();
        for (int i=0;i<varNum;i++)
        {
            //debug
        	{
        		std::stringstream _ss;
        		_ss << "retrieve of var: " << i << endl;
        		Database::log(_ss.str());
        	}
            const EntityBitSet& entityBitSet = (*iter)->getVarBitSet(i);
            IDList* idListPtr = &( (*iter)->getCandidateList(i) );
            this->retrieveEntity(entityBitSet, idListPtr, internal_vertices_arr);

            //debug
//            {
//                std::stringstream _ss;
//                _ss << "candidate num: " << idListPtr->size() << endl;
//                _ss << endl;
//                _ss << "isExist 473738: " << (idListPtr->isExistID(473738)?"true":"false") <<endl;
//                _ss << "isExist 473472: " << (idListPtr->isExistID(473472)?"true":"false") <<endl;
//                _ss << "isExist 473473: " << (idListPtr->isExistID(473473)?"true":"false") <<endl;
//                Database::log(_ss.str());
//            }

        }
    }
	Database::log("OUT retrieve");
}

/* build the VSTree from the _entity_signature_file. */
bool VSTree::buildTree(std::string _entry_file_path)
{
	Database::log("IN VSTree::buildTree");

    // create the entry buffer and node buffer.
    this->entry_buffer = new EntryBuffer(EntryBuffer::DEFAULT_CAPACITY);
    this->node_buffer = new LRUCache(LRUCache::DEFAULT_CAPACITY);

    // create the root node.
    VNode* rootNodePtr = new VNode();
    rootNodePtr->setAsRoot(true);
    rootNodePtr->setAsLeaf(true);
    rootNodePtr->setFileLine(this->root_file_line);
    this->node_buffer->set(this->root_file_line, rootNodePtr);
    this->node_num ++;
    this->height ++;

    /* when building a new VSTree,
     * we should first create a new tree node file as the external storage
     * of the node buffer on hard disk.*/
    this->node_buffer->createCache(VSTree::tree_node_file_path);

    FILE* filePtr = fopen(_entry_file_path.c_str(), "rb");
    if (filePtr == NULL)
    {
        cerr << "error, can not open file. @VSTree::buildTree" << endl;
        return false;
    }

    /* load the entry file to entry buffer in memory, when the entry buffer is full,
    insert them into the tree. */
    int n;
    n = this->entry_buffer->fillElemsFromFile(filePtr);
    while (n != 0)
    {
        for (int i=0;i<n;i++)
        {
           SigEntry* entryPtr = this->entry_buffer->getElem(i);

           /* the most important part of this function */
           this->insertEntry(*entryPtr);
           /* insertEntry one by one */

        }

        n = this->entry_buffer->fillElemsFromFile(filePtr);
    }

    //debug
    Database::log("insert entries to tree done.");

    //bool flag = this->node_buffer->flush();
    bool flag = this->saveTree();

    //debug
    {
        stringstream _ss;
        _ss << "tree height: " << this->getHeight() << endl;
        _ss << "node num: " << this->node_num << endl;
        Database::log(_ss.str());
    }

    Database::log("OUT VSTree::buildTree");

    //debug
//    {
//        Database::log(this->to_str());
//        Database::log("\n\n\n");
//    }

    return flag;
}

bool VSTree::deleteTree()
{
    this->height = 0;
    this->node_num = 0;
    this->entry_num = 0;
    this->root_file_line = 0;
    delete this->node_buffer;
    delete this->entry_buffer;

    // backup the tree data file.
    if (rename(VSTree::tree_file_foler_path.c_str(), (VSTree::tree_file_foler_path+"_bak").c_str()) == 0)
        return true;
    else
        return false;
}

/* Incrementally update bitset of _entity_id
 * conduct OR operation on Entry(_entity_id)'s EntityBitSet with _bitset
 * Entry of _entity_id must exists    */
bool VSTree::updateEntry(int _entity_id, const EntityBitSet& _bitset)
{
    VNode* leafNodePtr = this->getLeafNodeByEntityID(_entity_id);

    if (leafNodePtr == NULL)
    {
        cerr << "error, can not find the mapping leaf node. @VSTree::updateEntry" << endl;
        return false;
    }

    // find the mapping child entry, update it and refresh signature.
    int childNum = leafNodePtr->getChildNum();
    bool findFlag = false;
    for (int i=0;i<childNum;i++)
    {
        const SigEntry& entry = leafNodePtr->getChildEntry(i);

        if (entry.getEntityId() == _entity_id)
        {
            SigEntry newEntry = entry;
            newEntry |= SigEntry(EntitySig(_bitset), _entity_id);

            //debug
//            {
//                if (_entity_id == 10)
//                {
//                    stringstream _ss;
//                    _ss << "lead node line: " << leafNodePtr->getFileLine() << endl;
//                    _ss << "old entry:\n " << Signature::BitSet2str(entry.getEntitySig().entityBitSet) << endl;
//                    _ss << "new entry:\n " << Signature::BitSet2str(newEntry.getEntitySig().entityBitSet) << endl;
//                    Database::log(_ss.str());
//                }
//            }

            leafNodePtr->setChildEntry(i, newEntry);
            leafNodePtr->refreshAncestorSignature(*(this->node_buffer));
            findFlag = true;

            break;
        }
    }

    if (!findFlag)
    {
        cerr<< "error, can not find the mapping child entry in the leaf node. @VSTree::updateEntry" << endl;
        return false;
    }

    return true;
}

/* Replace the Entry(_enitty_id)'s EntityBitSet with _bitset
 * Entry of _entity_id must exists    */
bool VSTree::replaceEntry(int _entity_id, const EntityBitSet& _bitset)
{
    VNode* leafNodePtr = this->getLeafNodeByEntityID(_entity_id);

    if (leafNodePtr == NULL)
    {
        cerr << "error, can not find the mapping leaf node. @VSTree::replaceEntry" << endl;
        return false;
    }

    // find the mapping child entry, update it and refresh signature.
    int childNum = leafNodePtr->getChildNum();
    bool findFlag = false;
    for (int i=0;i<childNum;i++)
    {
        const SigEntry& entry = leafNodePtr->getChildEntry(i);
        if (entry.getEntityId() == _entity_id)
        {
            SigEntry newEntry(EntitySig(_bitset), _entity_id);
            leafNodePtr->setChildEntry(i, newEntry);
            leafNodePtr->refreshAncestorSignature(*(this->node_buffer));
            findFlag = true;
            break;
        }
    }

    if (!findFlag)
    {
        cerr << "error, can not find the mapping child entry in the leaf node. @VSTree::replaceEntry" << endl;
        return false;
    }

    return true;
}

/* insert an new Entry, whose entity doesn't exist before */
bool VSTree::insertEntry(const SigEntry& _entry)
{

	/* choose the best leaf node to insert the _entry */
    VNode* choosedNodePtr = this->chooseNode(this->getRoot(), _entry);

    //debug
//	{
//        if (_entry.getEntityId() == 4000001)
//        {
//            stringstream _ss;
//            if (choosedNodePtr)
//            {
//                _ss << "insert " << _entry.getEntityId()
//                    << " into [" << choosedNodePtr->getFileLine() << "],\t";
//                _ss << "whose childnum is " << choosedNodePtr->getChildNum() << endl;
//            }
//            else
//            {
//                _ss << "insert " << _entry.getEntityId() << " , can not choose a leaf node to insert entry. @VSTree::insert" << endl;
//            }
//            Database::log(_ss.str());
//        }
//	}

    if (choosedNodePtr == NULL)
    {
        cerr << "error, can not choose a leaf node to insert entry. @VSTree::insert" << endl;
        return false;
    }

    if (choosedNodePtr->isFull())
    {
        /* if the choosed leaf node to insert is full, the node should be split.*/
        this->split(choosedNodePtr, _entry, NULL);

        //debug
//        if (!choosedNodePtr->checkState())
//        {
//            stringstream _ss;
//            _ss << "node " << choosedNodePtr->getFileLine() << " childFileLine error. after split" << endl;
//            Database::log(_ss.str());
//        }
    }
    else
    {
        choosedNodePtr->addChildEntry(_entry, false);
        choosedNodePtr->refreshAncestorSignature(*(this->node_buffer));

        //debug
//        if (!choosedNodePtr->checkState())
//        {
//            stringstream _ss;
//            _ss << "node " << choosedNodePtr->getFileLine() << " childFileLine error. after addChildEntry" << endl;
//            _ss <<"child num=" << choosedNodePtr->getChildNum() << endl;
//            _ss <<"node num=" << this->node_num << " entry num=" << this->entry_num << endl;
//            Database::log(_ss.str());
//        }

        // update the entityID2FileLineMap.
        this->entityID2FileLineMap[_entry.getEntityId()] = choosedNodePtr->getFileLine();
    }
    this->entry_num ++;

    return true;
}

/* remove an existed Entry(_entity_id) from VSTree */
bool VSTree::removeEntry(int _entity_id)
{
    VNode* leafNodePtr = this->getLeafNodeByEntityID(_entity_id);

    if (leafNodePtr == NULL)
    {
        cerr<< "error, can not find the mapping leaf node. @VSTree::removeEntry" << endl;
        return false;
    }

    // seek the entry index of the leaf node.
    int entryIndex = -1;
    int childNum = leafNodePtr->getChildNum();
    for (int i=0;i<childNum;i++)
    {
        if (leafNodePtr->getChildEntry(i).getEntityId() == _entity_id)
        {
            entryIndex = i;
            break;
        }
    }

    if (entryIndex == -1)
    {
        cerr << "error, can not find the entry in leaf node. @VSTree::removeEntry" << endl;
        return false;
    }

    // remove the entry in this leaf node and refresh itself and its ancestors' signature.
    leafNodePtr->removeChild(entryIndex);
    leafNodePtr->refreshAncestorSignature(*(this->node_buffer));
    this->entry_num --;

    /* we do not consider the situation which the leaf node is to be empty by now...
     * in a better way, if the leaf node is empty after removing entry, we should delete it. and recursively judge whether its
     * father is empty, and delete its father node if true. to make the VSTree more balanced, we should combine two nodes if
     * their child number are less than the MIN_CHILD_NUM. when deleting one node from the tree, we should also remove it from
     * tree node file in hard disk by doing some operations on the node_buffer.
     */

    return true;
}

/* save the tree information to tree_info_file_path, and flush the tree nodes in memory to tree_node_file_path. */
bool VSTree::saveTree()
{
    bool flag = this->saveTreeInfo();

    if (flag)
    {
        flag = this->node_buffer->flush();
    }


    return flag;
}

bool VSTree::loadTree()
{
	//cout << "loadTree..." << endl;
	(this->node_buffer) = new LRUCache(LRUCache::DEFAULT_CAPACITY);

    bool flag = this->loadTreeInfo();

    //debug
    {
        stringstream _ss;
        _ss << "tree node num: " << this->node_num << endl;
        Database::log(_ss.str());
    }
    if (flag)
    {
        this->node_buffer->loadCache(VSTree::tree_node_file_path);
        //cout << "finish loadCache" << endl;
    }

    if (flag)
    {
        flag = loadEntityID2FileLineMap();
        //cout << "finish loadEntityID2FileLineMap" << endl;
    }

    return flag;
}

/* choose the best leaf node to insert the _entry,
 * return the choosed leaf node's pointer.
 * Recursion function! */
VNode* VSTree::chooseNode(VNode* _p_node, const SigEntry& _entry)
{
    if (_p_node->isLeaf())
    {
        return _p_node;
    }
    else
    {
        int minDis = Signature::ENTITY_SIG_LENGTH + 1;
        int candidateIndex[VNode::MAX_CHILD_NUM];
        int candidateNum = 0;
        int childNum = _p_node->getChildNum();
        for (int i=0;i<childNum;i++)
        {
            int curDis = _p_node->getChildEntry(i).xEpsilen(_entry);
            if (minDis >= curDis)
            {
                if (minDis > curDis)
                {
                    minDis = curDis;
                    candidateNum = 0;
                }
                candidateIndex[candidateNum ++] = i;
            }
        }

        minDis = Signature::ENTITY_SIG_LENGTH + 1;
        VNode* ret = NULL;
        for (int i=0;i<candidateNum;i++)
        {
        	int child_i = candidateIndex[i];
        	VNode* p_child = _p_node->getChild(child_i, *(this->node_buffer));
        	/* Recursion */
            VNode *candidateLeafPtr = this->chooseNode(p_child, _entry);
            int curDis = candidateLeafPtr->getEntry().xEpsilen(_entry);

            if (curDis == 0)
            {
                return candidateLeafPtr;
            }

            if (minDis > curDis)
            {
                minDis = curDis;
                ret = candidateLeafPtr;
            }
        }

        return ret;
    }
}

void VSTree::split(VNode* _p_node_being_split, const SigEntry& _insert_entry, VNode* _p_insert_node)
{
    //debug
//	{
//		stringstream _ss;
//		_ss << "**********************split happen at "
//			<< _p_node_being_split->getFileLine() << endl;
//		_ss << _p_node_being_split->to_str() << endl;
//		Database::log(_ss.str());
//	}
    // first, add the new child node(if not leaf) or child entry(if leaf) to the full node.
	bool just_insert_entry = (_p_insert_node == NULL);
    if (just_insert_entry)
    {
        _p_node_being_split->addChildEntry(_insert_entry, true);
    }
    else
    {
        _p_node_being_split->addChildNode(_p_insert_node, true);
    }

    SigEntry entryA, entryB;
    /* two seeds to generate two new nodes.
     * seedA kernel: the SigEntry with the minimal count of signature.
     * seedB kernel: the SigEntry with the second minimal count of signature.
     * */

    int minCount = 0; // record the minimal signature count.
    int entryA_index = 0; // record the seedA kernel index.
    for (int i=0;i<VNode::MAX_CHILD_NUM;i++)
    {
        int currentCount = (int) _p_node_being_split->getChildEntry(i).getSigCount();
        if (minCount < currentCount)
        {
            minCount = currentCount;
            entryA_index = i;
        }
    }
    entryA = _p_node_being_split->getChildEntry(entryA_index);

    minCount = 0;
    int entryB_index = 0; // record the seedB kernel index.
    for (int i=0;i<VNode::MAX_CHILD_NUM;i++)
    {
        int currentCount = entryA.xEpsilen(_p_node_being_split->getChildEntry(i));
        if (i != entryA_index && minCount <= currentCount)
        {
            minCount = currentCount;
            entryB_index = i;
        }
    }
    entryB = _p_node_being_split->getChildEntry(entryB_index);

    // AEntryIndex: the entry index near seedA.
    // BEntryIndex: the entry index near seedB.
    std::vector<int> entryIndex_nearA, entryIndex_nearB;
    entryIndex_nearA.clear();
    entryIndex_nearB.clear();
    entryIndex_nearA.push_back(entryA_index);
    entryIndex_nearB.push_back(entryB_index);

    /* just tmp variables, for more readibility */
    int nearA_max_size, nearB_max_size;
    bool nearA_tooSmall, nearB_tooSmall;

    for (int i=0;i<VNode::MAX_CHILD_NUM;i++)
    {
        if (i == entryA_index || i == entryB_index) continue;

        /* should guarantee that each new node has at least MIN_CHILD_NUM children. */
        nearA_max_size = VNode::MAX_CHILD_NUM - entryIndex_nearB.size();
        nearA_tooSmall = (nearA_max_size <= VNode::MIN_CHILD_NUM);

        if (nearA_tooSmall)
        {
            for (;i<VNode::MAX_CHILD_NUM;i++)
            {
                if (i == entryA_index || i == entryB_index) continue;
                entryIndex_nearA.push_back(i);
            }
            break;
        }

        nearB_max_size = VNode::MAX_CHILD_NUM - entryIndex_nearA.size();
        nearB_tooSmall = (nearB_max_size <= VNode::MIN_CHILD_NUM);
        if (nearB_tooSmall)
        {
            for (;i<VNode::MAX_CHILD_NUM;i++)
            {
                if (i == entryA_index || i == entryB_index) continue;
                entryIndex_nearB.push_back(i);
            }
            break;
        }

        /* calculate the distance from
         * the i-th child entry signature to seedA(or seedB).*/

        /*debug target 1*/
        int disToSeedA = entryA.xEpsilen(_p_node_being_split->getChildEntry(i));
        int disToSeedB = entryB.xEpsilen(_p_node_being_split->getChildEntry(i));
        // choose the near one seed to add into
        if (disToSeedA <= disToSeedB)
        {
        	 entryIndex_nearA.push_back(i);
        }
        else
        {
        	 entryIndex_nearB.push_back(i);
        }
    }

    // then create a new node to act as BEntryIndex's father.
    VNode* newNodePtr = this->createNode();

    //debug
//    {
//    	stringstream _ss;
//    	_ss << "new Node is :[" << newNodePtr->getFileLine() << "]" << endl;
//    	Database::log(_ss.str());
//    }
    // the old one acts as AEntryIndex's father.
    VNode* oldNodePtr = _p_node_being_split;

    // if the old node is leaf, set the new node as a leaf.
    if (oldNodePtr->isLeaf())
    {
        newNodePtr->setAsLeaf(true);
    }

    /* add all the entries in BEntryIndex into the new node child entry array,
    and calculate the new node's entry.*/
    for (unsigned i=0;i<entryIndex_nearB.size();i++)
    {
        if (oldNodePtr->isLeaf())
        {
            newNodePtr->addChildEntry(oldNodePtr->getChildEntry(entryIndex_nearB[i]), false);
        }
        else
        {
        	 /*debug target 2*/
        	VNode* childPtr = oldNodePtr->getChild(entryIndex_nearB[i], *(this->node_buffer));
            newNodePtr->addChildNode(childPtr);
        }
    }
    newNodePtr->refreshSignature();

    /* label the child being removed with -1,
     * and update the old node's entry.*/
    std::sort(entryIndex_nearA.begin(), entryIndex_nearA.end(), less<int>());

    //debug
//    {
//    	stringstream _ss;
//    	{
//    		_ss << "nearA: ";
//    		for(int i = 0; i < entryIndex_nearA.size(); i ++)
//    		{
//    			_ss << entryIndex_nearA[i] << " ";
//    		}
//    		_ss << endl;
//
//    		_ss << "nearB: ";
//    		for(int i = 0; i < entryIndex_nearB.size(); i ++)
//    		{
//    			_ss << entryIndex_nearB[i] << " ";
//    		}
//    		_ss << endl;
//    	}
//    	Database::log(_ss.str());
//    }

    for (unsigned i=0;i<entryIndex_nearA.size();i++)
    {
        oldNodePtr->setChildEntry(i, oldNodePtr->getChildEntry(entryIndex_nearA[i]));
        oldNodePtr->setChildFileLine(i, oldNodePtr->getChildFileLine(entryIndex_nearA[i]));
    }
    oldNodePtr->setChildNum(entryIndex_nearA.size());
    oldNodePtr->refreshSignature();

    int oldNode_index = oldNodePtr->getIndexInFatherNode(*(this->node_buffer));
    // full node's father pointer.
    VNode* oldNodeFatherPtr = oldNodePtr->getFather(*(this->node_buffer));
    if (oldNodePtr->isRoot())
    {
        /* if the old node is root,
         * split the root, create a new root,
         * and the tree height will be increased.*/
        VNode* RootNewPtr = this->createNode();

        /* change the old root node to not-root node,
         * and set the RootNew to root node.*/
        oldNodePtr->setAsRoot(false);
        RootNewPtr->setAsRoot(true);

        /* set the split two node(old node and new node) as the new root's child,
         * and update signatures.*/
        RootNewPtr->addChildNode(oldNodePtr);
        RootNewPtr->addChildNode(newNodePtr);
        RootNewPtr->refreshSignature();

        //debug
//        {
//            stringstream _ss;
//            _ss << "create new root:" << endl;
//            _ss << "before swap file line, two sons are: " << oldNodePtr->getFileLine() << " " << newNodePtr->getFileLine() << endl;
//            Database::log(_ss.str());
//        }

        /* should keep the root node always being
         * at the first line(line zero) of the tree node file.*/
        this->swapNodeFileLine(RootNewPtr, oldNodePtr);
        this->height ++;

        //debug
//        {
//            stringstream _ss;
//            _ss << "create new root:" << endl;
//            _ss << "two sons are: " << oldNodePtr->getFileLine() << " " << newNodePtr->getFileLine() << endl;
//            _ss << Signature::BitSet2str(oldNodePtr->getEntry().getEntitySig().entityBitSet) << endl;
//            _ss << RootNewPtr->to_str() << endl;
//            Database::log(_ss.str());
//        }
    }
    else
    {
        /* if the (OldNode) is not Root,
         * change the old node's signature to A's signature.*/
    	oldNodeFatherPtr->setChildEntry(oldNode_index, oldNodePtr->getEntry());


        if (oldNodeFatherPtr->isFull())
        {
        	oldNodeFatherPtr->refreshAncestorSignature(*(this->node_buffer));
            this->split(oldNodeFatherPtr, newNodePtr->getEntry(), newNodePtr);
        }
        else
        {
        	oldNodeFatherPtr->addChildNode(newNodePtr);
        	oldNodeFatherPtr->refreshAncestorSignature(*(this->node_buffer));
        }
    }

    //debug
//    if (!oldNodePtr->checkState())
//    {
//        stringstream _ss;
//        _ss << "node " << oldNodePtr->getFileLine() << " childFileLine error. oldNode when split" << endl;
//        Database::log(_ss.str());
//    }
//    if (!newNodePtr->checkState())
//    {
//        stringstream _ss;
//        _ss << "node " << newNodePtr->getFileLine() << " childFileLine error. newNode when split" << endl;
//        Database::log(_ss.str());
//    }

    // update the entityID2FileLineMap by these two nodes.
    this->updateEntityID2FileLineMap(oldNodePtr);
    this->updateEntityID2FileLineMap(newNodePtr);
}

/* create a new node when one node need splitting. */
VNode* VSTree::createNode()
{
    VNode* newNodePtr = new VNode();
    newNodePtr->setFileLine(this->node_num);
    this->node_buffer->set(this->node_num, newNodePtr);
    this->node_num ++;

    return newNodePtr;
}

/* swap two nodes' file line, their related nodes(father and children nodes) will also be updated. */
void VSTree::swapNodeFileLine(VNode* _p_node_a, VNode* _p_node_b)
{
    int oldNodeAFileLine = _p_node_a->getFileLine();
    int oldNodeBFileLine = _p_node_b->getFileLine();
    int newNodeAFileLine = oldNodeBFileLine;
    int newNodeBFileLine = oldNodeAFileLine;

    // at first, we should get their fathers' and children's pointer.
    VNode* nodeAFatherPtr = _p_node_a->getFather(*(this->node_buffer));
    int nodeARank = _p_node_a->getIndexInFatherNode(*(this->node_buffer));
    VNode* nodeBFatherPtr = _p_node_b->getFather(*(this->node_buffer));
    int nodeBRank = _p_node_b->getIndexInFatherNode(*(this->node_buffer));
    VNode* nodeAChildPtr[VNode::MAX_CHILD_NUM];
    VNode* nodeBChildPtr[VNode::MAX_CHILD_NUM];

    int nodeAChildNum = _p_node_a->getChildNum();
    int nodeBChildNum = _p_node_b->getChildNum();
    for (int i=0;i<nodeAChildNum;i++)
    {
        nodeAChildPtr[i] = _p_node_a->getChild(i, *(this->node_buffer));
    }
    for (int i=0;i<nodeBChildNum;i++)
    {
        nodeBChildPtr[i] = _p_node_b->getChild(i, *(this->node_buffer));
    }

    // update nodes self file line.
    _p_node_a->setFileLine(newNodeAFileLine);
    _p_node_b->setFileLine(newNodeBFileLine);

    // update nodes' fathers' child file line.
    if (!_p_node_a->isRoot())
    {
        nodeAFatherPtr->setChildFileLine(nodeARank, newNodeAFileLine);
    }
    if (!_p_node_b->isRoot())
    {
        nodeBFatherPtr->setChildFileLine(nodeBRank, newNodeBFileLine);
    }

    // update nodes' children's father file line.
    if (!_p_node_a->isLeaf())
    {
        for (int i=0;i<nodeAChildNum;i++)
        {
            nodeAChildPtr[i]->setFatherFileLine(newNodeAFileLine);
        }
    }
    if (!_p_node_b->isLeaf())
    {
        for (int i=0;i<nodeBChildNum;i++)
        {
            nodeBChildPtr[i]->setFatherFileLine(newNodeBFileLine);
        }
    }

    // update the node_buffer.
    this->node_buffer->update(newNodeAFileLine, _p_node_a);
    this->node_buffer->update(newNodeBFileLine, _p_node_b);
}

/* save VSTree's information to tree_info_file_path, such as node_num, entry_num, height, etc. */
bool VSTree::saveTreeInfo()
{
    FILE* filePtr = fopen(VSTree::tree_info_file_path.c_str(), "wb");

    if (filePtr == NULL)
    {
        cerr << "error, can not create tree info file. @VSTree::saveTreeInfo"  << endl;
        return false;
    }

    fseek(filePtr, 0, SEEK_SET);

    fwrite(&this->node_num, sizeof(int), 1, filePtr);
    fwrite(&this->root_file_line, sizeof(int), 1, filePtr);
    fwrite(&this->height, sizeof(int), 1, filePtr);
    int minChildNum = VNode::MIN_CHILD_NUM;
    fwrite(&minChildNum, sizeof(int), 1, filePtr);
    int maxChildNum = VNode::MAX_CHILD_NUM;
    fwrite(&maxChildNum, sizeof(int), 1, filePtr);
    int nodeSize = sizeof(VNode);
    fwrite(&nodeSize,sizeof(int), 1, filePtr);
    int sigLength = Signature::ENTITY_SIG_LENGTH;
    fwrite(&sigLength, sizeof(int), 1, filePtr);
    fwrite(&this->entry_num, sizeof(int), 1, filePtr);
    int nodeBufferSize = this->node_buffer->getCapacity();
    fwrite(&nodeBufferSize, sizeof(int), 1, filePtr);
    fclose(filePtr);

    return true;
}

/* load VSTree's information from tree_info_file_path. */
bool VSTree::loadTreeInfo()
{
    FILE* filePtr = fopen(VSTree::tree_info_file_path.c_str(), "rb");

    if (filePtr == NULL)
    {
        cerr << "error, can not open tree  file:[" <<
        		VSTree::tree_info_file_path <<
        		"]@VSTree::loadTreeInfo"  << endl;
        return false;
    }

    fseek(filePtr, 0, SEEK_SET);

    int tmp;

    fread(&this->node_num, sizeof(int), 1, filePtr);
    fread(&this->root_file_line, sizeof(int), 1, filePtr);
    fread(&this->height, sizeof(int), 1, filePtr);
    fread(&tmp, sizeof(int), 1, filePtr);
    fread(&tmp, sizeof(int), 1, filePtr);
    fread(&tmp,sizeof(int), 1, filePtr);
    int sigLength = Signature::ENTITY_SIG_LENGTH;
    fread(&sigLength, sizeof(int), 1, filePtr);
    if (sigLength > Signature::ENTITY_SIG_LENGTH)
    {
        cerr << "WARNING: signature length is too short. @VSTree::loadTreeInfo" << endl;
    }
    fread(&this->entry_num, sizeof(int), 1, filePtr);
    int nodeBufferSize = this->node_buffer->getCapacity();
    fread(&nodeBufferSize, sizeof(int), 1, filePtr);
    if (nodeBufferSize > this->node_buffer->getCapacity())
    {
        cerr << "WARNING: node buffer size may be too small. @VSTree::loadTreeInfo" << endl;
    }
    fclose(filePtr);

    return true;
}

/* traverse the tree_node_file_path file, load the mapping from entity id to file line. */
bool VSTree::loadEntityID2FileLineMap()
{
    FILE* filePtr = fopen(VSTree::tree_node_file_path.c_str(), "rb");

    if (filePtr == NULL)
    {
        cerr << "error, can not open tree node file. @VSTree::loadEntityID2FileLineMap"  << endl;
        return false;
    }

    size_t vNodeSize = sizeof(VNode);
    int flag = 0;

    flag = fseek(filePtr, 0, SEEK_SET);


    if (flag != 0)
    {
        cerr << "error,can't seek to the fileLine. @VSTree::loadEntityID2FileLineMap" << endl;
        return false;
    }

    this->entityID2FileLineMap.clear();

    VNode* nodePtr = new VNode();
    int cycle_count = 0;
    while (!feof(filePtr))
    {
        bool is_node_read = (fread((char *)nodePtr,vNodeSize,1,filePtr) == 1);
        if (is_node_read)
        {
            this->updateEntityID2FileLineMap(nodePtr);
            //debug
            {
                stringstream _ss;
                if (cycle_count != nodePtr->getFileLine())
                {
                    _ss << "line=" << cycle_count << " nodeLine=" << nodePtr->getFileLine() << endl;
                    Database::log(_ss.str());
                }
            }
            cycle_count ++;
        }
    }
    delete nodePtr;

    fclose(filePtr);

    return true;
}

/* update the entityID2FileLineMap with the _p_node's child entries, the _p_node should be leaf node. */
void VSTree::updateEntityID2FileLineMap(VNode* _p_node)
{
    if (_p_node->isLeaf())
    {
        int line = _p_node->getFileLine();
        int childNum = _p_node->getChildNum();
        for (int i=0;i<childNum;i++)
        {
            // update all this node's child entries' entityID to file line mapping.
            const SigEntry& entry = _p_node->getChildEntry(i);
            int entityID = entry.getEntityId();
            this->entityID2FileLineMap[entityID] = line;

            //debug
            {
                if (entityID == 4000001)
                {
                    Database::log("entity(4000001) found in leaf node!!!");
                }
            }
        }
    }
}

/* get the leaf node pointer by the given _entityID */
VNode* VSTree::getLeafNodeByEntityID(int _entityID)
{
    map<int,int>::iterator iter = this->entityID2FileLineMap.find(_entityID);

    if (iter == this->entityID2FileLineMap.end())
    {
        cerr << "error,can not find the _entityID's mapping fileLine. @VSTree::getLeafNodeByEntityID" << endl;

        return NULL;
    }

    int line = iter->second;

    return this->getNode(line);
}

/* retrieve the candidate entity ID which signature can cover the_entity_bit_set, and add them to the  _p_id_list. */
void VSTree::retrieveEntity(const EntityBitSet& _entity_bit_set, IDList* _p_id_list)
{
	Database::log("IN retrieveEntity");
    EntitySig filterSig(_entity_bit_set);
    std::queue<int>nodeFileFileQueue; //searching node file line queue.

    //debug
    {
        stringstream _ss;
        _ss << "filterSig=" << Signature::BitSet2str(filterSig.entityBitSet) << endl;
        Database::log(_ss.str());
    }

    const SigEntry& root_entry = (this->getRoot())->getEntry();
    Database::log("Get Root Entry");

    if(root_entry.cover(filterSig))
    {
        nodeFileFileQueue.push(this->getRoot()->getFileLine());
    	Database::log("root cover the filter_sig");
    }
    else
    {
    	Database::log("warning: root is not cover the filter_sig");
    }

    //debug
//    {
//    	Database::log(this->getRoot()->to_str());
//    	Database::log("Before BFS");
//    }

    /* using BFS algorithm to traverse the VSTree and retrieve the entry.*/
    while (!nodeFileFileQueue.empty())
    {
        int currentNodeFileLine = nodeFileFileQueue.front();
        nodeFileFileQueue.pop();
        VNode* currentNodePtr = this->getNode(currentNodeFileLine);

        int childNum = currentNodePtr->getChildNum();

        //debug
//        {
//        	std::stringstream _ss;
//        	_ss << "childNum of ["
//        		<< currentNodePtr->getFileLine()
//        		<< "] is " << childNum << endl;
//
//        	for (int i=0;i<childNum;i++)
//        	{
//        	    _ss << currentNodePtr->getChildFileLine(i) << " ";
//        	}
//        	_ss << endl;
//
//        	Database::log(_ss.str());
//        }

        for (int i=0;i<childNum;i++)
        {
            const SigEntry& entry = currentNodePtr->getChildEntry(i);

            if (entry.cover(filterSig))
            {
                if (currentNodePtr->isLeaf())
                {
                    // if leaf node, add the satisfying entries' entity id to result list.
                    _p_id_list->addID(entry.getEntityId());

                    //debug
//                    {
//                    stringstream _ss;
//                    _ss << "child_" << i << " cover filter sig" << endl;
//                    _ss << Signature::BitSet2str(entry.getEntitySig().entityBitSet)<< endl;
//                    Database::log(_ss.str());
//                    }
                }
                else
                {
                    // if non-leaf node, add the child node pointer to the searching queue.
                	//VNode* childPtr = currentNodePtr->getChild(i, *(this->node_buffer));
                    // if non-leaf node, add the child node file line to the searching queue.
                	int childNodeFileLine = currentNodePtr->getChildFileLine(i);
                	nodeFileFileQueue.push(childNodeFileLine);

                    //debug
//                    {
//                        stringstream _ss;
//                        _ss << "child[" << childPtr->getFileLine() << "] cover filter sig" << endl;
//                        Database::log(_ss.str());
//                    }
                }
            }
        }
    }
    Database::log("OUT retrieveEntity");
}

/* retrieve the candidate entity ID which signature can cover the_entity_bit_set, and add them to the  _p_id_list. */
void VSTree::retrieveEntity(const EntityBitSet& _entity_bit_set, IDList* _p_id_list, char* internal_tag_arr)
{
	Database::log("IN retrieveEntity");
    EntitySig filterSig(_entity_bit_set);
    std::queue<int>nodeFileFileQueue; //searching node file line queue.

    //debug
    {
        stringstream _ss;
        _ss << "filterSig=" << Signature::BitSet2str(filterSig.entityBitSet) << endl;
        Database::log(_ss.str());
    }

    const SigEntry& root_entry = (this->getRoot())->getEntry();
    Database::log("Get Root Entry");

    if(root_entry.cover(filterSig))
    {
        nodeFileFileQueue.push(this->getRoot()->getFileLine());
    	Database::log("root cover the filter_sig");
    }
    else
    {
    	Database::log("warning: root is not cover the filter_sig");
    }

    //debug
//    {
//    	Database::log(this->getRoot()->to_str());
//    	Database::log("Before BFS");
//    }

    /* using BFS algorithm to traverse the VSTree and retrieve the entry.*/
    while (!nodeFileFileQueue.empty())
    {
        int currentNodeFileLine = nodeFileFileQueue.front();
        nodeFileFileQueue.pop();
        VNode* currentNodePtr = this->getNode(currentNodeFileLine);

        int childNum = currentNodePtr->getChildNum();

        //debug
//        {
//        	std::stringstream _ss;
//        	_ss << "childNum of ["
//        		<< currentNodePtr->getFileLine()
//        		<< "] is " << childNum << endl;
//
//        	for (int i=0;i<childNum;i++)
//        	{
//        	    _ss << currentNodePtr->getChildFileLine(i) << " ";
//        	}
//        	_ss << endl;
//
//        	Database::log(_ss.str());
//        }

        for (int i=0;i<childNum;i++)
        {
            const SigEntry& entry = currentNodePtr->getChildEntry(i);

            if (entry.cover(filterSig))
            {
                if (currentNodePtr->isLeaf())
                {
                    // if leaf node, add the satisfying entries' entity id to result list.
					int _tmp_id = entry.getEntityId();
					if(internal_tag_arr[_tmp_id] == '1'){
						_p_id_list->addID(_tmp_id);
					}

                    //debug
//                    {
//                    stringstream _ss;
//                    _ss << "child_" << i << " cover filter sig" << endl;
//                    _ss << Signature::BitSet2str(entry.getEntitySig().entityBitSet)<< endl;
//                    Database::log(_ss.str());
//                    }
                }
                else
                {
                    // if non-leaf node, add the child node pointer to the searching queue.
                	//VNode* childPtr = currentNodePtr->getChild(i, *(this->node_buffer));
                    // if non-leaf node, add the child node file line to the searching queue.
                	int childNodeFileLine = currentNodePtr->getChildFileLine(i);
                	nodeFileFileQueue.push(childNodeFileLine);

                    //debug
//                    {
//                        stringstream _ss;
//                        _ss << "child[" << childPtr->getFileLine() << "] cover filter sig" << endl;
//                        Database::log(_ss.str());
//                    }
                }
            }
        }
    }
    Database::log("OUT retrieveEntity");
}

std::string VSTree::to_str()
{
    //debug
    {
        stringstream _ss;
        _ss << "after build tree, root is:" << endl;
        _ss << this->getRoot()->to_str() << endl;
        Database::log(_ss.str());
    }
	std::stringstream _ss;

	 std::queue<int> nodeFileLineQueue;
	 nodeFileLineQueue.push(this->getRoot()->getFileLine());
	 while(! nodeFileLineQueue.empty())
	 {
	        int currentNodeFileLine = nodeFileLineQueue.front();
	        nodeFileLineQueue.pop();
	        VNode* currentNodePtr = this->getNode(currentNodeFileLine);


	        _ss << currentNodePtr->to_str();

	        int childNum = currentNodePtr->getChildNum();
	        for(int i = 0; i < childNum; i ++)
	        {
	        	if(! currentNodePtr->isLeaf())
	        	{
                	int childNodeFileLine = currentNodePtr->getChildFileLine(i);
                	nodeFileLineQueue.push(childNodeFileLine);
	        	}
	       }
	 }

	return _ss.str();
}
