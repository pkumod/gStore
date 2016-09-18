/*
 * LRUCache.cpp
 *
 *  Created on: 2014-6-30
 *      Author: hanshuo
 */

#include"LRUCache.h"
#include"VNode.h"
#include"../Database/Database.h"
#include<stdio.h>
#include<algorithm>

int LRUCache::DEFAULT_CAPACITY = 1*1000*1000;

LRUCache::LRUCache(int _capacity)
{
	//cout << "LRUCache initial..." << endl;
    this->capacity = _capacity > 0 ? _capacity : LRUCache::DEFAULT_CAPACITY;
    // we should guarantee the cache is big enough.
    this->capacity = std::max(this->capacity, VNode::MAX_CHILD_NUM * 2000);

    this->next = new int[this->capacity + 2];
    this->prev = new int[this->capacity + 2];
    this->keys = new int[this->capacity + 2];
    this->values = new VNode*[this->capacity + 2];
    this->next[LRUCache::START_INDEX] = LRUCache::END_INDEX;
    this->next[LRUCache::END_INDEX] = LRUCache::NULL_INDEX;
    this->prev[LRUCache::START_INDEX] = LRUCache::NULL_INDEX;
    this->prev[LRUCache::END_INDEX] = LRUCache::START_INDEX;
    this->size = 0;
    //cout << "LRUCache initial finish" << endl;
}

LRUCache::~LRUCache()
{
    delete []this->next;
    delete []this->prev;
    delete []this->keys;
    delete []this->values;
}

/* load cache's elements from an exist data file. */
bool LRUCache::loadCache(string _filePath)
{
    this->dataFilePath = _filePath;

    FILE* filePtr = fopen(this->dataFilePath.c_str(),"rb");
    if (filePtr == NULL)
    {
        cerr << "error, can not load an exist data file. @LRUCache::loadCache"  << endl;
        return false;
    }

    int defaultLoadSize = this->capacity / 2;
    size_t vNodeSize = sizeof(VNode);
    int flag = 0;

    flag = fseek(filePtr, 0, SEEK_SET);

    if (flag != 0)
    {
        cerr << "error,can't seek to the fileLine. @LRUCache::loadCache" << endl;
        return false;
    }

    int _tmp_cycle_count = 0;

    while (this->size < defaultLoadSize)
    {
        VNode* nodePtr = new VNode();
        bool is_reach_EOF = feof(filePtr);
        bool is_node_read = (fread((char *)nodePtr,vNodeSize,1,filePtr) == 1);

        if (is_reach_EOF || !is_node_read)
        {
            break;
        }

        int pos = LRUCache::DEFAULT_NUM + this->size;
        this->setElem(pos, nodePtr->getFileLine(), nodePtr);

        //debug
        {
            if (_tmp_cycle_count != nodePtr->getFileLine())
            {
                stringstream _ss;
                _ss << "error file line: " << _tmp_cycle_count << " " << nodePtr->getFileLine() << " " << nodePtr->getChildNum() << endl;
                Database::log(_ss.str());
            }
        }

        _tmp_cycle_count ++;
    }

    fclose(filePtr);

    return true;
}

/* create a new empty data file, the original one will be overwrite. */
bool LRUCache::createCache(string _filePath)
{
    this->dataFilePath = _filePath;

    FILE* filePtr = fopen(this->dataFilePath.c_str(),"wb");
    if (filePtr == NULL)
    {
        cerr << "error, can not create a new data file. @LRUCache::createCache"  << endl;
        return false;
    }
    fclose(filePtr);

    return true;
}

