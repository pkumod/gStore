/*=============================================================================
# Filename: TempResult.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-08-14
# Description: 
=============================================================================*/

#ifndef _QUERY_TEMPRESULT_H
#define _QUERY_TEMPRESULT_H

#include "../Util/Util.h"
#include "../StringIndex/StringIndex.h"
#include "QueryTree.h"
#include "RegexExpression.h"
#include "Varset.h"

class FilterEvaluationMultitypeValue {
  public:
  class EffectiveBooleanValue {
public:
    enum EBV {
      true_value,
      false_value,
      error_value
    };
    EBV value;

    EffectiveBooleanValue()
        : value(error_value)
    {
    }
    EffectiveBooleanValue(bool _value)
    {
      if (_value)
        value = true_value;
      else
        value = false_value;
    }
    EffectiveBooleanValue(EBV _value)
        : value(_value)
    {
    }

    EffectiveBooleanValue operator!();
    EffectiveBooleanValue operator||(const EffectiveBooleanValue& x);
    EffectiveBooleanValue operator&&(const EffectiveBooleanValue& x);
    EffectiveBooleanValue operator==(const EffectiveBooleanValue& x);
    EffectiveBooleanValue operator!=(const EffectiveBooleanValue& x);
    EffectiveBooleanValue operator<(const EffectiveBooleanValue& x);
    EffectiveBooleanValue operator<=(const EffectiveBooleanValue& x);
    EffectiveBooleanValue operator>(const EffectiveBooleanValue& x);
    EffectiveBooleanValue operator>=(const EffectiveBooleanValue& x);
  };

  class DateTime {
private:
    std::vector<int> date;
    //year = date[0]
    //month = date[1]
    //day = date[2]
    //hour = date[3]
    //minute = date[4]
    //second = date[5]

public:
    DateTime(int _year = 0, int _month = 0, int _day = 0, int _hour = 0, int _minute = 0, int _second = 0)
    {
      this->date.reserve(6);
      this->date.push_back(_year);
      this->date.push_back(_month);
      this->date.push_back(_day);
      this->date.push_back(_hour);
      this->date.push_back(_minute);
      this->date.push_back(_second);
    }

    EffectiveBooleanValue operator==(const DateTime& x);
    EffectiveBooleanValue operator!=(const DateTime& x);
    EffectiveBooleanValue operator<(const DateTime& x);
    EffectiveBooleanValue operator<=(const DateTime& x);
    EffectiveBooleanValue operator>(const DateTime& x);
    EffectiveBooleanValue operator>=(const DateTime& x);
  };

  enum DataType {
    rdf_term,
    iri,
    literal,
    xsd_string,
    xsd_boolean,
    xsd_integer,
    xsd_decimal,
    xsd_float,
    xsd_double,
    xsd_datetime
  };

  DataType datatype;
  std::string term_value, str_value;
  EffectiveBooleanValue bool_value;
  int int_value;
  float flt_value;
  double dbl_value;
  DateTime dt_value;

  bool isSimpleLiteral();
  void getSameNumericType(FilterEvaluationMultitypeValue& x);
  FilterEvaluationMultitypeValue operator!();
  FilterEvaluationMultitypeValue operator||(FilterEvaluationMultitypeValue& x);
  FilterEvaluationMultitypeValue operator&&(FilterEvaluationMultitypeValue& x);
  FilterEvaluationMultitypeValue operator==(FilterEvaluationMultitypeValue& x);
  FilterEvaluationMultitypeValue operator!=(FilterEvaluationMultitypeValue& x);
  FilterEvaluationMultitypeValue operator<(FilterEvaluationMultitypeValue& x);
  FilterEvaluationMultitypeValue operator<=(FilterEvaluationMultitypeValue& x);
  FilterEvaluationMultitypeValue operator>(FilterEvaluationMultitypeValue& x);
  FilterEvaluationMultitypeValue operator>=(FilterEvaluationMultitypeValue& x);

  FilterEvaluationMultitypeValue()
      : datatype(rdf_term)
      , int_value(0)
      , flt_value(0)
      , dbl_value(0)
  {
  }
};

class TempResult {
  public:
  class ResultPair {
public:
    unsigned* id;
    std::vector<std::string> str;
    ResultPair()
        : id(NULL)
    {
    }
  };

  Varset id_varset, str_varset;
  std::vector<ResultPair> result;

  Varset getAllVarset();

  void release();

  static int compareRow(const ResultPair& x, const int x_id_cols, const std::vector<int>& x_pos,
                        const ResultPair& y, const int y_id_cols, const std::vector<int>& y_pos);
  void sort(int l, int r, const std::vector<int>& this_pos);
  int findLeftBounder(const std::vector<int>& this_pos, const ResultPair& x, const int x_id_cols, const std::vector<int>& x_pos) const;
  int findRightBounder(const std::vector<int>& this_pos, const ResultPair& x, const int x_id_cols, const std::vector<int>& x_pos) const;

  void convertId2Str(Varset convert_varset, StringIndex* stringindex, Varset& entity_literal_varset);
  void doJoin(TempResult& x, TempResult& r);
  void doUnion(TempResult& r);
  void doOptional(std::vector<bool>& binding, TempResult& x, TempResult& rn, TempResult& ra, bool add_no_binding);
  void doMinus(TempResult& x, TempResult& r);

  void getFilterString(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild& child, FilterEvaluationMultitypeValue& femv, ResultPair& row, int id_cols, StringIndex* stringindex);
  FilterEvaluationMultitypeValue matchFilterTree(QueryTree::GroupPattern::FilterTree::FilterTreeNode& filter, ResultPair& row, int id_cols, StringIndex* stringindex);
  void doFilter(QueryTree::GroupPattern::FilterTree::FilterTreeNode& filter, TempResult& r, StringIndex* stringindex, Varset& entity_literal_varset);

  void print(int no = -1);
};

class TempResultSet {
  public:
  std::vector<TempResult> results;

  void release();

  int findCompatibleResult(Varset& _id_varset, Varset& _str_varset);

  void doJoin(TempResultSet& x, TempResultSet& r, StringIndex* stringindex, Varset& entity_literal_varset);
  void doUnion(TempResultSet& x, TempResultSet& r);
  void doOptional(TempResultSet& x, TempResultSet& r, StringIndex* stringindex, Varset& entity_literal_varset);
  void doMinus(TempResultSet& x, TempResultSet& r, StringIndex* stringindex, Varset& entity_literal_varset);
  void doFilter(QueryTree::GroupPattern::FilterTree::FilterTreeNode& filter, TempResultSet& r, StringIndex* stringindex, Varset& entity_literal_varset);

  void doProjection1(Varset& proj, TempResultSet& r, StringIndex* stringindex, Varset& entity_literal_varset);
  void doDistinct1(TempResultSet& r);

  void print();
};

#endif // _QUERY_TEMPRESULT_H
