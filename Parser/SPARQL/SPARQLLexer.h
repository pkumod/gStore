
// Generated from ./SPARQL.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"




class  SPARQLLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, K_NOW = 30, K_YEAR = 31, K_UNION = 32, 
    K_IF = 33, K_ASK = 34, K_ASC = 35, K_CONCAT = 36, K_IN = 37, K_UNDEF = 38, 
    K_INSERT = 39, K_MONTH = 40, K_DEFAULT = 41, K_SELECT = 42, K_FLOOR = 43, 
    K_TZ = 44, K_COPY = 45, K_CEIL = 46, K_HOURS = 47, K_DATATYPE = 48, 
    K_ISNUMERIC = 49, K_STRUUID = 50, K_CONSTRUCT = 51, K_ADD = 52, K_BOUND = 53, 
    K_NAMED = 54, K_TIMEZONE = 55, K_MIN = 56, K_ISBLANK = 57, K_UUID = 58, 
    K_BIND = 59, K_CLEAR = 60, K_INTO = 61, K_AS = 62, K_ALL = 63, K_IRI = 64, 
    K_BASE = 65, K_BY = 66, K_DROP = 67, K_LOAD = 68, K_WITH = 69, K_BNODE = 70, 
    K_WHERE = 71, K_AVG = 72, K_SAMPLE = 73, K_UCASE = 74, K_SERVICE = 75, 
    K_MINUS = 76, K_SAMETERM = 77, K_STRSTARTS = 78, K_STR = 79, K_MOVE = 80, 
    K_HAVING = 81, K_COALESCE = 82, K_STRBEFORE = 83, K_ABS = 84, K_ISLITERAL = 85, 
    K_STRAFTER = 86, K_STRLEN = 87, K_LANG = 88, K_CREATE = 89, K_DESC = 90, 
    K_MAX = 91, K_FILTER = 92, K_USING = 93, K_NOT = 94, K_STRENDS = 95, 
    K_OFFSET = 96, K_CONTAINS = 97, K_PREFIX = 98, K_MINUTES = 99, K_REPLACE = 100, 
    K_REGEX = 101, K_DELETE = 102, K_SEPARATOR = 103, K_DAY = 104, K_SILENT = 105, 
    K_STRLANG = 106, K_ORDER = 107, K_ROUND = 108, K_GRAPH = 109, K_SECONDS = 110, 
    K_URI = 111, K_DISTINCT = 112, K_EXISTS = 113, K_GROUP = 114, K_SUM = 115, 
    K_REDUCED = 116, K_FROM = 117, K_LANGMATCHES = 118, K_ISURI = 119, K_TO = 120, 
    K_ISIRI = 121, K_RAND = 122, K_STRDT = 123, K_COUNT = 124, K_DESCRIBE = 125, 
    K_VALUES = 126, K_LCASE = 127, K_OPTIONAL = 128, K_LIMIT = 129, K_SUBSTR = 130, 
    K_SIMPLECYCLEPATH = 131, K_SIMPLECYCLEBOOLEAN = 132, K_CYCLEPATH = 133, 
    K_CYCLEBOOLEAN = 134, K_SHORTESTPATH = 135, K_SHORTESTPATHLEN = 136, 
    K_KHOPREACHABLE = 137, K_KHOPENUMERATE = 138, K_KHOPREACHABLEPATH = 139, 
    K_PPR = 140, K_TRIANGLECOUNTING = 141, K_CLOSENESSCENTRALITY = 142, 
    K_BFSCOUNT = 143, K_PR = 144, K_ALPHA = 145, K_MAXITER = 146, K_TOL = 147, 
    K_SSSP = 148, K_SSSPLEN = 149, K_LABELPROP = 150, K_WCC = 151, K_CLUSTERCOEFF = 152, 
    K_MAXIMUMCLIQUE = 153, K_PFN = 154, KK_INSERTDATA = 155, KK_DELETEDATA = 156, 
    KK_DELETEWHERE = 157, KK_ENCODE_FOR_URI = 158, KK_MD5 = 159, KK_SHA1 = 160, 
    KK_SHA256 = 161, KK_SHA384 = 162, KK_SHA512 = 163, KK_GROUP_CONCAT = 164, 
    IRIREF = 165, PNAME_NS = 166, PNAME_LN = 167, BLANK_NODE_LABEL = 168, 
    VAR1 = 169, VAR2 = 170, LANGTAG = 171, INTEGER = 172, DECIMAL = 173, 
    DOUBLE = 174, INTEGER_POSITIVE = 175, DECIMAL_POSITIVE = 176, DOUBLE_POSITIVE = 177, 
    INTEGER_NEGATIVE = 178, DECIMAL_NEGATIVE = 179, DOUBLE_NEGATIVE = 180, 
    EXPONENT = 181, STRING_LITERAL1 = 182, STRING_LITERAL2 = 183, STRING_LITERAL_LONG1 = 184, 
    STRING_LITERAL_LONG2 = 185, ECHAR = 186, NIL = 187, WS = 188, ANON = 189, 
    PN_CHARS_BASE = 190, PN_CHARS_U = 191, VARNAME = 192, PN_CHARS = 193, 
    PN_PREFIX = 194, PN_LOCAL = 195, PLX = 196, PERCENT = 197, HEX = 198, 
    PN_LOCAL_ESC = 199, COMMENT = 200
  };

  SPARQLLexer(antlr4::CharStream *input);
  ~SPARQLLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

