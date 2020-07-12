/*=============================================================================
# Filename: VSTree.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 14:02
# Description: 
=============================================================================*/

#include "VSTree.h"

using namespace std;

string VSTree::tree_file_foler_path;
string VSTree::tree_node_file_path; // to be determine
string VSTree::tree_info_file_path; // to be determine

VSTree::VSTree(std::string _store_path)
{
  this->height = 0;
  this->node_num = 0;
  this->entry_num = 0;
  this->root_file_line = 0;
  this->entry_buffer = NULL;
  this->node_buffer = NULL;
  //set the store path
  VSTree::tree_file_foler_path = _store_path;
  VSTree::tree_node_file_path = VSTree::tree_file_foler_path + "/tree_node_file.dat";
  VSTree::tree_info_file_path = VSTree::tree_file_foler_path + "/tree_info_file.dat";

  this->free_nid_list.clear();
  this->max_nid_alloc = 0;
}

VSTree::~VSTree()
{
  delete this->node_buffer;
  delete this->entry_buffer;
  this->free_nid_list.clear();
  this->max_nid_alloc = 0;
}

bool
VSTree::isEmpty() const
{
  return this->height == 0;
}

int
VSTree::getHeight() const
{
  return this->height;
}

//get the tree's root node pointer.
VNode*
VSTree::getRoot()
{
  //if(this->root_file_line < 0)
  //{
  //return NULL;
  //}
  return (this->node_buffer)->get(this->root_file_line);
}

//get the node pointer by its file line.
VNode*
VSTree::getNode(int _line)
{
  //if (_line >= this->node_num)
  if (_line >= this->max_nid_alloc) {
    //cout<<_line <<" "<<this->max_nid_alloc<<endl;
    cerr << "error, the parameter:_line is bigger than the tree node_num. @VSTree::getNode" << endl;
    return NULL;
  }

  return this->node_buffer->get(_line);
}

//retrieve candidate result set by the var_sig in the _query.
void VSTree::retrieve(SPARQLquery& _query)
{
  if (this->root_file_line < 0) {
    return;
  }

  Util::logging("IN retrieve");

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
  //	    Util::logging(_ss.str());
  //	}

  vector<BasicQuery*>& queryList = _query.getBasicQueryVec();
  // enumerate each BasicQuery and retrieve their variables' mapping entity in the VSTree.
  vector<BasicQuery*>::iterator iter = queryList.begin();
  for (; iter != queryList.end(); iter++) {
    int varNum = (*iter)->getVarNum();
    for (int i = 0; i < varNum; i++) {
      //debug
      {
        std::stringstream _ss;
        _ss << "retrieve of var: " << i << endl;
        Util::logging(_ss.str());
      }
      bool flag = (*iter)->isLiteralVariable(i);
      const EntityBitSet& entityBitSet = (*iter)->getVarBitSet(i);
      IDList* idListPtr = &((*iter)->getCandidateList(i));
      this->retrieveEntity(entityBitSet, idListPtr);
#ifdef DEBUG_VSTREE
      stringstream _ss;
      _ss << "total num: " << this->entry_num << endl;
      _ss << "candidate num: " << idListPtr->size() << endl;
      _ss << endl;
      _ss << "isExist 473738: " << (idListPtr->isExistID(473738) ? "true" : "false") << endl;
      _ss << "isExist 473472: " << (idListPtr->isExistID(473472) ? "true" : "false") << endl;
      _ss << "isExist 473473: " << (idListPtr->isExistID(473473) ? "true" : "false") << endl;
      Util::logging(_ss.str());
#endif

      if (!flag) {
        (*iter)->setReady(i);
      }
      //the basic query should end if one non-literal var has no candidates
      if (idListPtr->size() == 0 && !flag) {
        break;
      }
    }
  }
  Util::logging("OUT retrieve");
}

//NOTICE:this can only be done by one thread
//build the VSTree from the _entity_signature_file.
bool
VSTree::buildTree(std::string _entry_file_path, int _cache_size)
{
  Util::logging("IN VSTree::buildTree");

  //NOTICE: entry buffer don't  need to store all entities, just loop, read and deal
  //not so much memory: 2 * 10^6 * (4+800/8) < 1G

  // create the entry buffer and node buffer.
  this->entry_buffer = new EntryBuffer(EntryBuffer::DEFAULT_CAPACITY);
  //cout<<"entry buffer newed"<<endl;
  this->node_buffer = new LRUCache(_cache_size);
  //this->node_buffer = new LRUCache(LRUCache::DEFAULT_CAPACITY);

  // create the root node.
  //VNode* rootNodePtr = new VNode();
  VNode* rootNodePtr = this->createNode();
  rootNodePtr->setAsRoot(true);
  rootNodePtr->setAsLeaf(true);
  //rootNodePtr->setFileLine(this->root_file_line);
  //this->node_buffer->set(this->root_file_line, rootNodePtr);
  //this->node_num ++;
  this->height++;

  //when building a new VSTree,
  //we should first create a new tree node file as the external storage
  //of the node buffer on hard disk.
  this->node_buffer->createCache(VSTree::tree_node_file_path);

  FILE* filePtr = fopen(_entry_file_path.c_str(), "rb");
  if (filePtr == NULL) {
    cerr << "error, can not open file. @VSTree::buildTree" << endl;
    return false;
  }

  //load the entry file to entry buffer in memory, when the entry buffer is full,
  //insert them into the tree.
  int n;
  n = this->entry_buffer->fillElemsFromFile(filePtr);
  while (n != 0) {
    for (int i = 0; i < n; i++) {
      SigEntry* entryPtr = this->entry_buffer->getElem(i);

      //if(entryPtr->getEntityId() < 0)
      //{
      //cout<<"error: "<<i<<endl;
      //}

      //the most important part of this function
      //insertEntry one by one
      this->insertEntry(*entryPtr);
    }

    n = this->entry_buffer->fillElemsFromFile(filePtr);
  }

  //debug
  //Util::logging("insert entries to tree done.");

  //bool flag = this->node_buffer->flush();
  bool flag = this->saveTree();

  //debug
  {
    //stringstream _ss;
    //_ss << "tree height: " << this->getHeight() << endl;
    //_ss << "node num: " << this->node_num << endl;
    //Util::logging(_ss.str());
  }

  //Util::logging("OUT VSTree::buildTree");

  //debug
  //    {
  //        Util::logging(this->to_str());
  //        Util::logging("\n\n\n");
  //    }

  return flag;
}

bool
VSTree::deleteTree()
{
  this->height = 0;
  this->node_num = 0;
  this->entry_num = 0;
  this->root_file_line = -1;

  this->free_nid_list.clear();
  this->max_nid_alloc = 0;

  delete this->node_buffer;
  delete this->entry_buffer;

  // backup the tree data file.
  if (rename(VSTree::tree_file_foler_path.c_str(), (VSTree::tree_file_foler_path + "_bak").c_str()) == 0)
    return true;
  else
    return false;
}

