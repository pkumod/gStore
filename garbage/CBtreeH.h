/*
 * CBtree.h
 *
 *  Created on: 2012-3-5
 *      Author: liyouhuan
 */

#ifndef CBTREEH_H_
#define CBTREEH_H_

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <queue>
#include <stack>
#include <set>

#define BLOCKSIZE (1 << 14)              /* 16K */
#define ORDER_V 128                      /* 为简单起见，把v固定为2，实际的B+树v值应该是可配的 */
#define MAXNUM_KEY (ORDER_V * 2)         /* 内部结点中最多键个数，为2v ( 1~2v )*/
#define MAXNUM_POINTER (ORDER_V * 2 + 1) /* 内部结点中最多指向子树的指针个数，为2v ( 1~2v )*/
#define MAXNUM_DATA (ORDER_V * 2 + 1)    /* 结点中用作定义，为2v ( 1~2v )*/
#define TERM_NUMBER 1

#define FLAG_LEFT 5
#define FLAG_RIGHT 6
#define FLAG_ZERO 7
#define FLAG_NO_ZERO 8

#define nFlush1 2500000
#define nFlush2 2000000
#define nFlush3 1500000
#define nFlush4 1000000
#define level1 6000000
#define level2 4000000

using namespace std;

enum NODE_TYPE {
  NODE_TYPE_INTERNAL = 2, // 内部结点
  NODE_TYPE_LEAF = 3,     // 叶子结点
};
enum eletype {
  B_CHARARRAY = 1,
  B_INTEGER = 4,
};
extern eletype _eletype;
extern int _ii;
extern string filePath_o2sID;
extern string filePath_opID2sID;
extern string filePath_s2sID;
extern string filePath_sID2s;
extern int myStrcmp(char _str1[], int len1, char _str2[], int len2);
extern FILE* _log_btree;

class KeyType {
  public:
  int _num_insert;
  bool is_AtMem;
  char* sKey;
  int iKey;
  int mLenKey; //关于是否读入\n的试验
  KeyType()
  {
    KeyType_Initial();
  }
  KeyType(KeyType& a)
  {
    _num_insert = a._num_insert;
    iKey = a.iKey;
    mLenKey = a.mLenKey;
    is_AtMem = a.is_AtMem;
    this->set_sKey(a.sKey, a.mLenKey);
  }
  void KeyType_Initial()
  {
    sKey = NULL;
    iKey = -1;
    mLenKey = -1;
    is_AtMem = false;
    _num_insert = -1;
  }
  ~KeyType()
  {
    clear_skey();
  }
  void clear_skey()
  {
    if (sKey != NULL) {
      delete[] sKey;
      sKey = NULL;
    }
  }

  void set_sKey(const char* _str, int kLen)
  {
    if (_str == NULL) {
      sKey = NULL;
      mLenKey = 0;
      return;
    }
    if (sKey != NULL) {
      delete[] sKey;
      sKey = NULL;
    }
    mLenKey = kLen;
    sKey = new char[mLenKey + 1];
    memcpy(sKey, _str, mLenKey);
    sKey[mLenKey] = '\0';
  }
  void set_iKey(int _i)
  {
    mLenKey = 4;
    iKey = _i;
  }

  void RandInputKey(int _i)
  {
    if (_eletype == B_CHARARRAY) {
      char _c_key[100] = "a";
      int _input_key = rand() % 100000;
      _input_key += _i * 3;
      char _i_to_s[100] = "";
      sprintf(_i_to_s, "%d", _input_key);
      strcat(_c_key, _i_to_s);
      mLenKey = (int)strlen(_c_key);
      sKey = new char[mLenKey + 1];
      strcpy(sKey, _c_key);
      sKey[mLenKey] = '\0';
    } else {
      iKey = _i;
      mLenKey = 4;
    }
  }
  void HandInputKey()
  {
    if (_eletype == B_CHARARRAY) {
      cout << "input the key_string" << endl;
      char _c_key[1000];
      gets(_c_key);
      gets(_c_key);
      mLenKey = (int)strlen(_c_key);
      sKey = new char[mLenKey + 1];
      strcpy(sKey, _c_key);
      sKey[mLenKey] = '\0';
    } else {
      cout << "input the key_integer" << endl;
      int _input_key = 0;
      scanf("%d", &_input_key);
      iKey = _input_key;
      mLenKey = 4;
    }
  }

  void ReadKey(FILE* fp);

  void WriteKey(FILE* fp);
  /*
     * 合适的功能函数
     */
  int WriteSize()
  {
    int size_mLenKey = sizeof(mLenKey);
    int size_Key = 4;
    if (_eletype == B_CHARARRAY) {
      size_Key = (mLenKey + 1) * sizeof(char);
    } else if (_eletype == B_INTEGER) {
      size_Key = sizeof(int);
    } else {
      printf("err in writesize of keytype\n");
      system("pause");
      exit(0);
    }
    return size_mLenKey + size_Key;
  }

