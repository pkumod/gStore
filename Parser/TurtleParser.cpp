#include "TurtleParser.h"
#include <sstream>
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
using namespace std;
//---------------------------------------------------------------------------
TurtleParser::Exception::Exception(const std::string& message)
   : message(message)
   // Constructor
{
}
//---------------------------------------------------------------------------
TurtleParser::Exception::Exception(const char* message)
   : message(message)
   // Constructor
{
}
//---------------------------------------------------------------------------
TurtleParser::Exception::~Exception()
      // Destructor
{
}
//---------------------------------------------------------------------------
TurtleParser::Lexer::Lexer(istream& in)
   : in(in),putBack(Eof),line(1),readBufferStart(0),readBufferEnd(0)
   // Constructor
{
}
//---------------------------------------------------------------------------
TurtleParser::Lexer::~Lexer()
   // Destructor
{
}
//---------------------------------------------------------------------------
bool TurtleParser::Lexer::doRead(char& c)
   // Read new characters
{
   while (in) {
      readBufferStart=readBuffer;
      in.read(readBuffer,readBufferSize);
      if (!in.gcount()) return false;
      readBufferEnd=readBufferStart+in.gcount();

      if (readBufferStart<readBufferEnd) {
         c=*(readBufferStart++);
         return true;
      }
   }
   return false;
}
//---------------------------------------------------------------------------
static bool issep(char c) { return (c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='[')||(c==']')||(c=='(')||(c==')')||(c==',')||(c==';')||(c==':')||(c=='.'); }
//---------------------------------------------------------------------------
TurtleParser::Lexer::Token TurtleParser::Lexer::lexNumber(std::string& token,char c)
   // Lex a number
{
   token.resize(0);

   while (true) {
      // Sign?
      if ((c=='+')||(c=='-')) {
         token+=c;
         if (!read(c)) break;
      }

      // First number block
      if (c!='.') {
         if ((c<'0')||(c>'9')) break;
         while ((c>='0')&&(c<='9')) {
            token+=c;
            if (!read(c)) return Integer;
         }
         if (c != '.' && issep(c)) {
            unread();
            return Integer;
         }
      }

      // Dot?
      if (c=='.') {
         token+=c;
         if (!read(c)) break;
         // Second number block
         while ((c>='0')&&(c<='9')) {
            token+=c;
            if (!read(c)) return Decimal;
         }
         if (issep(c)) {
            unread();
            return Decimal;
         }
      }

      // Exponent
      if ((c!='e')&&(c!='E')) break;
      token+=c;
      if (!read(c)) break;
      if ((c=='-')||(c=='+')) {
         token+=c;
         if (!read(c)) break;
      }
      if ((c<'0')||(c>'9')) break;
      while ((c>='0')&&(c<='9')) {
         token+=c;
         if (!read(c)) return Double;
      }
      if (issep(c)) {
         unread();
         return Double;
      }
      break;
   }
   // REMARK: the string that start with number will throw exception: invalid Number   
   if (((c>='A')&&(c<='Z'))||((c>='a')&&(c<='z'))||(c=='_')) { // XXX unicode!
      token+=c;
      while (read(c)) {
         if (issep(c)) { unread(); break; }
         token+=c;
      }
      return Name;
   } else {
      stringstream msg;
      msg << "lexer error in line " << line << ": invalid number " << token << c;
      throw Exception(msg.str());
   }
}
//---------------------------------------------------------------------------
unsigned TurtleParser::Lexer::lexHexCode(unsigned len)
   // Parse a hex code
{
   unsigned result=0;
   for (unsigned index=0;;index++) {
      // Done?
      if (index==len) return result;

      // Read the next char
      char c;
      if (!read(c)) break;

      // Interpret it
      if ((c>='0')&&(c<='9')) result=(result<<4)|(c-'0'); else
      if ((c>='A')&&(c<='F')) result=(result<<4)|(c-'A'+10); else
      if ((c>='a')&&(c<='f')) result=(result<<4)|(c-'a'+10); else
         break;
   }
   stringstream msg;
   msg << "lexer error in line " << line << ": invalid unicode escape";
   throw Exception(msg.str());
}
//---------------------------------------------------------------------------
static string encodeUtf8(unsigned code)
   // Encode a unicode character as utf8
{
   string result;
   if (code&&(code<0x80)) {
      result+=static_cast<char>(code);
   } else if (code<0x800) {
      result+=static_cast<char>(0xc0 | (0x1f & (code >> 6)));
      result+=static_cast<char>(0x80 | (0x3f & code));
   } else {
      result+=static_cast<char>(0xe0 | (0x0f & (code >> 12)));
      result+=static_cast<char>(0x80 | (0x3f & (code >>  6)));
      result+=static_cast<char>(0x80 | (0x3f & code));
   }
   return result;
}
//---------------------------------------------------------------------------
void TurtleParser::Lexer::lexEscape(std::string& token)
   // Lex an escape sequence, \ already consumed
{
   while (true) {
      char c;
      if (!read(c)) break;
      // Standard escapes?
      if (c=='t') { token+='\t'; return; }
      if (c=='n') { token+='\n'; return; }
      if (c=='r') { token+='\r'; return; }
      if (c=='\"') { token+='\"'; return; }
      if (c=='>') { token+='>'; return; }
      if (c=='\\') { token+='\\'; return; }

      // Unicode sequences?
      if (c=='u') {
         unsigned code=lexHexCode(4);
         token+=encodeUtf8(code);
         return;
      }
      if (c=='U') {
         unsigned code=lexHexCode(8);
         token+=encodeUtf8(code);
         return;
      }

      // Invalid escape
      break;
   }
   stringstream msg;
   msg << "lexer error in line " << line << ": invalid escape sequence";
   throw Exception(msg.str());
}
//---------------------------------------------------------------------------
TurtleParser::Lexer::Token TurtleParser::Lexer::lexLongString(std::string& token)
   // Lex a long string, first """ already consumed
{
   char c;
   while (read(c)) {
      if (c=='\"') {
         if (!read(c)) break;
         if (c!='\"') { token+='\"'; continue; }
         if (!read(c)) break;
         if (c!='\"') { token+="\"\""; continue; }
         return String;
      }
      if (c=='\\') {
         lexEscape(token);
      } else {
         token+=c;
         if (c=='\n') line++;
      }
   }
   stringstream msg;
   msg << "lexer error in line " << line << ": invalid string";
   throw Exception(msg.str());
}
//---------------------------------------------------------------------------
TurtleParser::Lexer::Token TurtleParser::Lexer::lexString(std::string& token,char c)
   // Lex a string
{
   token.resize(0);

   // Check the next character
   if (!read(c)) {
      stringstream msg;
      msg << "lexer error in line " << line << ": invalid string";
      throw Exception(msg.str());
   }

   // Another quote?
   if (c=='\"') {
      if (!read(c))
         return String;
      if (c=='\"')
         return lexLongString(token);
      unread();
      return String;
   }

   // Process normally
   while (true) {
      if (c=='\"') return String;
      if (c=='\\') {
         lexEscape(token);
      } else {
         token+=c;
         if (c=='\n') line++;
      }
      if (!read(c)) {
         stringstream msg;
         msg << "lexer error in line " << line << ": invalid string";
         throw Exception(msg.str());
      }
   }
}
//---------------------------------------------------------------------------
TurtleParser::Lexer::Token TurtleParser::Lexer::lexURI(std::string& token,char c)
   // Lex a URI
{
   token.resize(0);

   // Check the next character
   if (!read(c)) {
      stringstream msg;
      msg << "lexer error in line " << line << ": invalid URI";
      throw Exception(msg.str());
   }

   // Process normally
   while (true) {
      if (c=='>') return URI;
      if (c=='\\') {
         lexEscape(token);
      } else {
         token+=c;
         if (c=='\n') line++;
      }
      if (c==' ' || !read(c)) {
         stringstream msg;
         msg << "lexer error in line " << line << ": invalid URI";
         throw Exception(msg.str());
      }
   }
}
//---------------------------------------------------------------------------
TurtleParser::Lexer::Token TurtleParser::Lexer::next(std::string& token)
   // Get the next token
{
   // Do we already have one?
   if (putBack!=Eof) {
      Token result=putBack;
      token=putBackValue;
      putBack=Eof;
      return result;
   }

   // Read more
   char c;
   while (read(c)) {
      switch (c) {
         case ' ': case '\t': case '\r': continue;
         case '\n': line++; continue;
         case '#': while (read(c)) if ((c=='\n')||(c=='\r')) break; if (c=='\n') ++line; continue;
         case '.': if (!read(c)) return Dot; unread(); if ((c>='0')&&(c<='9')) return lexNumber(token,'.'); return Dot;
         case ':': return Colon;
         case ';': return Semicolon;
         case ',': return Comma;
         case '[': return LBracket;
         case ']': return RBracket;
         case '(': return LParen;
         case ')': return RParen;
         case '@': return At;
         case '+': case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return lexNumber(token,c);
         case '^':
            if ((!read(c))||(c!='^')) {
               stringstream msg;
               msg << "lexer error in line " << line << ": '^' expected";
               throw Exception(msg.str());
            }
            return Type;
         case '\"': return lexString(token,c);
         case '<': return lexURI(token,c);
         default:
            if (((c>='A')&&(c<='Z'))||((c>='a')&&(c<='z'))||(c=='_')) { // XXX unicode!
               token=c;
               while (read(c)) {
                  if (issep(c)) { unread(); break; }
                  token+=c;
               }
               if (token=="a") return A;
               if (token=="true") return True;
               if (token=="false") return False;
               return Name;
            } else {
               stringstream msg;
               msg << "lexer error in line " << line << ": unexpected character " << c;
               throw Exception(msg.str());
            }
      }
   }

   return Eof;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TurtleParser::TurtleParser(istream& in)
   : lexer(in),triplesReader(0),nextBlank(0)
   // Constructor
{
}
//---------------------------------------------------------------------------
TurtleParser::~TurtleParser()
   // Destructor
{
}
//---------------------------------------------------------------------------
void TurtleParser::parseError(const string& message)
   // Report an error
{
   stringstream msg;
   msg << "parse error in line " << lexer.getLine() << ": " << message;
   throw Exception(msg.str());
}
//---------------------------------------------------------------------------
void TurtleParser::newBlankNode(std::string& node)
   // Construct a new blank node
{
   stringstream buffer;
   buffer << "_:_" << (nextBlank++);
   node=buffer.str();
}
//---------------------------------------------------------------------------
void TurtleParser::constructAbsoluteURI(std::string& uri)
   // Convert a relative URI into an absolute one
{
   // No base?
   if (base.empty())
      return;

   // Already absolute? XXX fix the check!
   if (uri.find("://")<10)
      return;

   // Put the base in front
   uri=base+uri;
}
//---------------------------------------------------------------------------
void TurtleParser::parseDirective()
   // Parse a directive
{
   std::string value;
   if (lexer.next(value)!=Lexer::Name)
      parseError("directive name expected after '@'");

   if (value=="base") {
      if (lexer.next(base)!=Lexer::URI)
         parseError("URI expected after @base");
   } else if (value=="prefix") {
      std::string prefixName;
      Lexer::Token token=lexer.next(prefixName);
      // A prefix name?
      if (token==Lexer::Name) {
         token=lexer.next();
      } else prefixName.resize(0);
      // Colon
      if (token!=Lexer::Colon)
         parseError("':' expected after @prefix");
      // URI
      std::string uri;
      if (lexer.next(uri)!=Lexer::URI)
         parseError("URI expected after @prefix");
      prefixes[prefixName]=uri;
   } else {
      parseError("unknown directive @"+value);
   }

   // Final dot
   if (lexer.next()!=Lexer::Dot)
      parseError("'.' expected after directive");
}
//---------------------------------------------------------------------------
inline bool TurtleParser::isName(Lexer::Token token)
   // Is a (generalized) name token?
{
   return (token==Lexer::Name)||(token==Lexer::A)||(token==Lexer::True)||(token==Lexer::False)||(token==Lexer::Integer);
}
//---------------------------------------------------------------------------
void TurtleParser::parseQualifiedName(const string& prefix,string& name)
   // Parse a qualified name
{
   if (lexer.next()!=Lexer::Colon)
      parseError("':' expected in qualified name");
   if (!prefixes.count(prefix))
      parseError("unknown prefix '"+prefix+"'");
   string expandedPrefix=prefixes[prefix];

   Lexer::Token token=lexer.next(name);
   if (isName(token)) {
      name=expandedPrefix+name;
   } else {
      lexer.unget(token,name);
      name=expandedPrefix;
   }
}
//---------------------------------------------------------------------------
void TurtleParser::parseBlank(std::string& entry)
   // Parse a blank entry
{
   Lexer::Token token=lexer.next(entry);
   switch (token) {
      case Lexer::Name:
         if ((entry!="_")||(lexer.next()!=Lexer::Colon)||(!isName(lexer.next(entry))))
            parseError("blank nodes must start with '_:'");
         entry="_:"+entry;
         return;
      case Lexer::LBracket:
         {
            newBlankNode(entry);
            token=lexer.next();
            if (token!=Lexer::RBracket) {
               lexer.ungetIgnored(token);
               std::string predicate,object,objectSubType;
               Type::ID objectType;
               parsePredicateObjectList(entry,predicate,object,objectType,objectSubType);
               triples.push_back(Triple(entry,predicate,object,objectType,objectSubType));
               if (lexer.next()!=Lexer::RBracket)
                  parseError("']' expected");
            }
            return;
         }
      case Lexer::LParen:
         {
            // Collection
            vector<string> entries,entrySubTypes;
            vector<Type::ID> entryTypes;
            while ((token=lexer.next())!=Lexer::RParen) {
               lexer.ungetIgnored(token);
               entries.push_back(string());
               entryTypes.push_back(Type::URI);
               entrySubTypes.push_back(string());
               parseObject(entries.back(),entryTypes.back(),entrySubTypes.back());
            }

            // Empty collection?
            if (entries.empty()) {
               entry="http://www.w3.org/1999/02/22-rdf-syntax-ns#nil";
               return;
            }

            // Build blank nodes
            vector<string> nodes;
            nodes.resize(entries.size());
            for (unsigned index=0;index<entries.size();index++)
               newBlankNode(nodes[index]);
            nodes.push_back("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil");

            // Derive triples
            for (unsigned index=0;index<entries.size();index++) {
               triples.push_back(Triple(nodes[index],"http://www.w3.org/1999/02/22-rdf-syntax-ns#first",entries[index],entryTypes[index],entrySubTypes[index]));
               triples.push_back(Triple(nodes[index],"http://www.w3.org/1999/02/22-rdf-syntax-ns#rest",nodes[index+1],Type::URI,""));
            }
            entry=nodes.front();
            return;
         }

      default: parseError("invalid blank entry");
   }
}
//---------------------------------------------------------------------------
void TurtleParser::parseSubject(Lexer::Token token,std::string& subject)
   // Parse a subject
{
   switch (token) {
      case Lexer::URI:
         // URI
         constructAbsoluteURI(subject);
         return;
      case Lexer::A: subject="http://www.w3.org/1999/02/22-rdf-syntax-ns#type"; return;
      case Lexer::Colon:
         // Qualified name with empty prefix?
         lexer.unget(token,subject);
         parseQualifiedName("",subject);
         return;
      case Lexer::Name:
         // Qualified name
         // Blank node?
         if (subject=="_") {
            lexer.unget(token,subject);
            parseBlank(subject);
            return;
         }
         // No
         parseQualifiedName(subject,subject);
         return;
      case Lexer::LBracket: case Lexer::LParen:
         // Opening bracket/parenthesis
         lexer.unget(token,subject);
         parseBlank(subject);
         return;
      default: parseError("invalid subject");
   }
}
//---------------------------------------------------------------------------
void TurtleParser::parseObject(std::string& object,Type::ID& objectType,std::string& objectSubType)
   // Parse an object
{
   Lexer::Token token=lexer.next(object);
   objectSubType="";
   switch (token) {
      case Lexer::URI:
         // URI
         constructAbsoluteURI(object);
         objectType=Type::URI;
         return;
      case Lexer::Colon:
         // Qualified name with empty prefix?
         lexer.unget(token,object);
         parseQualifiedName("",object);
         objectType=Type::URI;
         return;
      case Lexer::Name:
         // Qualified name
         // Blank node?
         if (object=="_") {
            lexer.unget(token,object);
            parseBlank(object);
            objectType=Type::URI;
            return;
         }
         // No
         parseQualifiedName(object,object);
         objectType=Type::URI;
         return;
      case Lexer::LBracket: case Lexer::LParen:
         // Opening bracket/parenthesis
         lexer.unget(token,object);
         parseBlank(object);
         objectType=Type::URI;
         return;
      case Lexer::Integer:
         // Literal
         objectType=Type::Integer;
         return;
      case Lexer::Decimal:
         // Literal
         objectType=Type::Decimal;
         return;
      case Lexer::Double:
         // Literal
         objectType=Type::Double;
         return;
      case Lexer::A:
         // Literal
         object="http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
         objectType=Type::URI;
         return;
      case Lexer::True:
         // Literal
         objectType=Type::Boolean;
         return;
      case Lexer::False:
         // Literal
         objectType=Type::Boolean;
         return;
      case Lexer::String:
         // String literal
         {
            token=lexer.next();
            objectType=Type::Literal;
            if (token==Lexer::At) {
               if (lexer.next(objectSubType)!=Lexer::Name)
                  parseError("language tag expected");
               objectType=Type::CustomLanguage;
            } else if (token==Lexer::Type) {
               string type;
               token=lexer.next(type);
               if (token==Lexer::URI) {
                  // Already parsed
               } else if (token==Lexer::Colon) {
                  parseQualifiedName("",type);
               } else if (token==Lexer::Name) {
                  parseQualifiedName(type,type);
               }
               if (type=="http://www.w3.org/2001/XMLSchema#string") {
                  objectType=Type::String;
               } else if (type=="http://www.w3.org/2001/XMLSchema#integer") {
                  objectType=Type::Integer;
               } else if (type=="http://www.w3.org/2001/XMLSchema#decimal") {
                  objectType=Type::Decimal;
               } else if (type=="http://www.w3.org/2001/XMLSchema#double") {
                  objectType=Type::Double;
               } else if (type=="http://www.w3.org/2001/XMLSchema#boolean") {
                  objectType=Type::Boolean;
               } else {
                  objectType=Type::CustomType;
                  objectSubType=type;
               }
            } else {
               lexer.ungetIgnored(token);
            }
            return;
         }
      default: parseError("invalid object");
   }
}
//---------------------------------------------------------------------------
void TurtleParser::parsePredicateObjectList(const string& subject,string& predicate,string& object,Type::ID& objectType,string& objectSubType)
   // Parse a predicate object list
{
   // Parse the first predicate
   Lexer::Token token;
   switch (token=lexer.next(predicate)) {
      case Lexer::URI: constructAbsoluteURI(predicate); break;
      case Lexer::A: predicate="http://www.w3.org/1999/02/22-rdf-syntax-ns#type"; break;
      case Lexer::Colon: lexer.unget(token,predicate); parseQualifiedName("",predicate); break;
      case Lexer::Name: if (predicate=="_") parseError("blank nodes not allowed as predicate"); parseQualifiedName(predicate,predicate); break;
      default: parseError("invalid predicate");
   }

   // Parse the object
   parseObject(object,objectType,objectSubType);

   // Additional objects?
   token=lexer.next();
   while (token==Lexer::Comma) {
      string additionalObject,additionalObjectSubType;
      Type::ID additionalObjectType;
      parseObject(additionalObject,additionalObjectType,additionalObjectSubType);
      triples.push_back(Triple(subject,predicate,additionalObject,additionalObjectType,additionalObjectSubType));
      token=lexer.next();
   }

   // Additional predicates?
   while (token==Lexer::Semicolon) {
      // Parse the predicate
      string additionalPredicate;
      switch (token=lexer.next(additionalPredicate)) {
         case Lexer::URI: constructAbsoluteURI(additionalPredicate); break;
         case Lexer::A: additionalPredicate="http://www.w3.org/1999/02/22-rdf-syntax-ns#type"; break;
         case Lexer::Colon: lexer.unget(token,additionalPredicate); parseQualifiedName("",additionalPredicate); break;
         case Lexer::Name: if (additionalPredicate=="_") parseError("blank nodes not allowed as predicate"); parseQualifiedName(additionalPredicate,additionalPredicate); break;
         default: lexer.unget(token,additionalPredicate); return;
      }

      // Parse the object
      string additionalObject,additionalObjectSubType;
      Type::ID additionalObjectType;
      parseObject(additionalObject,additionalObjectType,additionalObjectSubType);
      triples.push_back(Triple(subject,additionalPredicate,additionalObject,additionalObjectType,additionalObjectSubType));

      // Additional objects?
      token=lexer.next();
      while (token==Lexer::Comma) {
         parseObject(additionalObject,additionalObjectType,additionalObjectSubType);
         triples.push_back(Triple(subject,additionalPredicate,additionalObject,additionalObjectType,additionalObjectSubType));
         token=lexer.next();
      }
   }
   lexer.ungetIgnored(token);
}
//---------------------------------------------------------------------------
void TurtleParser::parseTriple(Lexer::Token token,std::string& subject,std::string& predicate,std::string& object,Type::ID& objectType,std::string& objectSubType)
   // Parse a triple
{
   parseSubject(token,subject);
   parsePredicateObjectList(subject,predicate,object,objectType,objectSubType);
   if (lexer.next()!=Lexer::Dot)
      parseError("'.' expected after triple");
}
//---------------------------------------------------------------------------
bool TurtleParser::parse(std::string& subject,std::string& predicate,std::string& object,Type::ID& objectType,std::string& objectSubType)
   // Read the next triple
{
   // Some triples left?
   if (triplesReader<triples.size()) {
      subject=triples[triplesReader].subject;
      predicate=triples[triplesReader].predicate;
      object=triples[triplesReader].object;
      objectType=triples[triplesReader].objectType;
      objectSubType=triples[triplesReader].objectSubType;
      if ((++triplesReader)>=triples.size()) {
         triples.clear();
         triplesReader=0;
      }
      return true;
   }

   // No, check if the input is done
   Lexer::Token token;
   while (true) {
      token=lexer.next(subject);
      if (token==Lexer::Eof) return false;

      // A directive?
      if (token==Lexer::At) {
         parseDirective();
         continue;
      } else break;
   }

   // No, parse a triple
   parseTriple(token,subject,predicate,object,objectType,objectSubType);
   return true;
}
//---------------------------------------------------------------------------