//Incrementally update bitset of _entity_id
//conduct OR operation on Entry(_entity_id)'s EntityBitSet with _bitset
//Entry of _entity_id must exists
bool VSTree::updateEntry(int _entity_id, const EntityBitSet& _bitset)
{
  //VNode* tp = this->getLeafNodeByEntityID(2402);
  //int tn = tp->getChildNum();
  //cout<<"file line for 2402: "<<tp->getFileLine()<<endl;
  //cout<<"child num for 2402: "<<tp->getChildNum()<<endl;
  //for(int j = 0; j < tn; ++j)
  //{
  //const SigEntry& entry = tp->getChildEntry(j);
  //if (entry.getEntityId() == 2402)
  //{
  //cout<<"found in pos "<<j<<endl;
  //break;
  //}
  //}

  VNode* leafNodePtr = this->getLeafNodeByEntityID(_entity_id);
  if (leafNodePtr == NULL) {
    cerr << "error, can not find the mapping leaf node. @VSTree::updateEntry" << endl;
    cerr << "the entity id: " << _entity_id << endl;
    return false;
  }

  //cout<<"file line for "<<_entity_id<<": "<<tp->getFileLine()<<endl;

  // find the mapping child entry, update it and refresh signature.
  int childNum = leafNodePtr->getChildNum();
  bool findFlag = false;
  for (int i = 0; i < childNum; i++) {
    const SigEntry& entry = leafNodePtr->getChildEntry(i);

    if (entry.getEntityId() == _entity_id) {
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
      //                    Util::logging(_ss.str());
      //                }
      //            }

      leafNodePtr->setChildEntry(i, newEntry);
      leafNodePtr->refreshAncestorSignature(*(this->node_buffer));
      findFlag = true;

      break;
    }
  }

  if (!findFlag) {
    cerr << "error, can not find the mapping child entry in the leaf node. @VSTree::updateEntry" << endl;
#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(leafNodePtr->node_lock));
#endif
    return false;
  }

  //the node has been changed
  leafNodePtr->setDirty();

#ifdef THREAD_VSTREE_ON
  pthread_mutex_unlock(&(leafNodePtr->node_lock));
#endif
  return true;
}

//Replace the Entry(_enitty_id)'s EntityBitSet with _bitset
//Entry of _entity_id must exists
bool
VSTree::replaceEntry(int _entity_id, const EntityBitSet& _bitset)
{
  if (this->root_file_line < 0) {
    return false;
  }

  //cout<<"begin replaceEntry()"<<endl;
  VNode* leafNodePtr = this->getLeafNodeByEntityID(_entity_id);

  if (leafNodePtr == NULL) {
    cerr << "error, can not find the mapping leaf node. @VSTree::replaceEntry" << endl;
    return false;
  }

  // find the mapping child entry, update it and refresh signature.
  int childNum = leafNodePtr->getChildNum();
  //cout<<"get child num, now to loop"<<endl;
  bool findFlag = false;
  for (int i = 0; i < childNum; i++) {
    const SigEntry& entry = leafNodePtr->getChildEntry(i);
    if (entry.getEntityId() == _entity_id) {
      //cout<<"find the entityid in pos "<<i<<endl;
      SigEntry newEntry(EntitySig(_bitset), _entity_id);
      leafNodePtr->setChildEntry(i, newEntry);
      leafNodePtr->refreshAncestorSignature(*(this->node_buffer));
      findFlag = true;
      break;
    }
  }

  //cout<<"root file line: "<<this->root_file_line<<"   "<<"max nid num: "<<this->max_nid_alloc<<endl;
  //cout<<"node num: "<<this->node_num<<"   "<<"file line: "<<leafNodePtr->getFileLine()<<"   "<<"child num: "<<childNum<<endl;
  //for(int j = 1; j < 4; ++j)
  //{
  //VNode* tmp = this->getNode(j);
  //for(int i = 0; i < tmp->getChildNum(); ++i)
  //{
  //const SigEntry& entry = tmp->getChildEntry(i);
  //cout << entry.getEntityId() << " ";
  //}
  //cout << endl;
  //}

  if (!findFlag) {
    cerr << "error, can not find the mapping child entry in the leaf node. @VSTree::replaceEntry" << endl;
#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(leafNodePtr->node_lock));
#endif
    return false;
  }

  leafNodePtr->setDirty();

#ifdef THREAD_VSTREE_ON
  pthread_mutex_unlock(&(leafNodePtr->node_lock));
#endif
  return true;
}

//NOTICE:It is hard for vstree to keep balance if admitting insert/delete
//So, small changes are ok, but please rebuild the vstree if so many changes

//insert an new Entry, whose entity doesn't exist before
bool
VSTree::insertEntry(const SigEntry& _entry)
{
  //choose the best leaf node to insert the _entry
  VNode* choosedNodePtr = NULL;
  if (this->root_file_line < 0) {
    choosedNodePtr = this->createNode();
    choosedNodePtr->setAsRoot(true);
    choosedNodePtr->setAsLeaf(true);
    this->height++;
    this->root_file_line = 0;
  } else {
    choosedNodePtr = this->chooseNode(this->getRoot(), _entry);
  }

#ifdef DEBUG_VSTREE
  if (_entry.getEntityId() == 200) {
    stringstream _ss;
    if (choosedNodePtr) {
      _ss << "insert " << _entry.getEntityId()
          << " into [" << choosedNodePtr->getFileLine() << "],\t";
      _ss << "whose childnum is " << choosedNodePtr->getChildNum() << endl;
    } else {
      _ss << "insert " << _entry.getEntityId() << " , can not choose a leaf node to insert entry. @VSTree::insert" << endl;
    }
    cout << _ss.str() << endl;
  }
#endif

  if (choosedNodePtr == NULL) {
    cerr << "error, can not choose a leaf node to insert entry. @VSTree::insert" << endl;
    return false;
  }

  choosedNodePtr->setDirty();
  if (choosedNodePtr->isFull()) {
#ifdef DEBUG_VSTREE
    cout << "split occur" << endl;
#endif
    //if the choosed leaf node to insert is full, the node should be split.
    this->split(choosedNodePtr, _entry, NULL);

    //debug
    //        if (!choosedNodePtr->checkState())
    //        {
    //            stringstream _ss;
    //            _ss << "node " << choosedNodePtr->getFileLine() << " childFileLine error. after split" << endl;
    //            Util::logging(_ss.str());
    //        }
  } else {
    choosedNodePtr->addChildEntry(_entry, false);
    choosedNodePtr->refreshAncestorSignature(*(this->node_buffer));

    //debug
    //        if (!choosedNodePtr->checkState())
    //        {
    //            stringstream _ss;
    //            _ss << "node " << choosedNodePtr->getFileLine() << " childFileLine error. after addChildEntry" << endl;
    //            _ss <<"child num=" << choosedNodePtr->getChildNum() << endl;
    //            _ss <<"node num=" << this->node_num << " entry num=" << this->entry_num << endl;
    //            Util::logging(_ss.str());
    //        }

    // update the entityID2FileLineMap.
    this->entityID2FileLineMap[_entry.getEntityId()] = choosedNodePtr->getFileLine();
  }

#ifdef THREAD_VSTREE_ON
  pthread_mutex_unlock(&(choosedNodePtr->node_lock));
#endif

  this->entry_num++;

#ifdef DEBUG_VSTREE
//cout<<"file line check: "<<this->entityID2FileLineMap[200]<<endl;
#endif

  return true;
}