  void PrintKey()
  {
    if (_eletype == B_CHARARRAY) {
      printf("[%s,%d]", sKey, this->mLenKey);
    } else if (_eletype == B_INTEGER) {
      printf("%d", iKey);
    } else {
      printf("bug in pk\n");
      system("pause");
      exit(0);
    }
  }
  bool Gt(const KeyType& a) const
  {
    if (_eletype == B_CHARARRAY) {
      if (myStrcmp(sKey, mLenKey, a.sKey, a.mLenKey) > 0)
        return true;
      return false;
    } else if (_eletype == B_INTEGER) {
      if (iKey > a.iKey)
        return true;
      return false;
    } else {
      printf("err in gt\n");
      system("pause");
      exit(0);
    }
    return true;
  }

  bool Lt(const KeyType& a) const
  {
    if (_eletype == B_CHARARRAY) {
      if (myStrcmp(sKey, mLenKey, a.sKey, a.mLenKey) < 0)
        return true;
      return false;
    } else if (_eletype == B_INTEGER) {
      if (iKey < a.iKey)
        return true;
      return false;
    } else {
      printf("err in gt\n");
      system("pause");
      exit(0);
    }
    return true;
  }

  bool Eq(const KeyType& a) const
  {
    if (_eletype == B_CHARARRAY) {
      if (myStrcmp(sKey, mLenKey, a.sKey, a.mLenKey) == 0)
        return true;
      return false;
    } else if (_eletype == B_INTEGER) {
      if (iKey == a.iKey)
        return true;
      return false;
    } else {
      printf("err in gt\n");
      system("pause");
      exit(0);
    }
    return true;
  }

  bool operator>(const KeyType& a) const
  {
    return Gt(a);
  }

  bool operator>=(const KeyType& a) const
  {
    return Gt(a) || Eq(a);
  }

  bool operator<(const KeyType& a) const
  {
    return Lt(a);
  }

  bool operator<=(const KeyType& a) const
  {
    return Lt(a) || Eq(a);
  }

  bool operator==(const KeyType& a) const
  {
    return Eq(a);
  }

  KeyType& operator=(const KeyType& a)
  {
    _num_insert = a._num_insert;
    iKey = a.iKey;
    mLenKey = a.mLenKey;
    is_AtMem = a.is_AtMem;
    this->set_sKey(a.sKey, a.mLenKey);
    return *this;
  }
};
extern KeyType KeyNULL;

class mBlockLink {
  public:
  long long int preBlockAddr;
  long long int nextBlockAddr;
  void mBlockLink_Initial()
  {
    preBlockAddr = -1;
    nextBlockAddr = -1;
  }
  mBlockLink& operator=(mBlockLink& a)
  {
    preBlockAddr = a.preBlockAddr;
    nextBlockAddr = a.nextBlockAddr;
    return *this;
  }
  mBlockLink()
  {
  }
  mBlockLink(mBlockLink& a)
  {
    preBlockAddr = a.preBlockAddr;
    nextBlockAddr = a.nextBlockAddr;
  }
};

class mValue {
  public:
  char* Term[TERM_NUMBER];
  int lenTerm[TERM_NUMBER];
  ~mValue()
  {
    for (int i = 0; i < TERM_NUMBER; i++) {
      if (Term[i] != NULL) {
        delete[] Term[i];
        Term[i] = NULL;
      }
    }
  }
  void mValue_Initial()
  {
    for (int i = 0; i < TERM_NUMBER; i++) {
      if (Term[i] != NULL) {
        cout << "term mem lost" << endl;
        system("pause");
        system("pause");
        exit(0);
      }
      Term[i] = NULL;
      lenTerm[i] = 0;
    }
  }

  void getValInteger(vector<int>& nodeSet)
  {
    { //for one order
      /*
			int _tag = 0;
			int i_size = sizeof(int);
			while(_tag + i_size <= lenTerm[0])
			{
				int _val = *( (int*)(Term[0]+_tag) );
				nodeSet.push_back(_val);
				_tag += 5;
			}
			*/
    }
    { //for the other order
      int _tag = lenTerm[0];
      int i_size = sizeof(int);
      while (_tag - i_size >= 0) {
        int _val = *((int*)(Term[0] + _tag - i_size));
        nodeSet.push_back(_val);
        _tag -= 5;
      }
    }
    return;
  }

  void iPrintval(int _i)
  {
    if (_i < 0 || _i > TERM_NUMBER - 1) {
      cout << "bug ipval" << endl;
      system("pause");
      exit(0);
    }
    if (Term[_i] == NULL) {
      cout << "bug ipvalt" << endl;
      system("pause");
      exit(0);
    }
    cout << Term[_i] << endl;
  }
  //think
  mValue& operator=(const mValue& a)
  {
    for (int i = 0; i < TERM_NUMBER; i++) {
      lenTerm[i] = a.lenTerm[i];
      if (Term[i] != NULL) {
        delete[] Term[i];
        Term[i] = NULL;
      }
      Term[i] = new char[lenTerm[i] + 1];
      memcpy(Term[i], a.Term[i], lenTerm[i] + 1);
    }
    return *this;
  }
  mValue()
  {
    for (int i = 0; i < TERM_NUMBER; i++) {
      Term[i] = NULL;
      lenTerm[i] = 0;
    }
  }
  //	think
  mValue(mValue& a)
  {
    for (int i = 0; i < TERM_NUMBER; i++) {
      lenTerm[i] = a.lenTerm[i];
      if (Term[i] != NULL) {
        delete[] Term[i];
        Term[i] = NULL;
      }
      Term[i] = new char[lenTerm[i] + 1];
      memcpy(Term[i], a.Term[i], lenTerm[i] + 1);
    }
  }

