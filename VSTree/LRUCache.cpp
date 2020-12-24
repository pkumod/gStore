/*
* LRUCache.cpp
*
*  Created on: 2014-6-30
*      Author: hanshuo
*/

#include "LRUCache.h"
#include "VNode.h"

using namespace std;

//NOTICE: we aim to support 1 billion triples in a single machine, whose entity num
//can not exceed the 2 billion limit, and the maxium VNODE num is 2000000000/100=20000000=20M
//However, if there is really 20M VNODEs, the whole memory can not afford it!
//NOTICE:In fact, real graph is not linear, we can assume that 1 billion triples contains at most 1 billion entities
//then the memory cost at most is 23448 * 10M = 200G, which is also too large
//But we can only see at most 200M entities in web graphs, then the memory cost is 40G, which is affordable
//
//CONSIDER:support memory-disk swap in vstree
//However, if we adjust the sig length according to entity num, and VNODE size is decided by sig length, we can control the
//whole vstree memory cost almost 20G
//What is more, if the system memory is enough(precisely, the memory you want to assign to gstore),
//we can also set the sig length larger(which should be included in config file)
//int LRUCache::DEFAULT_CAPACITY = 10000000;
int LRUCache::DEFAULT_CAPACITY = 1 * 1000 * 1000; //about 20G memory for vstree
//int LRUCache::DEFAULT_CAPACITY = 1000;
//NOTICE:10^6 is a good parameter, at most use 20G

//NOTICE: it is ok to set it 4000000 when building!!!  better to adjust according to the current memory usage
//also use 2000000 or smaller for query()
LRUCache::LRUCache(int _capacity)
{
//initialize the lock
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_init(&(this->cache_lock), NULL);
#endif

  //cout<<"size of VNODE: "<<sizeof(VNode)<<endl;
  cout << "size of VNODE: " << VNode::VNODE_SIZE << endl;
  cout << "LRUCache initial..." << endl;
  this->capacity = _capacity > 0 ? _capacity : LRUCache::DEFAULT_CAPACITY;

  //DEBUG:it seems that a minium size is required, for example, multiple path down(the height?)
  //at least 3*h
  //
  // we should guarantee the cache is big enough.
  //this->capacity = std::max(this->capacity, VNode::MAX_CHILD_NUM * 2000);

  this->next = new int[this->capacity + 2];
  this->prev = new int[this->capacity + 2];

  //for keys and values, each time we remove one ,we will fill one in the exact position
  //(each VNODE is the same size, so swap just one can be ok)
  //(if needing to remove, then we should move the last one to here)
  this->keys = new int[this->capacity + 2];
  this->values = new VNode* [this->capacity + 2];

  for (int i = 0; i < this->capacity + 2; ++i) {
    this->values[i] = NULL;
  }

  //prev and next are used to implement the LRU strategy
  this->next[LRUCache::START_INDEX] = LRUCache::END_INDEX;
  this->next[LRUCache::END_INDEX] = LRUCache::NULL_INDEX;
  this->prev[LRUCache::START_INDEX] = LRUCache::NULL_INDEX;
  this->prev[LRUCache::END_INDEX] = LRUCache::START_INDEX;
  this->size = 0;
  cout << "LRUCache initial finish" << endl;
}

LRUCache::~LRUCache()
{
  delete[] this->next;
  delete[] this->prev;
  delete[] this->keys;
  for (int i = 0; i < this->size; ++i) {
    delete this->values[i];
  }
  delete[] this->values;

//destroy the lock
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_destroy(&(this->cache_lock));
#endif
}