//remove an existed Entry(_entity_id) from VSTree
bool
VSTree::removeEntry(int _entity_id)
{
  if (this->root_file_line < 0) {
    return false;
  }

  //cout<<"file line check: "<<this->entityID2FileLineMap[200]<<endl;
  VNode* leafNodePtr = this->getLeafNodeByEntityID(_entity_id);

  if (leafNodePtr == NULL) {
    cerr << "error, can not find the mapping leaf node. @VSTree::removeEntry" << endl;
    return false;
  }

  // seek the entry index of the leaf node.
  int entryIndex = -1;
  int childNum = leafNodePtr->getChildNum();

  //cout<<"root file line: "<<this->root_file_line<<"   "<<"max nid num: "<<this->max_nid_alloc<<endl;
  //cout<<"node num: "<<this->node_num<<"   "<<"file line: "<<leafNodePtr->getFileLine()<<"   "<<"child num: "<<childNum<<endl;

  for (int i = 0; i < childNum; i++) {
    if (leafNodePtr->getChildEntry(i).getEntityId() == _entity_id) {
      entryIndex = i;
      break;
    }
  }

  if (entryIndex == -1) {
    cerr << "error, can not find the entry in leaf node. @VSTree::removeEntry" << endl;
    return false;
  }

  leafNodePtr->setDirty();
  //BETTER?:consider up->bopttom to deal, not find leaf and recursively
  if (leafNodePtr->isRoot()) {
    if (childNum == 1) { //the tree is empty now
      cout << "the vstree is empty now!" << endl;
      leafNodePtr->removeChild(entryIndex);
      leafNodePtr->refreshAncestorSignature(*(this->node_buffer));
      this->removeNode(leafNodePtr);
      leafNodePtr = NULL;

      //DEBUG: already deleted in freeElem
      //delete leafNodePtr;
      //leafNodePtr = NULL;

      this->root_file_line = -1;
      this->height = 0;
      this->entry_num = 0;
      this->node_num = 0;

      //reset the ID info
      this->free_nid_list.clear();
      this->max_nid_alloc = 0;
    } else {
      //cout<<"root remove a child"<<endl;
      leafNodePtr->removeChild(entryIndex);
      leafNodePtr->refreshAncestorSignature(*(this->node_buffer));
    }
  } else {
    if (childNum <= VNode::MIN_CHILD_NUM) {
      //cerr << "In VSTree::remove() -- the node is invalid" << endl;
      //TODO+BETTER:this may search again, too costly
      //VNode* fatherNodePtr = leafNodePtr->getFather(*(this->node_buffer));
      ////int index = leafNodePtr->getIndexInFatherNode(*(this->node_buffer));
      //int n = fatherNodePtr->getChildNum();
      //for (int i = 0; i < n; ++i)
      //{
      //if (fatherNodePtr->getChildFileLine(i) == leafNodePtr->getFileLine())
      //{
      //this->coalesce(fatherNodePtr, i, leafNodePtr, entryIndex);
      //break;
      //}
      //}
      //return false;
      this->coalesce(leafNodePtr, entryIndex);
    } else {
      leafNodePtr->removeChild(entryIndex);
      leafNodePtr->refreshAncestorSignature(*(this->node_buffer));
    }
  }

  if (leafNodePtr != NULL) {
#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(leafNodePtr->node_lock));
#endif
  }

  this->entry_num--;
  this->entityID2FileLineMap.erase(_entity_id);

  //NOTICE:insert is costly but can keep balance.
  //However, remove is not too costly but can not keep balance at all.
  //And remove maybe error if without coalesce!
  //If remove and insert are both wonderful, update/replace can both be balanced using delete and insert again.
  //(not care the balance now, if insert/delete many times, rebuilding is suggested)

  //we do not consider the situation which the leaf node is to be empty by now...
  //in a better way, if the leaf node is empty after removing entry, we should delete it. and recursively judge whether its
  //father is empty, and delete its father node if true. to make the VSTree more balanced, we should combine two nodes if
  //their child number are less than the MIN_CHILD_NUM. when deleting one node from the tree, we should also remove it from
  //tree node file in hard disk by doing some operations on the node_buffer.

  return true;
}

//save the tree information to tree_info_file_path, and flush the tree nodes in memory to tree_node_file_path.
bool
VSTree::saveTree()
{
  bool flag = this->saveTreeInfo();

  if (flag) {
    flag = this->node_buffer->flush();
  }

  return flag;
}

bool
VSTree::loadTree(int _cache_size)
{
  cout << "load VSTree..." << endl;
  (this->node_buffer) = new LRUCache(_cache_size);
  //(this->node_buffer) = new LRUCache(LRUCache::DEFAULT_CAPACITY);
  cout << "LRU cache built" << endl;

  bool flag = this->loadTreeInfo();
  cout << "tree info loaded" << endl;

  //debug
  {
    stringstream _ss;
    _ss << "tree node num: " << this->node_num << endl;
    Util::logging(_ss.str());
  }
  if (flag) {
    this->node_buffer->loadCache(VSTree::tree_node_file_path);
    cout << "finish loadCache" << endl;
  }

  //NOTICE:this must cover all nodes(but not save), different with loadCache
  if (flag) {
    flag = loadEntityID2FileLineMap();
    cout << "finish loadEntityID2FileLineMap" << endl;
  }

  return flag;
}

//choose the best leaf node to insert the _entry, return the choosed leaf node's pointer.  Recursion!
VNode*
VSTree::chooseNode(VNode* _p_node, const SigEntry& _entry)
{
  if (_p_node->isLeaf()) {
    return _p_node;
  } else {
    int minDis = Signature::ENTITY_SIG_LENGTH + 1;
    //int maxDis = Signature::ENTITY_SIG_LENGTH + 1;
    int candidateIndex[VNode::MAX_CHILD_NUM];
    int candidateNum = 0;
    int childNum = _p_node->getChildNum();
    for (int i = 0; i < childNum; i++) {
      int curDis = _p_node->getChildEntry(i).xEpsilen(_entry);
      if (minDis >= curDis) {
        if (minDis > curDis) {
          minDis = curDis;
          candidateNum = 0;
        }
        candidateIndex[candidateNum++] = i;
      }
    }

    //NOTICE: the basic idea is to place similar signatures together?(the smaller num?)
    //BETTER: recursion is too costly , and the performance maybe not so good

    minDis = Signature::ENTITY_SIG_LENGTH + 1;
    //maxDis = Signature::ENTITY_SIG_LENGTH + 1;
    VNode* ret = NULL;
    for (int i = 0; i < candidateNum; i++) {
      int child_i = candidateIndex[i];
      VNode* p_child = _p_node->getChild(child_i, *(this->node_buffer));
      //Recursion
      VNode* candidateLeafPtr = this->chooseNode(p_child, _entry);
      int curDis = candidateLeafPtr->getEntry().xEpsilen(_entry);

      if (curDis == 0) {
        if (ret != NULL) {
#ifdef THREAD_VSTREE_ON
          pthread_mutex_unlock(&(ret->node_lock));
#endif
        }
        return candidateLeafPtr;
      }

      if (minDis > curDis) {
        if (ret != NULL) {
#ifdef THREAD_VSTREE_ON
          pthread_mutex_unlock(&(ret->node_lock));
#endif
        }
        minDis = curDis;
        ret = candidateLeafPtr;
      } else {
#ifdef THREAD_VSTREE_ON
        pthread_mutex_unlock(&(candidateLeafPtr->node_lock));
#endif
      }
    }

#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(_p_node->node_lock));
#endif

    return ret;
  }
}

