#include "TurtleParser.h"
#include <sstream>
#include "../Util/Triple.h"
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
static bool issep(char c) { return (c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='[')||(c==']')||(c=='(')||(c==')')||(c==',')||(c==';')||(c==':')||(c=='.')||(c=='>'); }
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
         case '<':
         {
            char next_c;
            if (read(next_c) && next_c == '<')
               return TripleTermOpen;
            // Not <<, so next_c is the first content char of a URI
            unread();  // put back next_c
            return lexURI(token, c);
         }
         case '>':
         {
            char next_c;
            if (read(next_c) && next_c == '>')
               return TripleTermClose;
            // single > is an error
            unread();
            stringstream msg;
            msg << "lexer error in line " << line << ": unexpected '>'";
            throw Exception(msg.str());
         }

         case '~': return ReifierMark;
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
               if ((unsigned char)c > 0x80)
               {// chinese
                  token=c;
                  while (read(c)) {
                     if (issep(c)) { unread(); break; }
                     token+=c;
                  }
                  return Name;
               }
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

   if (value=="base" || value=="BASE") {
      if (lexer.next(base)!=Lexer::URI)
         parseError("URI expected after @base or BASE");
   } else if (value=="prefix" || value=="PREFIX") {
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
      case Lexer::TripleTermOpen:
         // Triple term as subject
         {
            // N-Triples strict mode: triple terms in subject position are not allowed
            if (strictNTriples)
               parseError("triple term not allowed as subject in N-Triples; use RDF reification instead");

            bool needsReif = false;
            std::string explicitReifier;
            parseTripleTermContent(subject, needsReif, explicitReifier);
            if (needsReif) {
               std::string reifier;
               if (!explicitReifier.empty()) {
                  reifier = explicitReifier;
               } else {
                  std::stringstream reif_id;
                  reif_id << "urn:gstore:reifier:" << (nextBlank++);
                  reifier = reif_id.str();
               }
               triples.push_back(Triple(reifier,
                  "http://www.w3.org/1999/02/22-rdf-syntax-ns#reifies",
                  subject, Type::URI, ""));
               // Emit component triples for variable triple term matching in SPARQL
               emitTripleTermComponents(subject);
               subject = reifier;
            }
            return;
         }
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
      case Lexer::TripleTermOpen:
         // Triple term as object
         {
            // N-Triples strict mode: only <<(...)>> (with parentheses) is allowed
            if (strictNTriples) {
               Lexer::Token peek = lexer.next();
               if (peek != Lexer::LParen)
                  parseError("Turtle-style <<...>> not allowed in N-Triples; use <<(...)>> with parentheses");
               lexer.ungetIgnored(peek);
            }
            bool needsReif = false;
            std::string explicitReifier;
            parseTripleTermContent(object, needsReif, explicitReifier);
            objectType=Type::URI;
            if (needsReif) {
               std::string reifier;
               if (!explicitReifier.empty()) {
                  reifier = explicitReifier;
               } else {
                  std::stringstream reif_id;
                  reif_id << "urn:gstore:reifier:" << (nextBlank++);
                  reifier = reif_id.str();
               }
               triples.push_back(Triple(reifier,
                  "http://www.w3.org/1999/02/22-rdf-syntax-ns#reifies",
                  object, Type::URI, ""));
               // Emit component triples for variable triple term matching in SPARQL
               emitTripleTermComponents(object);
               object = reifier;
            }
            return;
         }
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
//---------------------------------------------------------------------------
void TurtleParser::parseTripleTermContent(std::string& termStr, bool& needsReification, std::string& outReifier)
   // Parse a triple term content between << and >>
   // Handles both N-Triples style <<( ... )>> and Turtle style << ... >>
{
   // Check for optional LParen (N-Triples format)
   Lexer::Token token = lexer.next();
   bool hasParen = false;
   if (token == Lexer::LParen)
   {
      hasParen = true;
   }
   else
   {
      lexer.ungetIgnored(token);
   }

   // Parse the three components: subject, predicate, object
   std::string s, p, o;
   Type::ID oType;
   std::string oSubType;

   // Parse subject
   token = lexer.next(s);
   parseSubject(token, s);

   // Parse predicate - must be URI, 'a', or qualified name
   std::string pred;
   token = lexer.next(pred);
   switch (token) {
      case Lexer::URI:
         constructAbsoluteURI(pred);
         break;
      case Lexer::A:
         pred = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
         break;
      case Lexer::Colon:
         lexer.unget(token, pred);
         parseQualifiedName("", pred);
         break;
      case Lexer::Name:
         if (pred == "_")
            parseError("blank nodes not allowed as predicate in triple term");
         parseQualifiedName(pred, pred);
         break;
      default:
         parseError("invalid predicate in triple term");
   }
   p = pred;

   // Parse object
   parseObject(o, oType, oSubType);

   // Check for optional ~ reifier (Turtle explicit reifier syntax)
   // e.g., << :s :p :o ~ :iri >>
   std::string explicitReifier;
   bool hasExplicitReifier = false;
   token = lexer.next();
   if (token == Lexer::ReifierMark)
   {
      hasExplicitReifier = true;
      Lexer::Token reifToken = lexer.next(explicitReifier);
      if (reifToken == Lexer::URI)
      {
         constructAbsoluteURI(explicitReifier);
      }
      else if (reifToken == Lexer::Colon)
      {
         lexer.unget(reifToken, explicitReifier);
         parseQualifiedName("", explicitReifier);
      }
      else if (reifToken == Lexer::Name)
      {
         if (explicitReifier == "_")
         {
            // Blank node reifier: _:label
            if (lexer.next() != Lexer::Colon || !isName(lexer.next(explicitReifier)))
               parseError("blank node reifier must be _:label");
            explicitReifier = "_:" + explicitReifier;
         }
         else
            parseQualifiedName(explicitReifier, explicitReifier);
      }
      else if (reifToken == Lexer::A)
      {
         explicitReifier = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
      }
      else
      {
         // Anonymous reifier (just ~ without a reifier name)
         lexer.unget(reifToken, explicitReifier);
         hasExplicitReifier = false;
      }
   }
   else
   {
      lexer.ungetIgnored(token);
   }

      // Expect closing
   if (hasParen)
   {
      token = lexer.next();
      if (token != Lexer::RParen)
         parseError("')' expected in triple term");
   }

   token = lexer.next();
   if (token != Lexer::TripleTermClose)
      parseError("'>>' expected to close triple term");

   // Build canonical string: <<( <subject> <predicate> <object> )>>
   // Subject: wrap in <> for URIs, keep triple terms and blank nodes as-is
   if (s.length() > 0 && s[0] != '<' && s[0] != '_')
      s = "<" + s + ">";
   // Predicate: always wrap in <>
   if (p.length() > 0 && p[0] != '<')
      p = "<" + p + ">";
   // Object: wrap based on type
   if (oType == Type::String)
      o = "\"" + o + "\"^^<http://www.w3.org/2001/XMLSchema#string>";
   else if (oType == Type::Integer)
      o = "\"" + o + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
   else if (oType == Type::Decimal)
      o = "\"" + o + "\"^^<http://www.w3.org/2001/XMLSchema#decimal>";
   else if (oType == Type::Double)
      o = "\"" + o + "\"^^<http://www.w3.org/2001/XMLSchema#double>";
   else if (oType == Type::Boolean)
      o = "\"" + o + "\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
   else if (oType == Type::CustomLanguage)
      o = "\"" + o + "\"@" + oSubType;
   else if (oType == Type::CustomType)
      o = "\"" + o + "\"^^" + oSubType;
   else if (oType == Type::Literal)
      o = "\"" + o + "\"";
   else if (o.length() > 0 && o[0] != '<' && o[0] != '_')
   {
      // URI entity - wrap in <>
      o = "<" + o + ">";
   }
   // Triple terms and blank nodes are kept as-is

   termStr = "<<( " + s + " " + p + " " + o + " )>>";

   // Turtle-style <<...>> (no parens) needs reification
   // N-Triples style <<(...)>> (with parens) does not
   needsReification = !hasParen;
   outReifier = hasExplicitReifier ? explicitReifier : "";
}

/**
   Emit helper triples that link a triple term's canonical string to its
   individual subject, predicate, and object components.  This enables
   SPARQL queries to match variables inside triple terms by joining on
   the components rather than the monolithic string.

   @param canonicalTT the canonical triple term string <<( s p o )>>
*/
void TurtleParser::emitTripleTermComponents(const std::string& canonicalTT)
{
   std::string s, p, o;
   if (!isTripleTermString(canonicalTT)) return;
   if (!parseTripleTermString(canonicalTT, s, p, o)) return;

   // Helper to determine object type from a component string
   // (canonical form: URIs are <...>, literals start with ", blank nodes start with _:)
   auto determine_component_type = [](const std::string& val) -> Type::ID {
      if (val.empty()) return Type::URI;
      if (val[0] == '"') {
         // Check for ^^type or @lang suffix
         if (val.find("^^") != std::string::npos) return Type::CustomType;
         if (val.find('@') != std::string::npos) return Type::CustomLanguage;
         return Type::Literal;
      }
      if (val.length() >= 2 && val[0] == '_' && val[1] == ':')
         return Type::URI; // blank node stored as URI in gStore
      if (val.length() >= 4 && val[0] == '<' && val[1] == '<')
         return Type::URI; // nested triple term stored as URI
      return Type::URI; // default: IRI or qualified name
   };

   // Strip angle brackets from URI components (RDFParser will add them)
   // to avoid double-wrapping: the canonical string stores URIs as <...>
   // Blank nodes (starting with _:), literals (starting with "), and
   // nested triple terms (starting with <<() should NOT be stripped.
   auto strip_angle_brackets = [](std::string& val) {
      // Only strip if it looks like a plain URI: starts with <, ends with >
      // and is not a triple term (<<(...)>>)
      if (val.length() >= 2 && val[0] == '<' && val.back() == '>' &&
          !(val.length() >= 3 && val[1] == '<'))
         val = val.substr(1, val.length() - 2);
   };
   // Strip literal quotes from object components (RDFParser will re-add them
   // based on the Type::ID). The canonical string stores literals as "value"^^<type>
   // or "value"@lang, but RDFParser expects the raw value.
   auto strip_literal_quotes = [](std::string& val, std::string& sub_type, Type::ID& o_type) {
      if (val.empty() || val[0] != '"') return;
      // Find the closing quote (handle \" escapes)
      size_t endQuote = 1;
      while (endQuote < val.length()) {
         if (val[endQuote] == '\\') { endQuote += 2; continue; }
         if (val[endQuote] == '"') break;
         endQuote++;
      }
      if (endQuote >= val.length()) return;
      std::string inner = val.substr(1, endQuote - 1);
      std::string suffix = val.substr(endQuote + 1);
      if (!suffix.empty() && suffix[0] == '@') {
         o_type = Type::CustomLanguage;
         sub_type = suffix.substr(1);
         val = inner;
      } else if (suffix.length() >= 4 && suffix.substr(0, 4) == "^^<") {
         o_type = Type::CustomType;
         sub_type = suffix.substr(3, suffix.length() - 4);
         val = inner;
      } else {
         o_type = Type::Literal;
         val = inner;
      }
   };

   strip_angle_brackets(s);
   strip_angle_brackets(p);

   // Determine object type and strip wrapping from object component
   Type::ID o_type = determine_component_type(o);
   std::string o_sub_type = "";
   if (o_type != Type::URI)
      strip_literal_quotes(o, o_sub_type, o_type);
   else
      strip_angle_brackets(o);

   static const std::string subj_pred = "urn:gstore:tripleTermSubject";
   static const std::string pred_pred = "urn:gstore:tripleTermPredicate";
   static const std::string obj_pred  = "urn:gstore:tripleTermObject";

   triples.push_back(Triple(canonicalTT, subj_pred, s, Type::URI, ""));
   triples.push_back(Triple(canonicalTT, pred_pred, p, Type::URI, ""));
   triples.push_back(Triple(canonicalTT, obj_pred,  o, o_type, o_sub_type));
}

void TurtleParser::parseTriple(Lexer::Token token,std::string& subject,std::string& predicate,std::string& object,Type::ID& objectType,std::string& objectSubType)
   // Parse a triple
{
   parseSubject(token,subject);
   // Reification for triple term subjects is handled inside parseSubject
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

      // A directive? Handle both @prefix/@base and PREFIX/BASE (without @)
      if (token==Lexer::At) {
         parseDirective();
         continue;
      } else if (token==Lexer::Name && (subject=="PREFIX" || subject=="prefix" || subject=="BASE" || subject=="base")) {
         // Handle PREFIX/BASE directive without @ (no trailing dot required)
         if (subject=="BASE" || subject=="base") {
            if (lexer.next(base)!=Lexer::URI)
               parseError("URI expected after BASE");
         } else {
            std::string prefixName;
            Lexer::Token t=lexer.next(prefixName);
            if (t==Lexer::Name) {
               t=lexer.next();
            } else prefixName.resize(0);
            if (t!=Lexer::Colon)
               parseError("':' expected after PREFIX");
            std::string uri;
            if (lexer.next(uri)!=Lexer::URI)
               parseError("URI expected after PREFIX");
            prefixes[prefixName]=uri;
         }
         continue;
      } else break;
   }

   // No, parse a triple
   parseTriple(token,subject,predicate,object,objectType,objectSubType);
   return true;
}
//---------------------------------------------------------------------------