//NOTICE:this must be done in one thread(and only one time)
//load cache's elements from an exist data file.
bool LRUCache::loadCache(string _filePath)
{
  this->dataFilePath = _filePath;

  FILE* filePtr = fopen(this->dataFilePath.c_str(), "rb");
  if (filePtr == NULL) {
    cerr << "error, can not load an exist data file. @LRUCache::loadCache" << endl;
    return false;
  }

  //NOTICE:here we set it to the maxium, to ensure all VNODE in memory
  int defaultLoadSize = this->capacity;
  //int defaultLoadSize = this->capacity / 2;
  size_t vNodeSize = VNode::VNODE_SIZE;
  //size_t vNodeSize = sizeof(VNode);
  int flag = 0;

  flag = fseek(filePtr, 0, SEEK_SET);

  if (flag != 0) {
    cerr << "error,can't seek to the fileLine. @LRUCache::loadCache" << endl;
    return false;
  }

  //int _tmp_cycle_count = 0;

  while (this->size < defaultLoadSize) {
    bool is_reach_EOF = feof(filePtr);
    if (is_reach_EOF) {
      break;
    }

    VNode* nodePtr = new VNode(true);
    //VNode* nodePtr = NULL;
    //bool is_node_read = (fread((char *)nodePtr, vNodeSize, 1, filePtr) == 1);
    bool is_node_read = nodePtr->readNode(filePtr);

    if (!is_node_read) {
      delete nodePtr;
      break;
    }

    //NOTICE:not consider invalid node
    if (nodePtr->getFileLine() < 0) {
      //remove invalid node
      delete nodePtr;
      continue;
    }

    //this->size if the real size, while DEFAULT_NUM is the prefix
    //To maintain a double-linked list, the pos 0 is head, while the pos 1 is tail
    int pos = LRUCache::DEFAULT_NUM + this->size;
    this->setElem(pos, nodePtr->getFileLine(), nodePtr);

    //debug
    //{
    //if (_tmp_cycle_count != nodePtr->getFileLine())
    //{
    //stringstream _ss;
    //_ss << "error file line: " << _tmp_cycle_count << " " << nodePtr->getFileLine() << " " << nodePtr->getChildNum() << endl;
    //Util::logging(_ss.str());
    //}
    //}

    //_tmp_cycle_count++;
  }

  fclose(filePtr);

  return true;
}

//create a new empty data file, the original one will be overwrite.
bool LRUCache::createCache(string _filePath)
{
  this->dataFilePath = _filePath;

  FILE* filePtr = fopen(this->dataFilePath.c_str(), "wb");
  if (filePtr == NULL) {
    cerr << "error, can not create a new data file. @LRUCache::createCache" << endl;
    return false;
  }
  fclose(filePtr);

  return true;
}

//DEBUG+WARN:the memory-disk swap strategy exists serious bugs, however, we do not really use this startegy now!!!
//
//set the key(node's file line) and value(node's pointer). if the key exists now, the value of this key will be overwritten.
bool LRUCache::set(int _key, VNode* _value)
{
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_wrlock(&(this->cache_lock));
  pthread_mutex_lock(&(_value->node_lock));
#endif

  map<int, int>::iterator iter = this->key2pos.find(_key);

  // if the _key is found, overwrite its mapping value.
  if (iter != this->key2pos.end()) {
    int pos = iter->second;
    this->freeElem(pos);
    this->setElem(pos, _key, _value);
    //this->refresh(pos);
  }
  // if the cache is not full now, just put the key-value to the free slot.
  else if (this->size < this->capacity) {
#ifdef DEBUG_LRUCACHE
//cout<<"to insert a node in LRU cache"<<endl;
#endif
    int pos = LRUCache::DEFAULT_NUM + this->size;
    this->setElem(pos, _key, _value);
    //this->refresh(pos);
  }
  // if the cache is full, should swap out the least recently used one to hard disk.
  else {
#ifdef DEBUG_LRUCACHE
//cout<<"memory-disk swap hadppened in VSTree - LRUCache"<<endl;
#endif
    // write out and free the memory of the least recently used one.
    int pos = this->next[LRUCache::START_INDEX];
    //cout<<pos<<" "<<_key<<" "<<_value->getFileLine()<<endl;

    int ret = 0;
#ifdef THREAD_VSTREE_ON
    ret = pthread_mutex_trylock(&(this->values[pos]->node_lock));
#endif
    //TODO:scan and select a unlocked one to swap, if no, then wait by cond
    if (ret != 0) //not success
    {
      cout << "error: fail to get the vnode lock in LRUCache::set()" << endl;
    }
//NOTICE:we can unlock here because user has released this lock, if he want to read
//this node again, he must wait for this buffer operation to end up
#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(this->values[pos]->node_lock));
#endif

    this->writeOut(pos, this->keys[pos]);
    this->freeElem(pos);

    // set the new one to the memory pool.
    this->setElem(pos, _key, _value);
    //this->refresh(pos);
  }

#ifdef THREAD_VSTREE_ON
  pthread_rwlock_unlock(&(this->cache_lock));
#endif
  return false;
}