  void set_iVal(int _i, const char* _str, int _str_len)
  {
    if (_i < 0 || _i > TERM_NUMBER - 1) {
      cout << "bug setival" << endl;
      system("pause");
      exit(0);
    }
    if (Term[_i] != NULL) {
      delete[] Term[_i];
      Term[_i] = NULL;
    }

    Term[_i] = new char[_str_len + 1];
    memcpy(Term[_i], _str, _str_len + 1);
    lenTerm[_i] = _str_len;

    Term[_i][lenTerm[_i]] = '\0';
  }

  bool delete_iVal(int _i, char* del_str, int& flag)
  {
    if (_i < 0 || _i > TERM_NUMBER - 1) {
      cout << "bug setival" << endl;
      system("pause");
      exit(0);
    }
    if (Term[_i] == NULL) {
      cout << "bug addival" << endl;
      system("pause");
      exit(0);
    }

    int _tag = 0;
    int i_size = sizeof(int);
    while (_tag + i_size <= lenTerm[0]) {
      if (del_str[0] == Term[0][_tag] && del_str[1] == Term[0][_tag + 1] && del_str[2] == Term[0][_tag + 2]
          && del_str[3] == Term[0][_tag + 3])
        break;
      _tag += 5;
    }
    if (_tag + i_size > lenTerm[0]) {
      flag = FLAG_NO_ZERO;
      return false;
    }

    if (_tag + i_size == lenTerm[0]) {
      Term[0][_tag] = '\0';
      lenTerm[0] -= sizeof(int) + sizeof(char);
      //只有一个元素时很特殊， 长度是减少4， 其余减少5
      if (lenTerm[0] <= 0)
        flag = FLAG_ZERO;
      else
        flag = FLAG_NO_ZERO;
      return true;
    }

    for (int i = _tag; i + i_size < lenTerm[0]; i++) {
      Term[0][i] = Term[0][i + i_size + 1];
    }
    lenTerm[0] -= sizeof(int) + sizeof(char);
    flag = FLAG_NO_ZERO;
    return true;
  }

  void add_iVal(int _i, const char* add_str)
  {
    if (_i < 0 || _i > TERM_NUMBER - 1) {
      cout << "bug setival" << endl;
      system("pause");
      exit(0);
    }
    if (Term[_i] == NULL) {
      cout << "bug addival" << endl;
      system("pause");
      exit(0);
    }
    char* _tmp_term_i = NULL;

    int _tag = 0;
    int i_size = sizeof(int);
    while (_tag + i_size <= lenTerm[0]) {
      if (add_str[0] == Term[0][_tag] && add_str[1] == Term[0][_tag + 1] && add_str[2] == Term[0][_tag + 2]
          && add_str[3] == Term[0][_tag + 3])
        return;
      _tag += 5;
    }

    int newLen = sizeof(int) + sizeof(char) + lenTerm[_i];

    _tmp_term_i = new char[newLen + 1];

    memcpy(_tmp_term_i, add_str, 4 + 1);
    _tmp_term_i[4] = '	';
    memcpy(_tmp_term_i + 5, Term[_i], lenTerm[_i] + 1);
    lenTerm[0] = newLen;
    _tmp_term_i[lenTerm[_i]] = '\0';

    if (Term[_i] != NULL) {
      delete[] Term[_i];
      Term[_i] = NULL;
    } else {
      cout << "bug term" << endl;
      system("pause");
      exit(0);
    }
    Term[_i] = _tmp_term_i;
  }
  void InputVal()
  {
    char _input_str[1000] = "helloworld";
    for (int i = 0; i < TERM_NUMBER; i++) {
      lenTerm[i] = (int)strlen(_input_str);
      Term[i] = new char[lenTerm[i] + 1];
      strcpy(Term[i], _input_str);
      Term[i][lenTerm[i]] = '\0';
    }
  }

  void FputVal(FILE* fp)
  {
  }
  void ReadVal(FILE* fp);
  void WriteVal(FILE* fp);
  int WriteSize()
  {
    int _sum_size = 0;
    for (int i = 0; i < TERM_NUMBER; i++) {
      _sum_size += lenTerm[i] + 1;
    }
    _sum_size += TERM_NUMBER * (sizeof(int));
    return _sum_size;
  }
  void PrintVal()
  {
    for (int i = 0; i < TERM_NUMBER; i++) {
      if (Term[i] != NULL)
        printf("%s == ", Term[i]);
      cout << "len = " << lenTerm[i] << endl;
    }
  }
};

