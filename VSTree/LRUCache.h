/*
 * LRUCache.h
 *
 *  Created on: 2014-6-30
 *      Author: hanshuo
 */

#ifndef LRUCACHE_H_
#define LRUCACHE_H_

#include<map>
#include<string>
class VNode;

// before using the cache, you must loadCache or createCache.
class LRUCache
{
public:

	static int DEFAULT_CAPACITY;

    LRUCache(int _capacity=-1);
    ~LRUCache();
    /* load cache's elements from an exist data file. */
    bool loadCache(std::string _filePath="./tree_file");
    /* create a new empty data file, the original one will be overwrite. */
    bool createCache(std::string _filePath="./tree_file");
    /* get the value(node's pointer) by key(node's file line). */
    VNode* get(int _key);
    /* set the key(node's file line) and value(node's pointer). if the key exists now, the value of this key will be overwritten. */
    bool set(int _key, VNode * _value);
    /* update the _key's mapping _value. if the key do not exist, this operation will fail and return false. */
    bool update(int _key, VNode* _value);
    /* write out all the elements to hard disk. */
    bool flush();
    int getCapacity();
    int getRestAmount();
    void showAmount();
    bool isFull();
private:
    int capacity;
    int size;
    int* next;
    int* prev;
    int* keys;
    VNode** values;
    std::map<int,int> key2pos; // mapping from key to pos.
    std::string dataFilePath;
    static const int DEFAULT_NUM = 2;
    static const int START_INDEX = 0;
    static const int END_INDEX = 1;
    static const int NULL_INDEX = -1;
    static const int EOF_FLAG = -1;
    /* put the new visited one to the tail */
    void refresh(int _pos);
    /* free the memory of the _pos element in cache. */
    void freeElem(int _pos);
    /* set the memory of the _pos element in cache */
    void setElem(int _pos, int _key, VNode* _value);
    /* just write the values[_pos] to the hard disk, the VNode in memory will not be free. */
    bool writeOut(int _pos, int _fileLine=-1);
    /* read the value from hard disk, and put it to the values[_pos].
     * before use it, you must make sure that the _pos element in cache is free(unoccupied).*/
    bool readIn(int _pos, int _fileLine);
};

#endif /* LRUCACHE_H_ */
