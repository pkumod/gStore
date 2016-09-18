/*=============================================================================
# Filename: VSTree.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-11-01 13:02
# Description: 
=============================================================================*/

#ifndef _VSTREE_VSTREE_H
#define _VSTREE_VSTREE_H

#include "../Util/Util.h"
#include"../Query/SPARQLquery.h"
#include"VNode.h"
#include"LRUCache.h"
#include"EntryBuffer.h"

class VSTree
{
    friend class VNode;
public:
    VSTree(std::string _store_path);
    ~VSTree();
    int getHeight()const;
    /* build the VSTree from the _entity_signature_file. */
    bool buildTree(std::string _entity_signature_file);
    bool deleteTree();

    /* Incrementally update bitset of _entity_id
     * conduct OR operation on Entry(_entity_id)'s EntityBitSet with _bitset
     * Entry of _entity_id must exists    */
    bool updateEntry(int _entity_id, const EntityBitSet& _bitset);

    /* Replace the Entry(_enitty_id)'s EntityBitSet with _bitset
     * Entry of _entity_id must exists    */
    bool replaceEntry(int _entity_id, const EntityBitSet& _bitset);

    /* insert an new Entry, whose entity doesn't exist before */
    bool insertEntry(const SigEntry& _entry);

    /* remove an existed Entry(_entity_id) from VSTree */
    bool removeEntry(int _entity_id);

    /* save the tree information to tree_info_file_path, and flush the tree nodes in memory to tree_node_file_path. */
    bool saveTree();
    /* load tree from tree_info_file_path and tree_node_file_path files. */
    bool loadTree();
    /* get the tree's root node pointer. */
    VNode* getRoot();
    /* get the node pointer by its file line. */
    VNode* getNode(int _line);
    /* retrieve candidate result set by the var_sig in the _query. */
    void retrieve(SPARQLquery& _query);
	/* retrieve candidate result set by the var_sig in the _query. */
    void retrieve(SPARQLquery& _query, char* internal_vertices_arr);
private:
	int root_file_line;
	int node_num;
	int entry_num;
	int height;
	LRUCache* node_buffer;
	EntryBuffer* entry_buffer;
	map<int, int> entityID2FileLineMap; // record the mapping from entityID to their node's file line.
	static std::string tree_file_foler_path;
	static std::string tree_node_file_path;
	static std::string tree_info_file_path;

	/* choose the best leaf node to insert the _entry, return the choosed leaf node's pointer. */
	VNode* chooseNode(VNode* _p_node, const SigEntry& _entry);
	/* split the _p_full_node to two new node when it is full.
	 * the parameter _insert_entry and _p_insert_node are the entry/node
	 * need to be insert to the _p_full_node.
	 */
	void split(VNode* _p_full_node, const SigEntry& _insert_entry, VNode* _p_insert_node);
	/* create a new node when one node need splitting. */
	VNode* createNode();
	/* swap two nodes' file line, their related nodes(father and children nodes) will also be updated. */
	void swapNodeFileLine(VNode* _p_node_a, VNode* _p_node_b);
	/* save VSTree's information to tree_info_file_path, such as node_num, entry_num, height, etc. */
	bool saveTreeInfo();
	/* load VSTree's information from tree_info_file_path. */
	bool loadTreeInfo();
	/* traverse the tree_node_file_path file, load the mapping from entity id to file line. */
	bool loadEntityID2FileLineMap();
	/* update the entityID2FileLineMap with the _p_node's child entries, the _p_node should be leaf node. */
	void updateEntityID2FileLineMap(VNode* _p_node);
	/* get the leaf node pointer by the given _entityID */
	VNode* getLeafNodeByEntityID(int _entityID);
	/* retrieve the candidate entity ID which signature can cover the_entity_bit_set, and add them to the  _p_id_list. */
	void retrieveEntity(const EntityBitSet& _entity_bit_set, IDList* _p_id_list);
	/* retrieve the candidate entity ID which signature can cover the_entity_bit_set, and add them to the  _p_id_list. */
	void retrieveEntity(const EntityBitSet& _entity_bit_set, IDList* _p_id_list, char* internal_tag_arr);

	std::string to_str();
};


#endif /* VSTREE_H_ */