class mQueue {
  public:
  //	static const int qLenth = 5243005; /* 5*1024*1024 */
  static const int qLenth = 20971520; /* 20*1024*1024 */
  int qUsed;
  bool qAvailable[mQueue::qLenth];

  public:
  mQueue()
  {
    Initial();
  }

  void Initial()
  {
    qAvailable[0] = false;
    qUsed = 1;
    for (int i = 1; i < mQueue::qLenth; i++) {
      qAvailable[i] = true;
    }
  }

  void Push(long long int _iq)
  {
    if (_iq % BLOCKSIZE != 0) {
      printf("bug in _iq\n");
      system("pause");
      exit(0);
    }
    _iq /= BLOCKSIZE;
    if (qAvailable[_iq] || _iq >= mQueue::qLenth) // for check
    {
      printf("err in queue_insert: qLenth = %d\n", mQueue::qLenth);
      _iq %= mQueue::qLenth;
    }
    //		cout << "block back: " << (_iq * BLOCKSIZE) << endl;
    qAvailable[_iq] = true;
    qUsed--;
  }

  void MulPush(long long int _iq[], int _qlen)
  {
    for (int i = 0; i < _qlen; i++) {
      Push(_iq[i]);
    }
  }

  void MulPush(const vector<long long int>& _back_vec)
  {
    vector<long long int>::const_iterator citr;
    for (citr = _back_vec.begin(); citr != _back_vec.end(); citr++) {
      this->Push(*citr);
    }
  }

  long long int Pop()
  {
    for (int i = 0; i < mQueue::qLenth; i++) {
      if (qAvailable[i]) {
        long long int lli_tmp = i;
        lli_tmp = lli_tmp * BLOCKSIZE;
        //				cout << "block out: " << (lli_tmp) << endl;
        qAvailable[i] = false;
        qUsed++;
        return lli_tmp;
      }
    }
    printf("err in queue_pop\n");
    system("pause");
    exit(0);
  }

  void ReadQueue(char* _queuefile)
  {
    FILE* fp;
    if ((fp = fopen(_queuefile, "rb+")) == NULL) {
      printf("err in open r_queuefile\n");
      system("pause");
      exit(0);
    } else {
      fread(&qUsed, sizeof(int), 1, fp);
      fread(qAvailable, sizeof(bool), mQueue::qLenth, fp);
      //			printf("qLenth = %d\n", qLenth);
    }
    fclose(fp);
  }

  void WriteQueue(char* _queuefile)
  {
    FILE* fp;
    if ((fp = fopen(_queuefile, "wb+")) == NULL) {
      printf("err in open w_queuefile\n");
      system("pause");
      exit(0);
      return;
    } else {
      fwrite(&qUsed, sizeof(int), 1, fp);
      fwrite(qAvailable, sizeof(bool), mQueue::qLenth, fp);
    }
    fclose(fp);
  }

  int BlockUsed()
  {
    return qUsed;
  }
};

class mLeafNode;
class mItnlNode;
class BPlusTree;

class mNode {
  public:
  void mNode_Initial()
  {
    Modify = true;
    Count = 0;
    AddrFB = -1;
    mFather = NULL;
  }
  virtual ~mNode()
  {
  }

  void setModify()
  {
    Modify = true;
  }
  bool getModify()
  {
    return Modify;
  }
  void unModify()
  {
    Modify = false;
  }
  int getCount()
  {
    return Count;
  }
  void setCount(int _count)
  {
    Count = _count;
  }
  void IncCount()
  {
    Count++;
  }
  void DecCount()
  {
    Count--;
  }
  long long int getAddrFB()
  {
    return AddrFB;
  }
  void setAddrFB(long long int _addrfb)
  {
    AddrFB = _addrfb;
  }
  NODE_TYPE getType()
  {
    return Type;
  }
  void setType(NODE_TYPE _type)
  {
    Type = _type;
  }
  mNode* getFather()
  {
    return mFather;
  }
  void setFather(mNode* _mpather)
  {
    mFather = _mpather;
  }
  virtual mNode* getPointer(int _i)
  {
    return NULL;
  }
  virtual void setPointer(int _i, mNode* _mnode)
  {
  }
  virtual bool isInMemory(int _i)
  {
    return false;
  }
  virtual void setMemory(int _i)
  {
  }
  virtual int iInsert(KeyType& _keytype)
  {
    return 0;
  }
  virtual KeyType& getKey(int _i)
  {
    return KeyNULL;
  }
  virtual void setKey(int _i, KeyType& _keytype)
  {
  }
  virtual void printNode()
  {
    return;
  }

  mNode* getBrother(int& flag)
  {
    mNode* _pFather = this->getFather();
    if (_pFather == NULL)
      return NULL;

    mNode* _pBrother = NULL;
    for (int i = 1; i <= _pFather->getCount(); i++) {
      //ָ��ƥ��
      if (_pFather->getPointer(i) == this) {
        if (i == (_pFather->getCount()) + 1) {
          flag = FLAG_LEFT;
          _pBrother = _pFather->getPointer(i - 1);
        } else {
          flag = FLAG_RIGHT;
          _pBrother = _pFather->getPointer(i + 1);
        }
        break;
      }
    }
    if (_pBrother == NULL) {
      cout << "bug run here" << endl;
    }
    return _pBrother;
  }

