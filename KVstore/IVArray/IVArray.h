/*=======================================================================
 * File name: IVArray.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-02-09
 * Description:
 * a Key-Value Index for ID-Value pair in form of Array
 * =======================================================================*/

#include "../../Util/Util.h"
#include "IVEntry.h"
#include "IVBlockManager.h"

using namespace std;

#define IVMAXKEYNUM 1 << 31
#define IVMIN(a, b) a < b ? a : b

class IVArray {
  private:
  // stores at most 1 billion keys
  static const unsigned int MAX_KEY_NUM = 1 << 31;
  static const unsigned int SET_KEY_NUM = 1 << 10;     // minimum initial keys num
  static const unsigned int SET_KEY_INC = SET_KEY_NUM; // minimum keys num inc
  static const unsigned int SEG_LEN = 1 << 8;
  unsigned long long MAX_CACHE_SIZE;

  private:
  IVEntry* array;
  IVEntry* cache_head;
  int cache_tail_id;
  FILE* IVfile; // file that records index-store
  string IVfile_name;
  string filename;
  string dir_path;
  IVBlockManager* BM;
  unsigned CurEntryNum; // how many entries are available
  bool CurEntryNumChange;

  //Cache
  unsigned CurCacheSize;
  //map <unsigned, long> index_time_map;
  //multimap <long, unsigned> time_index_map;

  bool AddInCache(unsigned _key, char* _str, unsigned _len);
  bool SwapOut();
  bool UpdateTime(unsigned _key, bool HasLock = false);

  void RemoveFromLRUQueue(unsigned _key);

  mutex CacheLock;

  public:
  IVArray();
  IVArray(string _dir_path, string _filename, string mode, unsigned long long buffer_size, unsigned _key_num = 0);
  ~IVArray();

  bool search(unsigned _key, char*& _str, unsigned& _len);
  bool modify(unsigned _key, char* _str, unsigned _len);
  bool remove(unsigned _key);
  bool insert(unsigned _key, char* _str, unsigned _len);
  bool save();
  void PinCache(unsigned _key);
};