void
VSTree::split(VNode* _p_node_being_split, const SigEntry& _insert_entry, VNode* _p_insert_node)
{
//VNode* tp = this->getLeafNodeByEntityID(2402);
//if(tp != NULL)
//{
//int tn = tp->getChildNum();
//cout<<"file line for 2402: "<<tp->getFileLine()<<endl;
//cout<<"child num for 2402: "<<tp->getChildNum()<<endl;
//for(int j = 0; j < tn; ++j)
//{
//const SigEntry& entry = tp->getChildEntry(j);
//if (entry.getEntityId() == 2402)
//{
//cout<<"found in pos "<<j<<endl;
//break;
//}
//}
//}
#ifdef DEBUG_VSTREE
  cout << "split happen" << endl;
  cout << _p_node_being_split->getFileLine() << endl;
  cout << "to insert id " << _insert_entry.getEntityId() << endl;
  cout << "height: " << this->height << endl;
#endif
  int oldnode_fileline = _p_node_being_split->getFileLine();
  // first, add the new child node(if not leaf) or child entry(if leaf) to the full node.
  bool just_insert_entry = (_p_insert_node == NULL);
  if (just_insert_entry) {
    _p_node_being_split->addChildEntry(_insert_entry, true);
  } else {
    _p_node_being_split->addChildNode(_p_insert_node, true);
    //pthread_mutex_unlock(&(_p_insert_node->node_lock));
  }
  //NOTICE:now the child num in this node is exactly MAX_CHILD_NUM

  SigEntry entryA, entryB;

  //BETTER: use hanming, xor result or the vector included angle to guess the distince.
  //And then also use the farest two as seeds.
  //
  //two seeds to generate two new nodes.
  //seedA kernel: the SigEntry with the minimal count of signature.
  //seedB kernel: the SigEntry with the maximal count of signature.
  //
  //AIM: divide the entries into two parts, and the or result of the two parts are very different, i.e. a^b is the maxium

  int maxCount = 0;     // record the minimal signature count.
  int entryA_index = 0; // record the seedA kernel index.
  for (int i = 0; i < VNode::MAX_CHILD_NUM; i++) {
    int currentCount = (int)_p_node_being_split->getChildEntry(i).getSigCount();
    if (maxCount < currentCount) {
      maxCount = currentCount;
      entryA_index = i;
    }
  }
  entryA = _p_node_being_split->getChildEntry(entryA_index);

  //maxCount = 0;
  //int entryB_index = 0; // record the seedB kernel index.
  //for(int i = 0; i < VNode::MAX_CHILD_NUM; i++)
  //{
  ////NOTICE:I think xOR should be used here to choose the farest two
  //int currentCount = entryA.xOR(_p_node_being_split->getChildEntry(i));
  ////int currentCount = entryA.xEpsilen(_p_node_being_split->getChildEntry(i));
  //if(i != entryA_index && maxCount <= currentCount)
  //{
  //maxCount = currentCount;
  //entryB_index = i;
  //}
  //}
  //entryB = _p_node_being_split->getChildEntry(entryB_index);

  // AEntryIndex: the entry index near seedA.
  // BEntryIndex: the entry index near seedB.
  std::vector<int> entryIndex_nearA, entryIndex_nearB;
  entryIndex_nearA.clear();
  entryIndex_nearB.clear();

  multimap<int, int> dist;
  for (int i = 0; i < VNode::MAX_CHILD_NUM; ++i) {
    int d = entryA.xEpsilen(_p_node_being_split->getChildEntry(i));
    dist.insert(pair<int, int>(d, i));
  }

  int cnt = 0;
  multimap<int, int>::iterator it;
  for (it = dist.begin(); it != dist.end(); ++it) {
    if (cnt > VNode::MIN_CHILD_NUM) {
      break;
    }
    entryIndex_nearA.push_back(it->second);
    cnt++;
  }

  for (; it != dist.end(); ++it) {
    entryIndex_nearB.push_back(it->second);
  }
  dist.clear();

//NOTICE: code below maybe exist error, can not divide evenly(and maybe not necessary to compute distance)
//
//int nearA_max_size, nearB_max_size;
//bool nearA_tooSmall, nearB_tooSmall;
//for(int i = 0; i < VNode::MAX_CHILD_NUM; i++)
//{
//if(i == entryA_index || i == entryB_index) continue;

////should guarantee that each new node has at least MIN_CHILD_NUM children.
//nearA_max_size = VNode::MAX_CHILD_NUM - entryIndex_nearB.size();
//nearA_tooSmall = (nearA_max_size < VNode::MIN_CHILD_NUM);

////BETTER:is this too wasteful?
//if(nearA_tooSmall)
//{
//for(; i < VNode::MAX_CHILD_NUM; i++)
//{
//if (i == entryA_index || i == entryB_index) continue;
//entryIndex_nearA.push_back(i);
//}
//break;
//}

//nearB_max_size = VNode::MAX_CHILD_NUM - entryIndex_nearA.size();
//nearB_tooSmall = (nearB_max_size < VNode::MIN_CHILD_NUM);
//if(nearB_tooSmall)
//{
//for(; i < VNode::MAX_CHILD_NUM; i++)
//{
//if(i == entryA_index || i == entryB_index) continue;
//entryIndex_nearB.push_back(i);
//}
//break;
//}

////calculate the distance from
////the i-th child entry signature to seedA(or seedB).

////NOTICE:we should expect that the candidate can be almost contained!
////However, the precondition there are not too many 1s
//int disToSeedA = entryA.xEpsilen(_p_node_being_split->getChildEntry(i));
//int disToSeedB = entryB.xEpsilen(_p_node_being_split->getChildEntry(i));
//// choose the near one seed to add into
//if(disToSeedA <= disToSeedB)
//{
//entryIndex_nearA.push_back(i);
//}
//else
//{
//entryIndex_nearB.push_back(i);
//}
//}

#ifdef DEBUG_VSTREE
  cout << "A: " << entryIndex_nearA.size() << "    B: " << entryIndex_nearB.size() << endl;
#endif

  // the old one acts as AEntryIndex's father.
  VNode* oldNodePtr = _p_node_being_split;

  bool is_leaf = oldNodePtr->isLeaf();
  // then create a new node to act as BEntryIndex's father.
  VNode* newNodePtr = this->createNode(is_leaf);
#ifdef DEBUG_VSTREE
  cout << "new node file line: " << newNodePtr->getFileLine() << endl;
#endif

  // if the old node is leaf, set the new node as a leaf.
  if (is_leaf) {
    newNodePtr->setAsLeaf(true);
  }

  //TODO:use lock, then can be used in parallism
  //oldNodePtr = this->getNode(oldnode_fileline);
  //add all the entries in BEntryIndex into the new node child entry array,
  //and calculate the new node's entry.
  if (is_leaf) {
    //NOTICE:I think write the judgement outside is better(no need to judge each time in loop)
    for (unsigned i = 0; i < entryIndex_nearB.size(); i++) {
      //SigEntry st = oldNodePtr->getChildEntry(entryIndex_nearB[i]);
      newNodePtr->addChildEntry(oldNodePtr->getChildEntry(entryIndex_nearB[i]), false);
    }
  } else {
    for (unsigned i = 0; i < entryIndex_nearB.size(); i++) {
      //NOTICE: this requires that the buffer should be able to place half childs of a node
      //(to set the father as the new node)
      //If split occur recursively, the buffer size should be at least 101 * h + h = 102h
      VNode* childPtr = oldNodePtr->getChild(entryIndex_nearB[i], *(this->node_buffer));
      newNodePtr->addChildNode(childPtr);
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(childPtr->node_lock));
#endif
    }
  }
  newNodePtr->refreshSignature();