  mItnlNode* ifordebug;
  mLeafNode* lfordebug;

  private:
  int Count;
  long long int AddrFB;
  bool Modify;
  NODE_TYPE Type;
  mNode* mFather;
};

class mitnldata {
  public:
  KeyType mKey;
  mNode* mPointer;
  long long int mAddr_sonFB;

  void mitnldata_Initial()
  {
    mKey.KeyType_Initial();
    mPointer = NULL;
    mAddr_sonFB = -1;
  }

  mitnldata& operator=(mitnldata& a)
  {
    mKey = a.mKey;
    mPointer = a.mPointer;
    mAddr_sonFB = a.mAddr_sonFB;
    return *this;
  }
  void cleardata()
  {
    mKey.clear_skey();
  }
  ~mitnldata()
  {
  }

  mitnldata()
  {
  }
  mitnldata(mitnldata& a)
  {
    mKey = a.mKey;
    mPointer = a.mPointer;
    mAddr_sonFB = a.mAddr_sonFB;
  }

  bool operator>(const mitnldata& a)
  {
    return mKey > a.mKey;
  }
  bool operator<(const mitnldata& a)
  {
    return mKey < a.mKey;
  }

  void Hand_inputkey()
  {
    mKey.HandInputKey();
  }
  void Rand_inputkey(int _i)
  {
    mKey.RandInputKey(_i);
  }

  void Read_mitnldata(FILE* fp);
  bool Write_mitnldata(FILE* fp, int& _size_left);
};

class mItnlNode : public mNode {
  public:
  mitnldata ItnlData[MAXNUM_DATA];
  mItnlNode()
  {
    mNode_Initial();
    ifordebug = this;
    lfordebug = NULL;
    setType(NODE_TYPE_INTERNAL);
    for (int i = 0; i <= MAXNUM_KEY; i++) {
      ItnlData[i].mitnldata_Initial();
    }
  }
  ~mItnlNode()
  {
  }

  KeyType& getKey(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      return ItnlData[_i].mKey;
    } else {
      printf("err in getkey\n");
      system("pause");
      exit(0);
    }
    return KeyNULL;
  }
  void setKey(int _i, const KeyType& _keytype)
  {
    if (_i > 0 && _i <= MAXNUM_KEY) {
      ItnlData[_i].mKey = _keytype;
    } else {
      printf("err in setkey\n");
      system("pause");
      exit(0);
    }
    return;
  }

  mitnldata& getElement(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      return ItnlData[_i];
    } else {
      printf("err in getele\n");
      system("pause");
      exit(0);
    }
    return ItnlData[0];
  }

  void setElement(int _i, mitnldata& _itnldata)
  {
    if (_i > 0 && _i <= MAXNUM_KEY) {
      ItnlData[_i] = _itnldata;
    } else {
      printf("err in setele\n");
      system("pause");
      exit(0);
    }
  }

  mNode* getPointer(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      return ItnlData[_i].mPointer;
    } else {
      printf("err in getpointer\n");
      system("pause");
      exit(0);
    }
    return NULL;
  }

  void setPointer(int _i, mNode* _mpointer)
  {
    if (_i > 0 && _i <= MAXNUM_POINTER) {
      ItnlData[_i].mPointer = _mpointer;
      //			setFather !!!!!
      _mpointer->setFather(this);
    } else {
      printf("err in setpointer\n");
      system("pause");
      exit(0);
    }
  }

  //此函数需仔细考虑~~！！
  int iExist(const KeyType& _keytype)
  {
    int _ibegin = 1, _iend = getCount();
    int _imiddle;
    while (_ibegin < _iend) {
      _imiddle = (_ibegin + _iend) / 2;
      if (ItnlData[_ibegin].mKey == _keytype) {
        return _ibegin;
      }
      if (ItnlData[_iend].mKey == _keytype) {
        return _iend;
      }

      if (_ibegin == _iend - 1)
        return -1;
      // 后加可能有风险

      if (ItnlData[_imiddle].mKey > _keytype) {
        _iend = _imiddle;
      } else {
        _ibegin = _imiddle;
      }
    }
    return -1;
  }

  int iInsert(const KeyType& _keytype)
  {
    int _ibegin = 1, _iend = getCount();
    int _imiddle;
    while (_ibegin < _iend) {
      _imiddle = (_ibegin + _iend) / 2;
      if (ItnlData[_iend].mKey < _keytype) {
        return _iend + 1;
      }
      if (ItnlData[_ibegin].mKey < _keytype && ItnlData[_ibegin + 1].mKey > _keytype) {
        return _ibegin + 1;
      }
      if (ItnlData[_imiddle].mKey > _keytype) {
        _iend = _imiddle;
      } else {
        _ibegin = _imiddle;
      }
    }
    return -1;
  }
  int iNextFloor(const KeyType& _keytype)
  {
    int _ibegin, _iend, _imiddle;
    _ibegin = 1;
    _iend = getCount();

    if (ItnlData[_ibegin].mKey > _keytype) {
      //			printf("less than _ibegin\n");
      return -1;
    }

    while (_ibegin < _iend) {
      _imiddle = (_ibegin + _iend) / 2;
      if (ItnlData[_iend].mKey <= _keytype) {
        return _iend;
      }
      if (ItnlData[_ibegin].mKey <= _keytype && ItnlData[_ibegin + 1].mKey > _keytype) {
        return _ibegin;
      }
      if (ItnlData[_imiddle].mKey > _keytype) {
        _iend = _imiddle;
      } else {
        _ibegin = _imiddle;
      }
    }
    return -1;
  }
  bool isInMemory(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      if (ItnlData[_i].mKey.is_AtMem)
        return true;
      else
        return false;
    } else {
      printf("err in isinmemeory\n");
      system("pause");
      exit(0);
    }
  }
  void setMemory(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      ItnlData[_i].mKey.is_AtMem = true;
    } else {
      printf("err in setmemeory\n");
      system("pause");
      exit(0);
    }
  }

  long long int getSonAddr(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      return ItnlData[_i].mAddr_sonFB;
    } else {
      printf("err in getsonaddr\n");
      system("pause");
      exit(0);
    }
  }
  void setSonAddr(int _i, long long int _addr_son)
  {
    if (_i > 0 && _i <= MAXNUM_KEY) {
      ItnlData[_i].mAddr_sonFB = _addr_son;
    } else {
      printf("err in getsonaddr\n");
      system("pause");
      exit(0);
    }
  }

  void printNode()
  {
    printf("  ==  ");
    for (int i = 1; i <= getCount(); i++) {
      ItnlData[i].mKey.PrintKey();
      printf("  ");
    }
    printf("  ==  ");
  }

  // 插入键
  //  最左端递归向上
  bool Insert(mNode* _pmnode);
  // 删除键
  int Delete(const KeyType& _keytype);
  // 分裂结点
  KeyType& Split(mItnlNode* _mitnlnode);
  // 结合结点
  bool Combine(mItnlNode* _pmnode);
  // 从另一结点移一个元素到本结点
  bool MoveOneElement(mNode* _pmnode);
};