//Assume that the node of this key exist in memory now
bool
LRUCache::del(int _key)
{
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_wrlock(&(this->cache_lock));
#endif

#ifdef DEBUG_LRUCACHE
  cout << "to del in LRUCache " << _key << endl;
#endif
  map<int, int>::iterator iter = this->key2pos.find(_key);
  if (iter != this->key2pos.end()) {
    int pos1 = iter->second;
    int pos2 = LRUCache::DEFAULT_NUM + this->size - 1;
#ifdef DEBUG_LRUCACHE
    cout << "pos 1: " << pos1 << "  pos2: " << pos2 << endl;
#endif
    if (this->values[pos1]->getFileLine() != _key) {
#ifdef DEBUG_LRUCACHE
      cout << "error in del() - file line not mapping" << endl;
#endif
    }

    this->fillElem(pos1, pos2);
//this->refresh(pos1);

//NOTICE:we do not need to update the file now
//We only record the freed file_line, and not used now
//When this file_line is allocated again, then the new node can
//be written into the unused file part
//(VNode size is fixed)
#ifdef THREAD_VSTREE_ON
    pthread_rwlock_unlock(&(this->cache_lock));
#endif

    return true;
  }

#ifdef THREAD_VSTREE_ON
  pthread_rwlock_unlock(&(this->cache_lock));
#endif

  return false;
}

//get the value(node's pointer) by key(node's file line).
VNode* LRUCache::get(int _key)
{
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_rdlock(&(this->cache_lock));
#endif

  VNode* ret = NULL;
  //NOTICE:use map[] will cause the rbtree to enlarge, so we should use find
  map<int, int>::iterator iter = this->key2pos.find(_key);

  if (iter != this->key2pos.end()) {
    int pos = iter->second;
    ret = this->values[pos];
    this->refresh(pos);
  }
  // the value is not in memory now, should load it from hard disk.
  else if (this->size < this->capacity) {
#ifdef THREAD_VSTREE_ON
    pthread_rwlock_unlock(&(this->cache_lock));
    pthread_rwlock_wrlock(&(this->cache_lock));
#endif

//NOTICE+DEBUG:now all are loaded and there should not be any not read, goes here means error!
//And this will cause error in multiple threads program(even if only read)
#ifdef DEBUG_LRUCACHE
    cout << "new read hadppened in VSTree - LRUCache" << endl;
#endif
    int pos = LRUCache::DEFAULT_NUM + this->size;
    if (this->readIn(pos, _key)) {
      ret = this->values[pos];
      this->refresh(pos);
    } else {
      cout << "LRUCache::get() - readIn error in the second case" << endl;
    }
  }
  // if the memory pool is full now, should swap out the least recently used one, and swap in the required value.
  else {
#ifdef THREAD_VSTREE_ON
    pthread_rwlock_unlock(&(this->cache_lock));
    pthread_rwlock_wrlock(&(this->cache_lock));
#endif

#ifdef DEBUG_LRUCACHE
//cout<<"memory-disk swap hadppened in VSTree - LRUCache::get()"<<endl;
#endif
    int pos = this->next[LRUCache::START_INDEX];

    int retval = 0;
#ifdef THREAD_VSTREE_ON
    retval = pthread_mutex_trylock(&(this->values[pos]->node_lock));
#endif
    //TODO:scan and select a unlocked one to swap, if no, then wait by cond
    if (retval != 0) //not success
    {
      cout << "error: fail to get the vnode lock in LRUCache::set()" << endl;
    }
#ifdef THREAD_VSTREE_ON
    pthread_mutex_unlock(&(this->values[pos]->node_lock));
#endif

    this->writeOut(pos, this->keys[pos]);
    this->freeElem(pos);

    //NOTICE: readIn will call setElem to add the new node to tail
    //swap the head and push new to tail, so this is a LRU strategy
    if (this->readIn(pos, _key)) {
      ret = this->values[pos];
      this->refresh(pos);
    } else {
      cout << "LRUCache::get() - readIn error in the third case" << endl;
    }
  }

#ifdef THREAD_VSTREE_ON
  pthread_mutex_lock(&(ret->node_lock));
  pthread_rwlock_unlock(&(this->cache_lock));
#endif

  return ret;
}