//label the child being removed with -1,
//and update the old node's entry.
#ifndef PARALLEL_SORT
  sort(entryIndex_nearA.begin(), entryIndex_nearA.end(), less<int>());
#else
  omp_set_num_threads(thread_num);
  __gnu_parallel::sort(entryIndex_nearA.begin(), entryIndex_nearA.end(), less<int>());
#endif

#ifdef DEBUG_VSTREE
//stringstream _ss1;
//{
//_ss1 << "nearA: ";
//for(unsigned i = 0; i < entryIndex_nearA.size(); i++)
//{
//_ss1 << entryIndex_nearA[i] << " ";
//}
//_ss1 << endl;

//_ss1 << "nearB: ";
//for(unsigned i = 0; i < entryIndex_nearB.size(); i++)
//{
//_ss1 << entryIndex_nearB[i] << " ";
//}
//_ss1 << endl;
//}
//Util::logging(_ss1.str());
#endif

  //NOTICE:we need to sort entryIndex_nearA first because we must get from oldNodePtr and set oldNodePtr
  //(in case of overwriting!)
  if (is_leaf) {
    for (unsigned i = 0; i < entryIndex_nearA.size(); i++) {
      oldNodePtr->setChildEntry(i, oldNodePtr->getChildEntry(entryIndex_nearA[i]));
      //WARN:no child file line for leaf node
      //oldNodePtr->setChildFileLine(i, oldNodePtr->getChildFileLine(entryIndex_nearA[i]));
    }
  } else {
    for (unsigned i = 0; i < entryIndex_nearA.size(); i++) {
      oldNodePtr->setChildEntry(i, oldNodePtr->getChildEntry(entryIndex_nearA[i]));
      oldNodePtr->setChildFileLine(i, oldNodePtr->getChildFileLine(entryIndex_nearA[i]));
    }
  }
  oldNodePtr->setChildNum(entryIndex_nearA.size());
  oldNodePtr->refreshSignature();

  if (oldNodePtr->isRoot()) {
#ifdef DEBUG
    cout << "the root need to split" << endl;
#endif
    //if the old node is root,
    //split the root, create a new root,
    //and the tree height will be increased.
    VNode* RootNewPtr = this->createNode(false);

    //change the old root node to not-root node,
    //and set the RootNew to root node.
    oldNodePtr->setAsRoot(false);
    RootNewPtr->setAsRoot(true);

    //set the split two node(old node and new node) as the new root's child,
    //and update signatures.
    RootNewPtr->addChildNode(oldNodePtr);
    RootNewPtr->addChildNode(newNodePtr);
    RootNewPtr->refreshSignature();

    //debug
    //        {
    //            stringstream _ss;
    //            _ss << "create new root:" << endl;
    //            _ss << "before swap file line, two sons are: " << oldNodePtr->getFileLine() << " " << newNodePtr->getFileLine() << endl;
    //            Util::logging(_ss.str());
    //        }

    //should keep the root node always being
    //at the first line(line zero) of the tree node file.
    this->swapNodeFileLine(RootNewPtr, oldNodePtr);
    cout << "new root: " << RootNewPtr->getFileLine() << endl;
    this->height++;
//NOTICE:below is unnecessary
//this->root_file_line = RootNewPtr->getFileLine();

#ifdef DEBUG
    cout << "root file line " << this->root_file_line << " child num " << RootNewPtr->getChildNum() << endl;
#endif

//debug
//        {
//            stringstream _ss;
//            _ss << "create new root:" << endl;
//            _ss << "two sons are: " << oldNodePtr->getFileLine() << " " << newNodePtr->getFileLine() << endl;
//            _ss << Signature::BitSet2str(oldNodePtr->getEntry().getEntitySig().entityBitSet) << endl;
//            _ss << RootNewPtr->to_str() << endl;
//            Util::logging(_ss.str());
//        }
#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(RootNewPtr->node_lock));
#endif
  } else {
    int oldNode_index = oldNodePtr->getIndexInFatherNode(*(this->node_buffer));
    // full node's father pointer.
    VNode* oldNodeFatherPtr = oldNodePtr->getFather(*(this->node_buffer));
    //if the (OldNode) is not Root,
    //change the old node's signature to A's signature.
    oldNodeFatherPtr->setChildEntry(oldNode_index, oldNodePtr->getEntry());
    oldNodeFatherPtr->setDirty();

    if (oldNodeFatherPtr->isFull()) {
      oldNodeFatherPtr->refreshAncestorSignature(*(this->node_buffer));
      this->split(oldNodeFatherPtr, newNodePtr->getEntry(), newNodePtr);
    } else {
      oldNodeFatherPtr->addChildNode(newNodePtr);
      oldNodeFatherPtr->refreshAncestorSignature(*(this->node_buffer));
    }

#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(oldNodeFatherPtr->node_lock));
#endif
  }

  //debug
  //    if (!oldNodePtr->checkState())
  //    {
  //        stringstream _ss;
  //        _ss << "node " << oldNodePtr->getFileLine() << " childFileLine error. oldNode when split" << endl;
  //        Util::logging(_ss.str());
  //    }
  //    if (!newNodePtr->checkState())
  //    {
  //        stringstream _ss;
  //        _ss << "node " << newNodePtr->getFileLine() << " childFileLine error. newNode when split" << endl;
  //        Util::logging(_ss.str());
  //    }

  // update the entityID2FileLineMap by these two nodes.
  this->updateEntityID2FileLineMap(oldNodePtr);
  this->updateEntityID2FileLineMap(newNodePtr);

//pthread_mutex_unlock(&(oldNodePtr->node_lock));
#ifdef THREAD_VSTREE_ON
  pthread_mutex_unlock(&(newNodePtr->node_lock));
#endif
}

