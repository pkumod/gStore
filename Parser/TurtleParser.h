#ifndef H_tools_rdf3xload_TurtleParser
#define H_tools_rdf3xload_TurtleParser
//---------------------------------------------------------------------------
// RDF-3X
// (c) 2008 Thomas Neumann. Web site: http://www.mpi-inf.mpg.de/~neumann/rdf3x
//
// This work is licensed under the Creative Commons
// Attribution-Noncommercial-Share Alike 3.0 Unported License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/
// or send a letter to Creative Commons, 171 Second Street, Suite 300,
// San Francisco, California, 94105, USA.
//---------------------------------------------------------------------------
#include "Type.h"
#include "../Util/Util.h"
//---------------------------------------------------------------------------
/// Parse a turtle file
class TurtleParser {
  public:
  /// A parse error
  class Exception {
public:
    /// The message
    std::string message;

    /// Constructor
    Exception(const std::string& message);
    /// Constructor
    Exception(const char* message);
    /// Destructor
    ~Exception();
  };

  private:
  /// A turtle lexer
  class Lexer {
public:
    /// Possible tokens
    enum Token {
      Token_Eof,
      Token_Dot,
      Token_Colon,
      Token_Comma,
      Token_Semicolon,
      Token_LBracket,
      Token_RBracket,
      Token_LParen,
      Token_RParen,
      Token_At,
      Token_Type,
      Token_Integer,
      Token_Decimal,
      Token_Double,
      Token_Name,
      Token_A,
      Token_True,
      Token_False,
      Token_String,
      Token_URI
    };

private:
    /// The input
    std::istream& in;
    /// The putback
    Token putBack;
    /// The putback string
    std::string putBackValue;
    /// Buffer for parsing when ignoring the value
    std::string ignored;
    /// The current line
    unsigned line;

    /// Size of the read buffer
    static const unsigned readBufferSize = 1024;
    /// Read buffer
    char readBuffer[readBufferSize];
    /// Read buffer pointers
    char* readBufferStart, *readBufferEnd;

    /// Read new characters
    bool doRead(char& c);
    /// Read a character
    bool read(char& c)
    {
      if (readBufferStart < readBufferEnd) {
        c = *(readBufferStart++);
        return true;
      } else
        return doRead(c);
    }
    /// Unread the last character
    void unread()
    {
      readBufferStart--;
    }

    /// Lex a hex code
    unsigned lexHexCode(unsigned len);
    /// Lex an escape sequence
    void lexEscape(std::string& token);
    /// Lex a long string
    Token lexLongString(std::string& token);
    /// Lex a string
    Token lexString(std::string& token, char c);
    /// Lex a URI
    Token lexURI(std::string& token, char c);
    /// Lex a number
    Token lexNumber(std::string& token, char c);

public:
    /// Constructor
    Lexer(std::istream& in);
    /// Destructor
    ~Lexer();

    /// The next token (including value)
    Token next(std::string& value);
    /// The next token (ignoring the value)
    Token next()
    {
      return next(ignored);
    }
    /// Put a token and a string back
    void unget(Token t, const std::string& s)
    {
      putBack = t;
      if (t >= Token_Integer)
        putBackValue = s;
    }
    /// Put a token back
    void ungetIgnored(Token t)
    {
      putBack = t;
      if (t >= Token_Integer)
        putBackValue = ignored;
    }
    /// Get the line
    unsigned getLine() const
    {
      return line;
    }

    void readUntilSep(std::string& value);
    void discardLine()
    {
      char c;
      while (read(c) && c != '\n')
        ;
    }
  };
  /// A triple
  struct Triple {
    /// The entries
    std::string subject, predicate, object, objectSubType;
    /// Type for the object
    Type::Type_ID objectType;

    /// Constructor
    Triple(const std::string& subject, const std::string& predicate, const std::string& object, Type::Type_ID objectType, const std::string& objectSubType)
        : subject(subject)
        , predicate(predicate)
        , object(object)
        , objectSubType(objectSubType)
        , objectType(objectType)
    {
    }
  };

  /// The lexer
  Lexer lexer;
  /// The uri base
  std::string base;
  /// All known prefixes
  std::map<std::string, std::string> prefixes;
  /// The currently available triples
  std::vector<Triple> triples;
  /// Reader in the triples
  unsigned triplesReader;
  /// The next blank node id
  unsigned nextBlank;

  /// Is a (generalized) name token?
  static inline bool isName(Lexer::Token token);

  // Convert a relative URI into an absolute one
  void constructAbsoluteURI(std::string& uri);
  /// Construct a new blank node
  void newBlankNode(std::string& node);
  /// Report an error
  void parseError(const std::string& message);
  /// Parse a qualified name
  void parseQualifiedName(const std::string& prefix, std::string& name);
  /// Parse a blank entry
  void parseBlank(std::string& entry);
  /// Parse a subject
  void parseSubject(Lexer::Token token, std::string& subject);
  /// Parse an object
  void parseObject(std::string& object, Type::Type_ID& objectType, std::string& objectSubType);
  /// Parse a predicate object list
  void parsePredicateObjectList(const std::string& subject, std::string& predicate, std::string& object, Type::Type_ID& objectType, std::string& objectSubType);
  /// Parse a directive
  void parseDirective();
  /// Parse a new triple
  void parseTriple(Lexer::Token token, std::string& subject, std::string& predicate, std::string& object, Type::Type_ID& objectType, std::string& objectSubType);

  public:
  /// Constructor
  TurtleParser(std::istream& in);
  /// Destructor
  ~TurtleParser();

  /// Read the next triple
  bool parse(std::string& subject, std::string& predicate, std::string& object, Type::Type_ID& objectType, std::string& objectSubType);

  void discardLine()
  {
    lexer.discardLine();
  }
};
//---------------------------------------------------------------------------
#endif