//update the _key's mapping _value. if the key do not exist, this operation will fail and return false.
bool LRUCache::update(int _key, VNode* _value)
{
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_wrlock(&(this->cache_lock));
#endif

  // should swap it into cache first.
  VNode* valuePtr = this->get(_key);

  if (valuePtr != NULL) {
    int pos = this->key2pos[_key];
    //BETTER:remove the below cerr
    if (this->keys[pos] != _key) {
      cerr << "error, the pos is wrong. @LRUCache::update" << endl;

#ifdef THREAD_VSTREE_ON
      pthread_rwlock_unlock(&(this->cache_lock));
#endif

      return false;
    }

    this->values[pos] = _value;

#ifdef THREAD_VSTREE_ON
    pthread_rwlock_unlock(&(this->cache_lock));
#endif

    return true;
  }

  cerr << "error:the key not exist!" << endl;
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_unlock(&(this->cache_lock));
#endif

  return false;
}

int LRUCache::getCapacity()
{
  return this->capacity;
}

int LRUCache::getRestAmount()
{
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_rdlock(&(this->cache_lock));
#endif
  int t = this->size;
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_unlock(&(this->cache_lock));
#endif

  return this->capacity - t;
  //return this->capacity - this->size;
}

void LRUCache::showAmount()
{
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_rdlock(&(this->cache_lock));
#endif

  printf(
      "TotalAmount=%d\tUsedAmount=%d\tUsedPercent=%.2f%%\n",
      this->capacity, this->size,
      (double)this->size / this->capacity * 100.0);

#ifdef THREAD_VSTREE_ON
  pthread_rwlock_unlock(&(this->cache_lock));
#endif
}

bool LRUCache::isFull()
{
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_rdlock(&(this->cache_lock));
#endif
  bool ret = this->size == this->capacity;
#ifdef THREAD_VSTREE_ON
  pthread_rwlock_unlock(&(this->cache_lock));
#endif

  return ret;
  //return this->size == this->capacity;
}

//LRU: put the new visited one to the tail
void LRUCache::refresh(int _pos)
{
  int prevPos, nextPos;

  nextPos = this->next[_pos];
  if (nextPos == LRUCache::END_INDEX) {
    //already the last element
    return;
  }

  prevPos = this->prev[_pos];
  this->next[prevPos] = nextPos;
  this->prev[nextPos] = prevPos;

  prevPos = this->prev[LRUCache::END_INDEX];
  nextPos = LRUCache::END_INDEX;

  this->next[prevPos] = _pos;
  this->prev[nextPos] = _pos;

  this->next[_pos] = LRUCache::END_INDEX;
  this->prev[_pos] = prevPos;
}

//free the memory of the _pos element in cache.
void LRUCache::freeElem(int _pos)
{
  if (_pos < LRUCache::DEFAULT_NUM || _pos >= LRUCache::DEFAULT_NUM + this->size) {
    cerr << "error in LRUCache::freeElem() -- invalid pos" << endl;
    return;
  }

  if (this->values[_pos] != NULL) {
    delete this->values[_pos];
    this->values[_pos] = NULL;
  }

  this->key2pos.erase(this->keys[_pos]);
  this->keys[_pos] = LRUCache::NULL_INDEX;

  // update the double linked list.
  int prevPos = this->prev[_pos];
  int nextPos = this->next[_pos];
  this->next[prevPos] = nextPos;
  this->prev[nextPos] = prevPos;
  this->next[_pos] = LRUCache::NULL_INDEX;
  this->prev[_pos] = LRUCache::NULL_INDEX;

  this->size--;
}

//NOTICE: setElem will append the ele to the end, so LRU is ok
//set the memory of the _pos element in cache
void LRUCache::setElem(int _pos, int _key, VNode* _value)
{
  this->key2pos[_key] = _pos;
  this->keys[_pos] = _key;
  this->values[_pos] = _value;

  // put the new element to the tail of the linked list.
  int prevPos = this->prev[LRUCache::END_INDEX];
  int nextPos = LRUCache::END_INDEX;
  this->next[prevPos] = _pos;
  this->prev[nextPos] = _pos;
  this->next[_pos] = LRUCache::END_INDEX;
  this->prev[_pos] = prevPos;
  //NOTICE: this cannot be placed in loadCache() because this may be called by other functions
  this->size++;
}