//the _entry_index in _child is to be removed.
//node can only be deleted in this function.
void
VSTree::coalesce(VNode*& _child, int _entry_index)
{
#ifdef DEBUG
  cout << "coalesce happen" << endl;
#endif

  //found the father and index
  VNode* _father = _child->getFather(*(this->node_buffer));
  int cn = _child->getChildNum();

  if (_father == NULL) //this is already root
  {
    //cout<<"the father is NULL!!!"<<endl;
    //NOTICE:when root is leaf, at least one key, otherwise the tree is empty
    //But when root is internal, at least two key, if one key then shrink
    //(1-key internal root is not permitted)
    //
    //Notice that leaf-root case has been discussed in upper function removeEntry()
    //so here the root must be internal node
    _child->removeChild(_entry_index);
    if (cn == 2) {
      //only one key after remove, shrink root
      VNode* newRoot = _child->getChild(0, *(this->node_buffer));
      newRoot->setAsRoot(true);
      newRoot->setDirty();
#ifdef DEBUG
      cout << "shrink root in coalesce() -- to swap node file" << endl;
#endif
      this->swapNodeFileLine(newRoot, _child);

#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(newRoot->node_lock));
#endif

      //this->root_file_line = newRoot->getFileLine();
      this->height--;
      this->removeNode(_child);
      _child = NULL;
    }
    return;
  }

  //cout<<"num: "<<cn<<endl;
  if (cn > VNode::MIN_CHILD_NUM) {
#ifdef DEBUG_VSTREE
    cout << "no need to move or union in coalesce()" << endl;
#endif
    _child->removeChild(_entry_index);
    _child->refreshAncestorSignature(*(this->node_buffer));
    return;
  }

  int fn = _father->getChildNum();
  int i, _child_index = -1;

  for (i = 0; i < fn; ++i) {
    if (_father->getChildFileLine(i) == _child->getFileLine()) {
      break;
    }
  }
  if (i == fn) {
    cerr << "not found the leaf node in VSTree::coalesce()" << endl;
    return;
  } else {
    _child_index = i;
  }

  //_child->removeChild(_entry_index);
  //_child->setChildNum(cn);

  //NOTICE:we do not consider the efficiency here, so just ensure the operation is right
  //BETTER:find good way to ensure signatures are separated(maybe similar ones together)
  int ccase = 0;
  VNode* p = NULL;
  int n = 0;

  if (_child_index < fn - 1) {
    p = _father->getChild(_child_index + 1, *(this->node_buffer));
    n = p->getChildNum();
    if (n > VNode::MIN_CHILD_NUM) {
      ccase = 2;
    } else {
      ccase = 1;
    }
  }

  if (_child_index > 0) {
    VNode* tp = _father->getChild(_child_index - 1, *(this->node_buffer));
    int tn = tp->getChildNum();
    if (ccase < 2) {
      if (ccase == 0)
        ccase = 3;
      if (tn > VNode::MIN_CHILD_NUM)
        ccase = 4;
    }
    if (ccase > 2) {
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(p->node_lock));
#endif
      p = tp;
      n = tn;
    }
  }

  VNode* tmp = NULL;
  int child_no = _child->getFileLine();
  int father_no = _father->getFileLine();

#ifdef DEBUG
  if (ccase == 1 || ccase == 3) {
    cout << "union happened" << endl;
  } else if (ccase == 2 || ccase == 4) {
    cout << "move happened" << endl;
  }
  cout << "father num: " << fn << "   child num: " << cn << "   neighbor num: " << n << endl;
  cout << "child file line " << child_no << endl;
  cout << "neighbor file line " << p->getFileLine() << endl;
#endif

  switch (ccase) {
  case 1: //union right to this
    if (_child->isLeaf()) {
      //_child->setChildFileLine(_entry_index, p->getChildFileLine(0));
      _child->setChildEntry(_entry_index, p->getChildEntry(0));
      for (int i = 1; i < n; ++i) {
        //_child->setChildFileLine(cn+i-1, p->getChildFileLine(i));
        _child->addChildEntry(p->getChildEntry(i));
      }
    } else {
      _child->setChildFileLine(_entry_index, p->getChildFileLine(0));
      _child->setChildEntry(_entry_index, p->getChildEntry(0));
      tmp = p->getChild(0, *(this->node_buffer));
      tmp->setFatherFileLine(child_no);
      tmp->setDirty();
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(tmp->node_lock));
#endif
      for (int i = 1; i < n; ++i) {
        tmp = p->getChild(i, *(this->node_buffer));
        //cout<<i<<"   "<<_child->getChildNum()<<endl;
        _child->addChildNode(tmp);
//_child->setChildNum(cn+i);
//tmp->setFatherFileLine(child_no);
#ifdef THREAD_VSTREE_ON
        pthread_mutex_unlock(&(tmp->node_lock));
#endif
      }
    }
    p->setDirty();
    this->removeNode(p);
    //p = NULL;
    _child->refreshSignature();
    _father->setDirty();
    _father->setChildEntry(_child_index, _child->getEntry());
    //recursive:to remove child index+1 in father
    this->coalesce(_father, _child_index + 1);
    break;

  case 2: //move one from right
    if (!_child->isLeaf()) {
      _child->setChildFileLine(_entry_index, p->getChildFileLine(n - 1));
    }
    _child->setChildEntry(_entry_index, p->getChildEntry(n - 1));
    _child->refreshSignature();
    if (!_child->isLeaf()) {
      tmp = p->getChild(n - 1, *(this->node_buffer));
      tmp->setFatherFileLine(child_no);
      tmp->setDirty();
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(tmp->node_lock));
#endif
    }
    p->setDirty();
    p->removeChild(n - 1);
    p->refreshSignature();
    _father->setDirty();
    _father->setChildEntry(_child_index, _child->getEntry());
    _father->setChildEntry(_child_index + 1, p->getEntry());
    _father->refreshAncestorSignature(*(this->node_buffer));
    break;

  case 3: //union left to this
    if (_child->isLeaf()) {
      //_child->setChildFileLine(_entry_index, p->getChildFileLine(0));
      _child->setChildEntry(_entry_index, p->getChildEntry(0));
      for (int i = 1; i < n; ++i) {
        //_child->setChildFileLine(cn+i-1, p->getChildFileLine(i));
        _child->addChildEntry(p->getChildEntry(i));
      }
    } else {
      _child->setChildFileLine(_entry_index, p->getChildFileLine(0));
      _child->setChildEntry(_entry_index, p->getChildEntry(0));
      tmp = p->getChild(0, *(this->node_buffer));
      tmp->setFatherFileLine(child_no);
      tmp->setDirty();
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(tmp->node_lock));
#endif
      for (int i = 1; i < n; ++i) {
        tmp = p->getChild(i, *(this->node_buffer));
        //cout<<i<<"   "<<_child->getChildNum()<<endl;
        _child->addChildNode(tmp);
//_child->setChildNum(cn+i);
//tmp->setFatherFileLine(child_no);
#ifdef THREAD_VSTREE_ON
        pthread_mutex_unlock(&(tmp->node_lock));
#endif
      }
    }
    p->setDirty();
    this->removeNode(p);
    _child->refreshSignature();
    _father->setDirty();
    _father->setChildEntry(_child_index, _child->getEntry());
    //recursive:to remove child index-1 in father
    this->coalesce(_father, _child_index - 1);
    break;

  case 4: //move one from left
    if (!_child->isLeaf()) {
      _child->setChildFileLine(_entry_index, p->getChildFileLine(n - 1));
    }
    _child->setChildEntry(_entry_index, p->getChildEntry(n - 1));
    _child->refreshSignature();
    if (!_child->isLeaf()) {
      VNode* tmp = p->getChild(n - 1, *(this->node_buffer));
      tmp->setFatherFileLine(child_no);
      tmp->setDirty();
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(tmp->node_lock));
#endif
    }
    p->setDirty();
    p->removeChild(n - 1);
    p->refreshSignature();
    _father->setDirty();
    _father->setChildEntry(_child_index, _child->getEntry());
    _father->setChildEntry(_child_index - 1, p->getEntry());
    _father->refreshAncestorSignature(*(this->node_buffer));
    break;

  default:
    cout << "error in coalesce: Invalid case!";
    break;
  }

#ifdef THREAD_VSTREE_ON
  pthread_mutex_unlock(&(_father->node_lock));
#endif

  //BETTER:this maybe very costly because many entity no need to update
  if (_child->isLeaf()) {
    this->updateEntityID2FileLineMap(_child);
    if (ccase == 2 || ccase == 4) {
      this->updateEntityID2FileLineMap(p);
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(p->node_lock));
#endif
    } else {
      //DEBUG
      //delete p;
      //p = NULL;
    }
  }
}