class mleafdata {
  public:
  KeyType mData;
  mValue* pVal;
  void mleafdata_Initial()
  {
    if (mData.sKey != NULL || pVal != NULL) {
      cout << "mem lost" << endl;
      system("pause");
      system("pause");
      exit(0);
    }
    mData.KeyType_Initial();
    pVal = NULL;
  }

  ~mleafdata()
  {
    cleardata();
  }
  mleafdata()
  {
    mData.KeyType_Initial();
    pVal = NULL;
  }

  void cleardata()
  {
    if (pVal != NULL) {
      delete pVal;
      pVal = NULL;
    }
    mData.clear_skey();
  }

  void assign_val_ptr(mleafdata& a)
  {
    this->pVal = a.pVal;
  }

  //	think
  mleafdata& operator=(const mleafdata& a)
  {
    mData = a.mData;
    if (pVal != NULL) {
      delete pVal;
      pVal = NULL;
    }
    pVal = new mValue;
    pVal->mValue_Initial();
    *pVal = *(a.pVal);
    return *this;
  }

  void SubIDPrint()
  {
    mData.PrintKey();
    cout << " k 2 v " << endl;
    vector<int> nSet;
    pVal->getValInteger(nSet);
    for (unsigned int i = 0; i < nSet.size(); i++) {
      cout << nSet[i] << " ";
      if (i % 50 == 0)
        cout << endl;
    }
  }
  void PrintLeafdata()
  {
    mData.PrintKey();
    cout << " k2v ";
    pVal->PrintVal();
  }
  void fPrintLeafdata(FILE* fp)
  {
    char* skey = mData.sKey;
    char* sterm = pVal->Term[0];
    fprintf(fp, "%s\n\n\n", skey);
    fprintf(fp, "%s\n\n\n", sterm);
    return;
  }

  void iPrintdata(int _i)
  {
    pVal->iPrintval(_i);
  }

  void getdataInteger(vector<int>& nodeSet)
  {
    if (pVal == NULL) {
      cout << "bug pval" << endl;
      system("pause");
      exit(0);
    }
    pVal->getValInteger(nodeSet);
  }