/* set the key(node's file line) and value(node's pointer). if the key exists now, the value of this key will be overwritten. */
bool LRUCache::set(int _key, VNode * _value)
{
    map<int,int>::iterator iter = this->key2pos.find(_key);

    // if the _key is found, overwrite its mapping value.
    if (iter != this->key2pos.end())
    {
        int pos = iter->second;
        this->freeElem(pos);
        this->setElem(pos,_key,_value);
    }
    // if the cache is not full now, just put the key-value to the free slot.
    else if (this->size < this->capacity)
    {
        int pos = LRUCache::DEFAULT_NUM + this->size;
        this->setElem(pos,_key,_value);
    }
    // if the cache is full, should swap out the least recently used one to hard disk.
    else
    {
        // write out and free the memory of the least recently used one.
        int pos = this->next[LRUCache::START_INDEX];
        this->writeOut(pos, this->keys[pos]);
        this->freeElem(pos);

        // set the new one to the memory pool.
        this->setElem(pos,_key,_value);
    }
    return false;
}

/* get the value(node's pointer) by key(node's file line). */
VNode* LRUCache::get(int _key)
{
    VNode* ret = NULL;
    map<int,int>::iterator iter = this->key2pos.find(_key);

    if (iter != this->key2pos.end())
    {
        int pos = iter->second;
        ret = this->values[pos];
    }
    // the value is not in memory now, should load it from hard disk.
    else if (this->size < this->capacity)
    {
        int pos = LRUCache::DEFAULT_NUM + this->size;
        if (this->readIn(pos, _key))
        {
            ret = this->values[pos];
        }
    }
    // if the memory pool is full now, should swap out the least recently used one, and swap in the required value.
    else
    {
        int pos = this->next[LRUCache::START_INDEX];
        this->writeOut(pos, this->keys[pos]);
        this->freeElem(pos);

        if (this->readIn(pos, _key))
        {
            ret = this->values[pos];
        }
    }

    return ret;
}

/* update the _key's mapping _value. if the key do not exist, this operation will fail and return false. */
bool LRUCache::update(int _key, VNode* _value)
{
    // should swap it into cache first.
    VNode* valuePtr = this->get(_key);

    if (valuePtr != NULL)
    {
        int pos = this->key2pos[_key];
        if (this->keys[pos] != _key)
        {
            cerr << "error, the pos is wrong. @LRUCache::update"  << endl;
            return false;
        }

        this->values[pos] = _value;

        return true;
    }

    return false;
}

int LRUCache::getCapacity()
{
    return this->capacity;
}
int LRUCache::getRestAmount()
{
    return this->capacity - this->size;
}
void LRUCache::showAmount()
{
    printf(
    		"TotalAmount=%d\tUsedAmount=%d\tUsedPercent=%.2f%%\n",
    		this->capacity, this->size,
    		(double)this->size / this->capacity * 100.0);
}
bool LRUCache:: isFull()
{
    return this->size == this->capacity;
}

/* put the new visited one to the tail */
void LRUCache::refresh(int _pos)
{
    int prevPos,nextPos;

    prevPos = this->prev[_pos];
    nextPos = this->next[_pos];
    this->next[prevPos] = nextPos;
    this->prev[nextPos] = prevPos;

    prevPos = this->prev[LRUCache::END_INDEX];
    nextPos = LRUCache::END_INDEX;

    this->next[prevPos] = _pos;
    this->prev[nextPos] = _pos;

    this->next[_pos] = LRUCache::END_INDEX;
    this->prev[_pos] = prevPos;
}

/* free the memory of the _pos element in cache. */
void LRUCache:: freeElem(int _pos)
{
    delete this->values[_pos];
    this->key2pos.erase(this->keys[_pos]);
    this->keys[_pos] = LRUCache::NULL_INDEX;

    // update the double linked list.
    int prevPos = this->prev[_pos];
    int nextPos = this->next[_pos];
    this->next[prevPos] = nextPos;
    this->prev[nextPos] = prevPos;
    this->next[_pos] = LRUCache::NULL_INDEX;
    this->prev[_pos] = LRUCache::NULL_INDEX;

    this->size --;
}

/* set the memory of the _pos element in cache */
void LRUCache:: setElem(int _pos, int _key, VNode* _value)
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

    this->size ++;
}