//create a new node when one node need splitting.
VNode*
VSTree::createNode(bool _is_leaf)
{
  VNode* newNodePtr = new VNode(_is_leaf);
  int key = -1;
  if (this->free_nid_list.empty()) {
    key = this->max_nid_alloc++;
    //cout<<"get key by adding "<<key<<endl;
  } else {
    key = *(this->free_nid_list.begin());
    this->free_nid_list.pop_front();

    //cout<<"createNode() - get key "<<key<<endl;
    //int nkey = *(this->free_nid_list.begin());
    //cout<<"createNode() - next key "<<nkey<<endl;
  }
  //key = this->node_num;
  newNodePtr->setFileLine(key);
  newNodePtr->setDirty();
  this->node_buffer->set(key, newNodePtr);
  this->node_num++;

  return newNodePtr;
}

//swap two nodes' file line, their related nodes(father and children nodes) will also be updated.
void
VSTree::swapNodeFileLine(VNode* _p_node_a, VNode* _p_node_b)
{
  //NOTICE+BETTER:used when root split to ensure root is at the first;
  //or when a node is deleted, move the last one to here(not need swap, just fill)
  //we can also consider record the root num here, but why not?
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
  for (int i = 0; i < nodeAChildNum; ++i) {
    nodeAChildPtr[i] = _p_node_a->getChild(i, *(this->node_buffer));
  }
  for (int i = 0; i < nodeBChildNum; ++i) {
    nodeBChildPtr[i] = _p_node_b->getChild(i, *(this->node_buffer));
  }

  // update nodes self file line.
  _p_node_a->setFileLine(newNodeAFileLine);
  _p_node_b->setFileLine(newNodeBFileLine);

  // update nodes' fathers' child file line.
  if (!_p_node_a->isRoot()) {
    nodeAFatherPtr->setChildFileLine(nodeARank, newNodeAFileLine);
  }
  if (!_p_node_b->isRoot()) {
    nodeBFatherPtr->setChildFileLine(nodeBRank, newNodeBFileLine);
  }

#ifdef THREAD_VSTREE_ON
  pthread_mutex_unlock(&(nodeAFatherPtr->node_lock));
  pthread_mutex_unlock(&(nodeBFatherPtr->node_lock));
#endif

  // update nodes' children's father file line.
  if (!_p_node_a->isLeaf()) {
    for (int i = 0; i < nodeAChildNum; ++i) {
      nodeAChildPtr[i]->setFatherFileLine(newNodeAFileLine);
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(nodeAChildPtr[i]->node_lock));
#endif
    }
  }
  if (!_p_node_b->isLeaf()) {
    for (int i = 0; i < nodeBChildNum; ++i) {
      nodeBChildPtr[i]->setFatherFileLine(newNodeBFileLine);
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(nodeBChildPtr[i]->node_lock));
#endif
    }
  }

  // update the node_buffer.
  this->node_buffer->update(newNodeAFileLine, _p_node_a);
  this->node_buffer->update(newNodeBFileLine, _p_node_b);
}