  void fputLeafdata(FILE* fp)
  {
    char _str[5][100];
    char _latter[500];
    int _ikey = 0;
    if (pVal != NULL) {
      delete pVal;
      pVal = NULL;
    }
    pVal = new mValue;
    pVal->mValue_Initial();
    if (_eletype == B_CHARARRAY) {
      fprintf(fp, "%s	%s	%s	%s	%s		", _str[0], _str[1], _str[2], _str[3], _str[4]);
      fgets(_latter, 500, fp);
      mData.set_sKey(_str[0], (int)strlen(_str[0]));
      for (int i = 0; i <= 3; i++)
        pVal->set_iVal(i, _str[i + 1], (int)strlen(_str[i + 1]));
      pVal->set_iVal(4, _latter, (int)strlen(_latter));
    } else if (_eletype == B_INTEGER) {
      fprintf(fp, "%d	%s	%s	%s	%s		", _ikey, _str[1], _str[2], _str[3], _str[4]);
      fgets(_latter, 500, fp);
      mData.set_iKey(_ikey);
      for (int i = 0; i <= 3; i++)
        pVal->set_iVal(i, _str[i + 1], (int)strlen(_str[i + 1]));
      pVal->set_iVal(4, _latter, (int)strlen(_latter));
    } else {
      cout << "bug fput" << endl;
      system("pause");
      exit(0);
    }
  }

  void set_iKey_sValue(int _ikey, char _sval[])
  {
    mData.set_iKey(_ikey);
  }

  void set_sKey_sValue(const char _skey[], int _klen, const char _sval[], int val_len)
  {
    if (pVal != NULL) {
      delete pVal;
      pVal = NULL;
    }
    pVal = new mValue;
    pVal->mValue_Initial();
    ;
    mData.set_sKey(_skey, _klen);
    pVal->set_iVal(0, _sval, val_len);
  }

  bool deletePartVal(char PartVal[], int& flag)
  {
    if (pVal == NULL) {
      cout << "null error" << endl;
      system("pause");
      exit(0);
    }
    return pVal->delete_iVal(0, PartVal, flag);
  }

  void inputLeafdata()
  {
    if (pVal == NULL) {
      pVal = new mValue;
    }
    pVal->InputVal();
  }

  //	think
  mleafdata(mleafdata& a)
  {
    mData = a.mData;
    pVal = new mValue;
    pVal->mValue_Initial();
    *pVal = *(a.pVal);
  }

  bool operator>(const mleafdata& a)
  {
    return mData > a.mData;
  }
  bool operator<(const mleafdata& a)
  {
    return mData < a.mData;
  }

  void Hand_inputkey()
  {
    mData.HandInputKey();
  }
  void Rand_inputkey(int i)
  {
    mData.RandInputKey(i);
  }

  bool Read_mleafdata(FILE* fp);
  bool Write_mleafdata(FILE* fp, int& _size_left, int& _key_size, int& _val_size);
};

class mLeafNode : public mNode {
  public:
  mLeafNode* preNode;
  mLeafNode* nextNode;
  mBlockLink leafLink;
  mleafdata LeafData[MAXNUM_DATA];

  mLeafNode()
  {
    mNode_Initial();
    lfordebug = this;
    ifordebug = NULL;
    preNode = NULL;
    nextNode = NULL;
    leafLink.mBlockLink_Initial();
    setType(NODE_TYPE_LEAF);
    for (int i = 0; i <= MAXNUM_KEY; i++) {
      LeafData[i].mleafdata_Initial();
    }
  }
  ~mLeafNode()
  {
  }
  //
  //	~mLeafNode()
  //	{
  //		cout << "call leaf" << this ->getCount() << endl;
  //		for(int i = 1; i <= this ->getCount(); i ++)
  //		{
  //			LeafData[i].cleardata();
  //		}
  //		cout << "here1328" << endl;
  //	}

