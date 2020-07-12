/*=======================================================================
 * File name: ISArray.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-02-09
 * Description:
 * a Key-Value Index for ID-Value pair in form of Array
 * =======================================================================*/

#include "ISEntry.h"
#include "ISBlockManager.h"

using namespace std;

#define ISMAXKEYNUM 1 << 30
#define ISMIN(a, b) a < b ? a : b

class ISArray {
  private:
  // stores at most 1 billion keys
  static const unsigned int MAX_KEY_NUM = 500000000;
  static const unsigned int SET_KEY_NUM = 10 * 1000 * 1000; // minimum initial keys num, same as RDFParser::TRIPLE_NUM_PER_GROUP
  static const unsigned int SET_KEY_INC = SET_KEY_NUM;      // minimum keys num inc
  static const unsigned int SEG_LEN = 1 << 8;
  unsigned long long MAX_CACHE_SIZE;

  private:
  ISEntry* array;
  ISEntry* cache_head;
  int cache_tail_id;
  FILE* ISfile; // file that records index-store
  string ISfile_name;
  string filename;
  string dir_path;
  ISBlockManager* BM;
  unsigned CurEntryNum; // how many entries are available
  bool CurEntryNumChange;

  //Cache
  unsigned CurCacheSize;
  //map <unsigned, long> index_time_map;
  //multimap <long, unsigned> time_index_map;

  bool AddInCache(unsigned _key, char* _str, unsigned _len);
  bool SwapOut();
  bool UpdateTime(unsigned _key);
  void RemoveFromLRUQueue(unsigned _key);

  bool PreLoad();

  mutex AccessLock;

  public:
  ISArray();
  ISArray(string _dir_path, string _filename, string mode, unsigned long long buffer_size, unsigned _key_num = 0);
  ~ISArray();

  bool search(unsigned _key, char*& _str, unsigned& _len);
  bool modify(unsigned _key, char* _str, unsigned _len);
  bool remove(unsigned _key);
  bool insert(unsigned _key, char* _str, unsigned _len);
  bool save();
};