//save VSTree's information to tree_info_file_path, such as node_num, entry_num, height, etc.
bool
VSTree::saveTreeInfo()
{
  FILE* filePtr = fopen(VSTree::tree_info_file_path.c_str(), "wb");

  if (filePtr == NULL) {
    //cerr << "error, can not create tree info file. @VSTree::saveTreeInfo"  << endl;
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
  fwrite(&nodeSize, sizeof(int), 1, filePtr);

  int sigLength = Signature::ENTITY_SIG_LENGTH;
  fwrite(&sigLength, sizeof(int), 1, filePtr);
  fwrite(&this->entry_num, sizeof(int), 1, filePtr);

  int nodeBufferSize = this->node_buffer->getCapacity();
  fwrite(&nodeBufferSize, sizeof(int), 1, filePtr);

  //write max id and free id list
  fwrite(&(this->max_nid_alloc), sizeof(int), 1, filePtr);
  list<int>::iterator it = this->free_nid_list.begin();
  for (; it != this->free_nid_list.end(); ++it) {
    int ele = *it;
    //if(ele == 25)
    //{
    //cout<<"saveTreeInfo() - get id 25"<<endl;
    //}
    //cout<<ele<<endl;
    fwrite(&ele, sizeof(int), 1, filePtr);
  }

  fclose(filePtr);
  return true;
}

//load VSTree's information from tree_info_file_path.
bool
VSTree::loadTreeInfo()
{
  FILE* filePtr = fopen(VSTree::tree_info_file_path.c_str(), "rb");

  if (filePtr == NULL) {
    cerr << "error, can not open tree  file:[" << VSTree::tree_info_file_path << "]@VSTree::loadTreeInfo" << endl;
    fclose(filePtr);
    return false;
  }

  fseek(filePtr, 0, SEEK_SET);

  int tmp;

  fread(&this->node_num, sizeof(int), 1, filePtr);
  fread(&this->root_file_line, sizeof(int), 1, filePtr);
  fread(&this->height, sizeof(int), 1, filePtr);
  fread(&tmp, sizeof(int), 1, filePtr);
  fread(&tmp, sizeof(int), 1, filePtr);
  fread(&tmp, sizeof(int), 1, filePtr);

  int sigLength = Signature::ENTITY_SIG_LENGTH;
  fread(&sigLength, sizeof(int), 1, filePtr);

  if (sigLength > Signature::ENTITY_SIG_LENGTH) {
    cerr << "WARNING: signature length is too short. @VSTree::loadTreeInfo" << endl;
  }

  fread(&this->entry_num, sizeof(int), 1, filePtr);
  int nodeBufferSize = this->node_buffer->getCapacity();
  fread(&nodeBufferSize, sizeof(int), 1, filePtr);

  if (nodeBufferSize > this->node_buffer->getCapacity()) {
    cerr << "WARNING: node buffer size may be too small. @VSTree::loadTreeInfo" << endl;
  }

  //read max id and free id list
  fread(&(this->max_nid_alloc), sizeof(int), 1, filePtr);
  this->free_nid_list.clear();
  int key = -1;
  fread(&key, sizeof(int), 1, filePtr);
  while (!feof(filePtr)) {
#ifdef DEBUG_VSTREE
    if (key == 1) {
      cout << "loadTreeInfo() - get id 1" << endl;
    }
#endif
    this->free_nid_list.push_back(key);
    fread(&key, sizeof(int), 1, filePtr);
  }
  fclose(filePtr);

  //NOTICE: the tree can be empty
  //if(this->root_file_line < 0 || this->root_file_line >= this->max_nid_alloc)
  if (this->root_file_line >= this->max_nid_alloc) {
    return false;
  }

  return true;
}

//traverse the tree_node_file_path file, load the mapping from entity id to file line.
bool
VSTree::loadEntityID2FileLineMap()
{
  FILE* filePtr = fopen(VSTree::tree_node_file_path.c_str(), "rb");

  if (filePtr == NULL) {
    cerr << "error, can not open tree node file. @VSTree::loadEntityID2FileLineMap" << endl;
    return false;
  }

  size_t vNodeSize = sizeof(VNode);
  int flag = 0;

  flag = fseek(filePtr, 0, SEEK_SET);

  if (flag != 0) {
    cerr << "error,can't seek to the fileLine. @VSTree::loadEntityID2FileLineMap" << endl;
    return false;
  }

  this->entityID2FileLineMap.clear();

  //int cycle_count = 0;

  while (!feof(filePtr)) {
    VNode* nodePtr = new VNode(true);
    //bool is_node_read = (fread((char *)nodePtr,vNodeSize,1,filePtr) == 1);
    bool is_node_read = nodePtr->readNode(filePtr);

    //NOTICE:not consider invalid node
    if (is_node_read && nodePtr->getFileLine() >= 0 && nodePtr->isLeaf()) {
      this->updateEntityID2FileLineMap(nodePtr);
      //debug
      //{
      //stringstream _ss;
      //if (cycle_count != nodePtr->getFileLine())
      //{
      //_ss << "line=" << cycle_count << " nodeLine=" << nodePtr->getFileLine() << endl;
      //Util::logging(_ss.str());
      //}
      //}
      //cycle_count ++;
    }
    delete nodePtr;
  }

  fclose(filePtr);

  return true;
}

//update the entityID2FileLineMap with the _p_node's child entries, the _p_node should be leaf node.
void
VSTree::updateEntityID2FileLineMap(VNode* _p_node)
{
  int line = _p_node->getFileLine();
  //cout<<"updateEntityID2FileLineMap() - file line "<<line<<endl;
  int childNum = _p_node->getChildNum();
  for (int i = 0; i < childNum; i++) {
    // update all this node's child entries' entityID to file line mapping.
    const SigEntry& entry = _p_node->getChildEntry(i);
    int entityID = entry.getEntityId();

    //if(entityID == 2402)
    //{
    //cout<<"updateEntityID2FileLineMap() - update id 2402 "<<endl;
    //}

    this->entityID2FileLineMap[entityID] = line;

    //debug
    //{
    //if (entityID == 4000001)
    //{
    //Util::logging("entity(4000001) found in leaf node!!!");
    //}
    //}
  }
}

//get the leaf node pointer by the given _entityID
VNode*
VSTree::getLeafNodeByEntityID(int _entityID)
{
  map<int, int>::iterator iter = this->entityID2FileLineMap.find(_entityID);

  if (iter == this->entityID2FileLineMap.end()) {
    cerr << "error,can not find the _entityID's mapping fileLine. @VSTree::getLeafNodeByEntityID" << endl;
    return NULL;
  }

  int line = iter->second;

  return this->getNode(line);
}

//retrieve the candidate entity ID which signature can cover the _entity_bit_set, and add them to the  _p_id_list.
void
VSTree::retrieveEntity(const EntityBitSet& _entity_bit_set, IDList* _p_id_list)
{
  if (this->root_file_line < 0) {
    return;
  }

  //NOTICE:this may cause parallism error
  //Util::logging("IN retrieveEntity");
  EntitySig filterSig(_entity_bit_set);
#ifdef DEBUG_VSTREE
  cerr << "the filter signature: " << filterSig.to_str() << endl;
#endif
  queue<int> nodeQueue; //searching node file line queue.

#ifdef DEBUG_VSTREE
  stringstream _ss;
  _ss << "filterSig=" << Signature::BitSet2str(filterSig.entityBitSet) << endl;
  Util::logging(_ss.str());
#endif

  const SigEntry& root_entry = (this->getRoot())->getEntry();
  //Util::logging("Get Root Entry");

  if (root_entry.cover(filterSig)) {
    nodeQueue.push(this->getRoot()->getFileLine());
    //Util::logging("root cover the filter_sig");
  } else {
    //Util::logging("warning: root is not cover the filter_sig");
  }

  //debug
  //    {
  //    	Util::logging(this->getRoot()->to_str());
  //    	Util::logging("Before BFS");
  //    }

  //using BFS algorithm to traverse the VSTree and retrieve the entry.
  while (!nodeQueue.empty()) {
    int currentNodeFileLine = nodeQueue.front();
    //cout<<"current node file line: "<<currentNodeFileLine<<endl;

    nodeQueue.pop();
    VNode* currentNodePtr = this->getNode(currentNodeFileLine);

    int childNum = currentNodePtr->getChildNum();
    //cout<<"child num: "<<childNum<<endl;

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
    //        	Util::logging(_ss.str());
    //        }

    int valid = 0;
    for (int i = 0; i < childNum; i++) {
      const SigEntry& entry = currentNodePtr->getChildEntry(i);

#ifdef DEBUG_VSTREE
//cerr << "current entry: " << entry.to_str() << endl;
#endif

      if (entry.cover(filterSig)) {
        valid++;
        if (currentNodePtr->isLeaf()) {
          // if leaf node, add the satisfying entries' entity id to result list.
          _p_id_list->addID(entry.getEntityId());

          //debug
          //                    {
          //                    stringstream _ss;
          //                    _ss << "child_" << i << " cover filter sig" << endl;
          //                    _ss << Signature::BitSet2str(entry.getEntitySig().entityBitSet)<< endl;
          //                    Util::logging(_ss.str());
          //                    }
        } else {
          // if non-leaf node, add the child node pointer to the searching queue.
          //VNode* childPtr = currentNodePtr->getChild(i, *(this->node_buffer));
          // if non-leaf node, add the child node file line to the searching queue.
          int childNodeFileLine = currentNodePtr->getChildFileLine(i);
          nodeQueue.push(childNodeFileLine);

          //the root
          //if(currentNodePtr->getFileLine() == 0)
          //{
          //cout<<"root: "<<i<<" "<<childNodeFileLine<<endl;
          //}

          //debug
          //                    {
          //                        stringstream _ss;
          //                        _ss << "child[" << childPtr->getFileLine() << "] cover filter sig" << endl;
          //                        Util::logging(_ss.str());
          //                    }
        }
      }
    }
#ifdef DEBUG_VSTREE
//cerr << "child num: " << childNum << "   valid num: " << valid << endl;
#endif

#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(currentNodePtr->node_lock));
#endif
  }
  //Util::logging("OUT retrieveEntity");
}

void
VSTree::removeNode(VNode* _vp)
{
  int key = _vp->getFileLine();

#ifdef DEBUG
  cout << "the key to remove: " << key << endl;
#endif

  if (key == this->max_nid_alloc - 1) {
    this->max_nid_alloc--;
  } else {
    this->free_nid_list.push_back(key);
  }
  this->node_buffer->del(key);
  this->node_num--;
  //delete _vp;
  //_vp->setFileLine(-1);
}

string
VSTree::to_str()
{
  //debug
  {
    stringstream _ss;
    _ss << "after build tree, root is:" << endl;
    _ss << this->getRoot()->to_str() << endl;
    Util::logging(_ss.str());
  }
  std::stringstream _ss;

  std::queue<int> nodeFileLineQueue;
  nodeFileLineQueue.push(this->getRoot()->getFileLine());
  while (!nodeFileLineQueue.empty()) {
    int currentNodeFileLine = nodeFileLineQueue.front();
    nodeFileLineQueue.pop();
    VNode* currentNodePtr = this->getNode(currentNodeFileLine);

    _ss << currentNodePtr->to_str();

    int childNum = currentNodePtr->getChildNum();
    for (int i = 0; i < childNum; i++) {
      if (!currentNodePtr->isLeaf()) {
        int childNodeFileLine = currentNodePtr->getChildFileLine(i);
        nodeFileLineQueue.push(childNodeFileLine);
      }
    }
  }

  return _ss.str();
}