//NOTICE: fillElem will change the pos1's next to the end, so LRU is ok(pos2 is always the current maximium position)
//move pos2 ele to pos1, and pos1 ele should be freed
void LRUCache::fillElem(int _pos1, int _pos2)
{
  cout << "fill elem in LRUCache() happen" << endl;

  //NOTICE:update to disk, set the node as invalid
  this->freeDisk(_pos1);
  //NOTICE:size is reduced in freeElem
  this->freeElem(_pos1);
  if (_pos1 >= _pos2) //0 ele or 1 ele(just remove the only one)
  {
#ifdef DEBUG_LRUCACHE
    cout << "LRUCache::fillElem() - no need to fill" << endl;
#endif
    return;
  }

  int key = this->keys[_pos2];
#ifdef DEBUG_LRUCACHE
  cout << "another key in fillElem() - " << key << endl;
#endif

  if (this->values[_pos2] == NULL) {
    cout << "error in fillElem() - value for pos2 is NULL" << endl;
  }
  this->key2pos[key] = _pos1;
  this->keys[_pos1] = key;
  this->values[_pos1] = this->values[_pos2];

  this->keys[_pos2] = LRUCache::NULL_INDEX;
  this->values[_pos2] = NULL;
  int prevPos = this->prev[_pos2];
  int nextPos = this->next[_pos2];

  //a real LRU strategy should be used, i.e. push new in tail, remove the oldest in head
  //TODO:change to list and LRU, search only keep current node is ok, but update not
  //lock: tree and buffer
  //
  //QUERY:if pos1 and pos2 are neighbors in prev-next relations
  //can this conflict with freeElem?
  this->next[prevPos] = _pos1;
  this->prev[nextPos] = _pos1;
  this->next[_pos1] = nextPos;
  this->prev[_pos1] = prevPos;
}

bool
LRUCache::freeDisk(int _pos)
{
  VNode* nodePtr = this->values[_pos];
  FILE* filePtr = fopen(this->dataFilePath.c_str(), "r+b");

  if (nodePtr == NULL) {
    cerr << "error, VNode do not exist. @LRUCache::freeDisk" << endl;
    return false;
  }
  if (filePtr == NULL) {
    cerr << "error, can't open file. @LRUCache::freeDisk" << endl;
    return false;
  }

  //size_t vNodeSize = sizeof(VNode);
  size_t vNodeSize = VNode::VNODE_SIZE;
  int line = nodePtr->getFileLine();
  int flag = 0;
  long long seekPos = (long long)line * vNodeSize;

  flag = fseek(filePtr, seekPos, SEEK_SET);

  if (flag != 0) {
    cerr << "error, can't seek to the fileLine. @LRUCache::writeOut" << endl;
    return false;
  }

  nodePtr->setFileLine(-1);
  //fwrite((char *)nodePtr, vNodeSize, 1, filePtr);
  nodePtr->writeNode(filePtr);

  fclose(filePtr);

  return true;
}

//just write the values[_pos] to the hard disk, the VNode in memory will not be free.
bool
LRUCache::writeOut(int _pos, int _fileLine)
{
  VNode* nodePtr = this->values[_pos];
  FILE* filePtr = fopen(this->dataFilePath.c_str(), "r+b");

  if (nodePtr == NULL) {
    cerr << "error, VNode do not exist. @LRUCache::writeOut" << endl;
    return false;
  }
  if (filePtr == NULL) {
    cerr << "error, can't open file. @LRUCache::writeOut" << endl;
    return false;
  }

  if (nodePtr->getFileLine() != _fileLine) {
    cerr << "error, fileLine " << _fileLine << " " << nodePtr->getFileLine() << " wrong. @LRUCache::writeOut" << endl;
  }

  if (!nodePtr->isDirty()) {
    //cout<<"the node not dirty!"<<endl;
    fclose(filePtr);
    return true;
  } else //is modified
  {
    nodePtr->setDirty(false);
  }

  int line = _fileLine == -1 ? nodePtr->getFileLine() : _fileLine;
  size_t vNodeSize = VNode::VNODE_SIZE;
  //size_t vNodeSize = sizeof(VNode);
  int flag = 0;
  long long seekPos = (long long)line * vNodeSize;

  flag = fseek(filePtr, seekPos, SEEK_SET);

  if (flag != 0) {
    cerr << "error, can't seek to the fileLine. @LRUCache::writeOut" << endl;
    return false;
  }

  //fwrite((char *)nodePtr, vNodeSize, 1, filePtr);
  nodePtr->writeNode(filePtr);
  fclose(filePtr);

  return true;
}

