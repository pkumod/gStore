#ifndef H_infra_util_Type
#define H_infra_util_Type
//---------------------------------------------------------------------------
// RDF-3X
// (c) 2009 Thomas Neumann. Web site: http://www.mpi-inf.mpg.de/~neumann/rdf3x
//
// This work is licensed under the Creative Commons
// Attribution-Noncommercial-Share Alike 3.0 Unported License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/
// or send a letter to Creative Commons, 171 Second Street, Suite 300,
// San Francisco, California, 94105, USA.
//---------------------------------------------------------------------------
/// Information about the type system
class Type {
  public:
  /// Different literal types
  enum Type_ID {
    Type_URI,
    Type_Literal,
    Type_CustomLanguage,
    Type_CustomType,
    Type_String,
    Type_Integer,
    Type_Decimal,
    Type_Double,
    Type_Boolean
  };
  /// Does the type have a sub-type?
  static inline bool hasSubType(Type_ID t)
  {
    return (t == Type_CustomLanguage) || (t == Type_CustomType);
  }
  /// Get the type of the sub-type
  static inline Type_ID getSubTypeType(Type_ID t)
  {
    return (t == Type_CustomLanguage) ? Type_Literal : Type_URI;
  }
};
//---------------------------------------------------------------------------
#endif