  KeyType& getKey(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      return LeafData[_i].mData;
    } else {
      printf("err in leaf getkey\n");
      system("pause");
      exit(0);
    }
    return KeyNULL;
  }

  void setKey(int _i, KeyType& _keytype)
  {
    if (_i > 0 && _i <= MAXNUM_KEY) {
      LeafData[_i].mData = _keytype;
    } else {
      printf("err in leaf setkey\n");
      system("pause");
      exit(0);
    }
  }

  mleafdata& getElement(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      return LeafData[_i];
    } else {
      printf("err in getele\n");
      system("pause");
      exit(0);
    }
  }

  void setElement(int _i, const mleafdata& _leafdata)
  {
    if (_i > 0 && _i <= MAXNUM_KEY) {
      LeafData[_i] = _leafdata;
    } else {
      printf("err in setele\n");
      system("pause");
      exit(0);
    }
  }

  //  此两个函数对叶节点无意义
  mNode* getPointer(int _i)
  {
    return NULL;
  }

  void setPointer(int _i, mNode* _pmnode)
  {
    return;
  }

  int iExist(const KeyType& _keytype)
  {
    int _ibegin = 1, _iend = getCount();
    int _imiddle;
    if (_iend <= 0) {
      //			cout << "not found for the bug of getCount : " << _iend << endl;
      return -1;
    }
    if (LeafData[1].mData == _keytype)
      return 1;
    while (_ibegin < _iend) {
      _imiddle = (_ibegin + _iend) / 2;
      if (LeafData[_ibegin].mData == _keytype) {
        return _ibegin;
      }
      if (LeafData[_iend].mData == _keytype) {
        return _iend;
      }
      if (_ibegin + 1 == _iend) {
        return -1;
      }
      if (LeafData[_imiddle].mData > _keytype) {
        _iend = _imiddle;
      } else {
        _ibegin = _imiddle;
      }
    }
    return -1;
  }
  //  考虑cout = 0的情况
  int iInsert(const KeyType& _keytype)
  {
    int _ibegin = 1, _iend = getCount();
    int _imiddle;
    if (_iend <= 0 || LeafData[_ibegin].mData > _keytype) {
      //			cout << "iInsert ret 1!!" << endl;
      return 1;
    }
    //		count = 0 && count = 1
    if (LeafData[_iend].mData < _keytype) {
      return _iend + 1;
    }
    while (_ibegin < _iend) {
      _imiddle = (_ibegin + _iend) / 2;
      if (LeafData[_ibegin].mData < _keytype && LeafData[_ibegin + 1].mData > _keytype) {
        return _ibegin + 1;
      }
      if (LeafData[_imiddle].mData > _keytype) {
        _iend = _imiddle;
      } else {
        _ibegin = _imiddle;
      }
    }
    return -1;
  }

  bool isInMemory(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      if (LeafData[_i].mData.is_AtMem)
        return true;
      else
        return false;
    } else {
      printf("err in setmemeory\n");
      system("pause");
      exit(0);
    }
  }
  void setMemory(int _i)
  {
    if (_i > 0 && _i <= getCount()) {
      LeafData[_i].mData.is_AtMem = true;
    } else {
      printf("err in setmemeory\n");
      system("pause");
      exit(0);
    }
  }

  void printNode()
  {
    printf("  ==  ");
    for (int i = 1; i <= getCount(); i++) {
      LeafData[i].mData.PrintKey();
      printf("  ");
    }
    printf("  ==  ");
  }

  // 插入数据
  //  最左端递归向上
  bool Insert(const mleafdata& _leafdata);
  // 删除数据
  int Delete(KeyType& _keytype);
  //重载delete partval
  int Delete(KeyType& _keytype, char partval[], int& pvFlag);
  // 分裂结点
  KeyType& Split(mLeafNode* _mpnode);
  // 结合结点
  bool Combine(mLeafNode* _mpnode);
  // 重复插入
  bool dupInsert(const mleafdata& _leafdata, int _index_insert);
};

extern mNode* ReadNode(FILE* fp, long long int _addr);
extern void WriteNode(mNode* pNode, FILE* fp, mQueue& _mqueue);
extern void DelDisk(FILE* fp, long long int _addr, mQueue& _mqueue);
extern void FP_Initial(FILE*& _fp);
extern bool DATAINPUT(FILE* ifp, char _row_str[], char _str[][600]);
extern void BuildTree();
extern bool Delete_KeyVal(char keyStr[], int keyLen, BPlusTree* _ptree);
extern bool Delete_Key_PartVal(char keyStr[], int keyLen, char partVal[], BPlusTree* _ptree);
extern bool Delete_sID2sub(int _sID, BPlusTree* _p_sID2sub);
extern bool Delete_s2sID(char _sub_str[], BPlusTree* _p_s2sID);
extern bool Delete_obj2sID(char _obj_str[], int _del_sID, BPlusTree* _p_obj2sID);
extern bool Delete_objpID2sID(char _obj_str[], int _pID, int _del_sID, BPlusTree* _p_objpID2sID);

/* B+树数据结构 */
class BPlusTree {
  public:
  // 以下两个变量用于实现双向链表
  mLeafNode* pmLeafHead; // 头结点
  mLeafNode* pmLeafTail; // 尾结点
  mNode* mRoot;          // 根结点
  mQueue mblockQueue;
  FILE* mfp;
  char mTreeName[1024];
  int mDepth; // 树的深度
  int insert_count;

  void Initial();
  BPlusTree(const char* const_tree_name, const char* _build_or_open);
  ~BPlusTree();
  string getTreeFileName();
  void log(const char* _log) const;
  void forcheck();

  // 获取和设置根结点
  mNode* getRoot()
  {
    return mRoot;
  }

  void setRoot(mNode* root)
  {
    mRoot = root;
  }
  void Flush();
  // 为插入而查找叶子结点
  mLeafNode* SearchLeafNode(const KeyType& data) const;
  //插入键到中间结点
  bool InsertItnlNode(mItnlNode* pNode, mNode* pSon);
  // 在中间结点中删除键
  bool DeleteItnlNode(mItnlNode* pNode, KeyType& key);
  // 查找指定的数据
  bool Search(KeyType& data, mleafdata& _ret);
  // 插入指定的数据
  bool Insert(const mleafdata& _mleafdata);
  // 删除指定的数据
  bool Delete(KeyType& data);
  // 重载删除函数
  bool Delete(KeyType& data, char PartVal[]);
  // 清除树
  void ClearTree();
  // 打印树
  void PrintTree();
  //读出根节点
  void ReadRoot();
  //保存树结构
  void StoreTree();
};
#endif /* CBTREE_H_ */