/* just write the values[_pos] to the hard disk, the VNode in memory will not be free. */
bool LRUCache::writeOut(int _pos, int _fileLine)
{
    VNode* nodePtr = this->values[_pos];
    FILE* filePtr = fopen(this->dataFilePath.c_str(),"r+b");

    if (nodePtr == NULL)
    {
        cerr << "error, VNode do not exist. @LRUCache::writeOut" << endl;
        return false;
    }
    if (filePtr == NULL)
    {
        cerr << "error, can't open file. @LRUCache::writeOut" << endl;
        return false;
    }

    if (nodePtr->getFileLine() != _fileLine)
    {
        cerr << "error, fileLine " << _fileLine << "wrong. @LRUCache::writeOut" << endl;
    }

    int line = _fileLine == -1 ? nodePtr->getFileLine() : _fileLine;
    size_t vNodeSize = sizeof(VNode);
    int flag = 0;
    long long seekPos = (long long)line * vNodeSize;

    flag = fseek(filePtr, seekPos, SEEK_SET);

    if (flag != 0)
    {
        cerr << "error, can't seek to the fileLine. @LRUCache::writeOut" << endl;
        return false;
    }

    fwrite((char *)nodePtr,vNodeSize,1,filePtr);
    fclose(filePtr);

    return true;
}

/* read the value from hard disk, and put it to the values[_pos].
 * before use it, you must make sure that the _pos element in cache is free(unoccupied).*/
bool LRUCache::readIn(int _pos, int _fileLine)
{
    VNode* nodePtr = new VNode();
    FILE* filePtr = fopen(this->dataFilePath.c_str(),"rb");

    if (nodePtr == NULL)
    {
        cerr << "error, can not new a VNode. @LRUCache::readIn" << endl;
        return false;
    }
    if (filePtr == NULL)
    {
        cerr << "error, can't open " <<
        		"[" << this->dataFilePath << "]" <<
        		". @LRUCache::readIn" << endl;
        return false;
    }

    int line = _fileLine;
    size_t vNodeSize = sizeof(VNode);
    int flag = 0;
    long long seekPos = (long long)line * vNodeSize;

    flag = fseek(filePtr, seekPos, SEEK_SET);

    if (flag != 0)
    {
        cerr << "error,can't seek to the fileLine. @LRUCache::readIn" << endl;
        return false;
    }

    //bool is_node_read = (fread((char *)nodePtr,vNodeSize,1,filePtr) == 1);
    fclose(filePtr);

    if (nodePtr == NULL || nodePtr->getFileLine() != _fileLine)
    {
        cerr << "error,node fileLine error. @LRUCache::readIn" << endl;
    }

    this->setElem(_pos,_fileLine,nodePtr);

    return true;
}

/* write out all the elements to hard disk. */
bool LRUCache::flush()
{
    FILE* filePtr = fopen(this->dataFilePath.c_str(),"r+b");

    if (filePtr == NULL)
    {
        cerr << "error, can't open file. @LRUCache::flush" << endl;
        return false;
    }

    int startIndex = LRUCache::DEFAULT_NUM;
    int endIndex = startIndex + this->size;
    size_t vNodeSize = sizeof(VNode);

    for (int i=startIndex;i<endIndex;i++)
    {
        VNode* nodePtr = this->values[i];
        int line = this->keys[i];

        //debug
        {
            if (nodePtr->getFileLine() != line)
            {
                stringstream _ss;
                _ss << "line error at !!!" << line << " " << nodePtr->getFileLine() << endl;
                Database::log(_ss.str());
            }
        }

        if (nodePtr == NULL)
        {
            cerr << "error, VNode do not exist. @LRUCache::flush" << endl;
            return false;
        }

        int flag = 0;
        long long seekPos = (long long)line * vNodeSize;
        flag = fseek(filePtr, seekPos, SEEK_SET);

        if (flag != 0)
        {
            cerr << "error, can't seek to the fileLine. @LRUCache::flush" << endl;
            return false;
        }

        fwrite((char *)nodePtr,vNodeSize,1,filePtr);
    }
    fclose(filePtr);

    return true;
}
