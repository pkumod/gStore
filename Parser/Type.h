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
   enum ID {
      URI, Literal, CustomLanguage, CustomType,
      String, Integer, Decimal, Double, Boolean
   };
   /// Does the type have a sub-type?
   static inline bool hasSubType(ID t) { return (t==CustomLanguage)||(t==CustomType); }
   /// Get the type of the sub-type
   static inline ID getSubTypeType(ID t) { return (t==CustomLanguage)?Literal:URI; }
};
//---------------------------------------------------------------------------
#endif
