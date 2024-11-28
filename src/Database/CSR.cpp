#include "CSR.h"

using namespace std;

CSR::CSR()
{

}

CSR::CSR(unsigned pnum)
{
	this->w = 4;
	this->pre_num = pnum;
	std::shared_ptr<std::vector<unsigned>[]> id2vid_sptr(new std::vector<unsigned>[this->pre_num], std::default_delete<std::vector<unsigned>[]>());
	std::shared_ptr<std::map<unsigned, unsigned>[]> vid2id_sptr(new map<unsigned, unsigned>[this->pre_num], std::default_delete<map<unsigned, unsigned>[]>());
	std::shared_ptr<std::vector<unsigned>[]> offset_list_sptr(new std::vector<unsigned>[this->pre_num], std::default_delete<std::vector<unsigned>[]>());
	std::shared_ptr<std::vector<unsigned>[]> adjacency_list_sptr(new std::vector<unsigned>[this->pre_num], std::default_delete<std::vector<unsigned>[]>());
	this->id2vid = id2vid_sptr;
	this->vid2id = vid2id_sptr;
	this->offset_list = offset_list_sptr;
	this->adjacency_list = adjacency_list_sptr;
}

CSR::~CSR()
{
}

void CSR::init(unsigned pnum)
{
	this->w = 4;
	this->pre_num = pnum;
	std::shared_ptr<std::vector<unsigned>[]> id2vid_sptr(new std::vector<unsigned>[this->pre_num], std::default_delete<std::vector<unsigned>[]>());
	std::shared_ptr<std::map<unsigned, unsigned>[]> vid2id_sptr(new map<unsigned, unsigned>[this->pre_num], std::default_delete<map<unsigned, unsigned>[]>());
	std::shared_ptr<std::vector<unsigned>[]> offset_list_sptr(new std::vector<unsigned>[this->pre_num], std::default_delete<std::vector<unsigned>[]>());
	std::shared_ptr<std::vector<unsigned>[]> adjacency_list_sptr(new std::vector<unsigned>[this->pre_num], std::default_delete<std::vector<unsigned>[]>());
	this->id2vid = id2vid_sptr;
	this->vid2id = vid2id_sptr;
	this->offset_list = offset_list_sptr;
	this->adjacency_list = adjacency_list_sptr;
	this->n = 0;
	this->m = 0;
	// this->valid = new bool[this->pre_num];
}

// void CSR::compress(unsigned w)
// {
// 	for(unsigned i=0; i<this->pre_num; ++i)
// 	{
// 		vector<unsigned> offset_list0;
// 		vector<unsigned> adjacency_list0;
// 		offset_list0 = move(this->offset_list[i]);
// 		adjacency_list0 = move(this->adjacency_list[i]);
// 		unsigned vsize = offset_list0.size();
// 		unsigned asize = adjacency_list0.size();
// 		unsigned offset = 0;
// 		for(unsigned vid = 0; vid < vsize; ++vid)
// 		{
// 			unsigned begin = offset_list0[vid];
// 			unsigned end;
// 			if(vid == vsize-1)
// 				end = asize;
// 			else
// 				end = offset_list0[vid+1];
// 			unsigned current_gid = 0;
// 			unsigned group = 0;
// 			unsigned len = 0;
// 			for(unsigned j = begin; j < end; ++j)
// 			{
// 				unsigned nid = adjacency_list0[j]; //neighbor vertex id
// 				unsigned bid = nid%w; //bit id
// 				unsigned gid = (nid-bid)/w; //group id
// 				if(gid == current_gid)
// 					group += 1<<bid;
// 				else
// 				{
// 					if(group != 0)
// 					{
// 						this->adjacency_list[i].push_back(group);
// 						++len;
// 					}
// 					current_gid = gid;
// 					group = (gid<<w) + 1<<bid;
// 				}
// 			}
// 			if(group != 0)
// 			{
// 				this->adjacency_list[i].push_back(group);
// 				++len;
// 			}
// 			this->offset_list[i].push_back(offset);
// 			offset += len;	
// 		}
// 		offset_list0.clear();
// 		adjacency_list0.clear();
// 	}
// }

void CSR::print()
{
	unsigned i, j;
	for (i = 0; i < pre_num; i++)
	{
		cout << "Predicate ID = " << i << endl;

		cout << "	[Index]Vector ID:" << endl;
		cout << "	";
		for (j = 0; j < id2vid[i].size(); j++)
			cout << '[' << j << ']' << id2vid[i][j] << ' ';
		cout << endl;

		cout << "	Offset list:" << endl;
		cout << "	";
		for (j = 0; j < offset_list[i].size(); j++)
			cout << offset_list[i][j] << ' ';
		cout << endl;

		cout << "	Adjacency list:" << endl;
		cout << "	";
		for (j = 0; j < adjacency_list[i].size(); j++)
			cout << adjacency_list[i][j] << ' ';
		cout << endl;
	}
}

long long CSR::sizeInBytes()
{
	long long sz = 0;
	for (unsigned i = 0; i < pre_num; i++)
		sz += 2 * id2vid[i].size() + offset_list[i].size() + adjacency_list[i].size();
	sz *= 8;
	return sz;
}