//read the value from hard disk, and put it to the values[_pos].
//before use it, you must make sure that the _pos element in cache is free(unoccupied).
bool LRUCache::readIn(int _pos, int _fileLine)
{
#ifdef DEBUG_LRUCACHE
//cout<<"pos: "<<_pos<<" "<<"fileline: "<<_fileLine<<endl;
#endif
  VNode* nodePtr = new VNode(true);
  //VNode* nodePtr = NULL;
  FILE* filePtr = fopen(this->dataFilePath.c_str(), "rb");

  //if (nodePtr == NULL)
  //{
  //cerr << "error, can not new a VNode. @LRUCache::readIn" << endl;
  //return false;
  //}

  if (filePtr == NULL) {
    cerr << "error, can't open "
         << "[" << this->dataFilePath << "]"
         << ". @LRUCache::readIn" << endl;
    return false;
  }

  int line = _fileLine;
  size_t vNodeSize = VNode::VNODE_SIZE;
  //size_t vNodeSize = sizeof(VNode);
  int flag = 0;
  long long seekPos = (long long)line * vNodeSize;

  flag = fseek(filePtr, seekPos, SEEK_SET);

  if (flag != 0) {
    cerr << "error,can't seek to the fileLine. @LRUCache::readIn" << endl;
    return false;
  }

  //bool is_node_read = (fread((char *)nodePtr, vNodeSize, 1, filePtr) == 1);
  //fread((char *)nodePtr, vNodeSize, 1, filePtr);
  nodePtr->readNode(filePtr);
  fclose(filePtr);

  if (nodePtr == NULL || nodePtr->getFileLine() != _fileLine) {
    cout << "node file line: " << nodePtr->getFileLine() << endl;
    cerr << "error,node fileLine error. @LRUCache::readIn" << endl;
  }

  this->setElem(_pos, _fileLine, nodePtr);

  return true;
}

//NOTICE:this can only be done by one thread
//write out all the elements to hard disk.
bool LRUCache::flush()
{
#ifdef DEBUG_VSTREE
  cout << "to flush in LRUCache" << endl;
#endif
  FILE* filePtr = fopen(this->dataFilePath.c_str(), "r+b");

  if (filePtr == NULL) {
    cerr << "error, can't open file. @LRUCache::flush" << endl;
    return false;
  }

  int startIndex = LRUCache::DEFAULT_NUM;
  int endIndex = startIndex + this->size;
  size_t vNodeSize = VNode::VNODE_SIZE;
  //size_t vNodeSize = sizeof(VNode);

  //NOTICE:values are continuous
  for (int i = startIndex; i < endIndex; ++i) {
    VNode* nodePtr = this->values[i];
    int line = this->keys[i];
//cout<<"file line to write "<<line<<endl;

#ifdef DEBUG
    if (nodePtr->getFileLine() != line) {
      cout << "line error at !!!" << line << " " << nodePtr->getFileLine() << endl;
    }
#endif

    if (nodePtr == NULL) {
      cerr << "error, VNode do not exist. @LRUCache::flush" << endl;
      return false;
    }

    if (!nodePtr->isDirty()) {
      continue;
    }

    int flag = 0;
    long long seekPos = (long long)line * vNodeSize;
    flag = fseek(filePtr, seekPos, SEEK_SET);

    if (flag != 0) {
      cerr << "error, can't seek to the fileLine. @LRUCache::flush" << endl;
      return false;
    }

    //fwrite((char *)nodePtr, vNodeSize, 1, filePtr);
    nodePtr->writeNode(filePtr);
  }
  fclose(filePtr);

  return true;
}
