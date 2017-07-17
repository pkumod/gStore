/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-05-05
# Description: implement functions in GeneralEvaluation.h
=============================================================================*/

#include "GeneralEvaluation.h"

using namespace std;

bool GeneralEvaluation::parseQuery(const string &_query)
{
	try
	{
		this->query_parser.SPARQLParse(_query, this->query_tree);
	}
	catch(const char *e)
	{
		printf("%s\n", e);
		return false;
	}

	return true;
}

QueryTree& GeneralEvaluation::getQueryTree()
{
	return this->query_tree;
}

bool GeneralEvaluation::doQuery()
{
	if (!this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjection().empty())
		return false;

	this->query_tree.getGroupPattern().getVarset();
	this->query_tree.getGroupByVarset() = this->query_tree.getGroupByVarset() * this->query_tree.getGroupPattern().grouppattern_resultset_maximal_varset;

	if (this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjection().empty() && !this->query_tree.getGroupByVarset().empty())
	{
		printf("[ERROR]	Select * and Group By can't appear at the same time.\n");
		return false;
	}

	if (!this->query_tree.checkSelectAggregateFunctionGroupByValid())
	{
		printf("[ERROR]	The vars/aggregate functions in the Select Clause are invalid.\n");
		return false;
	}

	if (this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset.hasCommonVar(this->query_tree.getGroupPattern().grouppattern_predicate_maximal_varset))
	{
		printf("[ERROR]	There are some vars occur both in subject/object and predicate.\n");
		return false;
	}

	this->strategy = Strategy(this->kvstore, this->vstree, this->pre2num, this->limitID_predicate, this->limitID_literal,this->limitID_entity);
	if (this->query_tree.checkWellDesigned())
	{
		printf("=================\n");
		printf("||well-designed||\n");
		printf("=================\n");

		this->rewriting_evaluation_stack.clear();
		this->rewriting_evaluation_stack.push_back(this->query_tree.getGroupPattern());
		this->temp_result = this->rewritingBasedQueryEvaluation(0);
	}
	else
	{
		printf("=====================\n");
		printf("||not well-designed||\n");
		printf("=====================\n");

		this->temp_result = this->semanticBasedQueryEvaluation(this->query_tree.getGroupPattern());
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator ! ()
{
	if (this->value == true_value)	return false_value;
	if (this->value == false_value)	return true_value;
	if (this->value == error_value)	return error_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator || (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return false_value;
	if (this->value == true_value && x.value == error_value)	return true_value;
	if (this->value == error_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == error_value)	return error_value;
	if (this->value == error_value && x.value == false_value)	return error_value;
	if (this->value == error_value && x.value == error_value)	return error_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator && (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return false_value;
	if (this->value == true_value && x.value == error_value)	return error_value;
	if (this->value == error_value && x.value == true_value)	return error_value;
	if (this->value == false_value && x.value == error_value)	return false_value;
	if (this->value == error_value && x.value == false_value)	return false_value;
	if (this->value == error_value && x.value == error_value)	return error_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator == (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator != (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator < (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator <= (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator > (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator >= (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::DateTime::operator == (const DateTime &x)
{
	if (this->date == x.date)	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::DateTime::operator != (const DateTime &x)
{
	if (this->date != x.date)	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::DateTime::operator < (const DateTime &x)
{
	if (this->date < x.date)	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::DateTime::operator <= (const DateTime &x)
{
	if (this->date <= x.date)	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::DateTime::operator > (const DateTime &x)
{
	if (this->date > x.date)	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::DateTime::operator >= (const DateTime &x)
{
	if (this->date >= x.date)	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}


bool GeneralEvaluation::FilterEvaluationMultitypeValue::isSimpleLiteral()
{
	if (this->datatype == literal)
	{
		int length = this->str_value.length();
		if (length >= 2 && this->str_value[0] == '"' && this->str_value[length - 1] == '"')
			return true;
	}
	return false;
}

void GeneralEvaluation::FilterEvaluationMultitypeValue::getSameNumericType (FilterEvaluationMultitypeValue &x)
{
	DataType to_type = max(this->datatype, x.datatype);

	if (this->datatype == xsd_integer && to_type == xsd_decimal)
		this->flt_value = this->int_value;
	if (this->datatype == xsd_integer && to_type == xsd_float)
		this->flt_value = this->int_value;
	if (this->datatype == xsd_integer && to_type == xsd_double)
		this->dbl_value = this->int_value;
	if (this->datatype == xsd_decimal && to_type == xsd_double)
		this->dbl_value = this->flt_value;
	if (this->datatype == xsd_float && to_type == xsd_double)
		this->dbl_value = this->flt_value;
	this->datatype = to_type;

	if (x.datatype == xsd_integer && to_type == xsd_decimal)
		x.flt_value = x.int_value;
	if (x.datatype == xsd_integer && to_type == xsd_float)
		x.flt_value = x.int_value;
	if (x.datatype == xsd_integer && to_type == xsd_double)
		x.dbl_value = x.int_value;
	if (x.datatype == xsd_decimal && to_type == xsd_double)
		x.dbl_value = x.flt_value;
	if (x.datatype == xsd_float && to_type == xsd_double)
		x.dbl_value = x.flt_value;
	x.datatype = to_type;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator !()
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = !this->bool_value;
	return ret_femv;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator || (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean && x.datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = (this->bool_value || x.bool_value);
	return ret_femv;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator && (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean && x.datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = (this->bool_value && x.bool_value);
	return ret_femv;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator == (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value == x.bool_value);
		return ret_femv;
	}

	if ((this->datatype == xsd_boolean) ^ (x.datatype == xsd_boolean))
	{
		if (this->datatype == xsd_boolean && this->bool_value.value == EffectiveBooleanValue::error_value)
			return ret_femv;
		if (x.datatype == xsd_boolean && x.bool_value.value == EffectiveBooleanValue::error_value)
			return ret_femv;

		ret_femv.bool_value = EffectiveBooleanValue::false_value;
		return ret_femv;
	}

	if (xsd_integer <= this->datatype && this->datatype <= xsd_double && xsd_integer <= x.datatype && x.datatype <= xsd_double)
	{
		this->getSameNumericType(x);

		if (this->datatype == xsd_integer && x.datatype == xsd_integer)
			ret_femv.bool_value = (this->int_value == x.int_value);
		if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value == x.flt_value);
		if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value == x.flt_value);
		if (this->datatype == xsd_double && x.datatype == xsd_double)
			ret_femv.bool_value = (this->dbl_value == x.dbl_value);

		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value == x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value == x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value == x.dt_value);
		return ret_femv;
	}

	ret_femv.bool_value = (this->term_value == x.term_value);
	return ret_femv;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator != (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value != x.bool_value);
		return ret_femv;
	}

	if ((this->datatype == xsd_boolean) ^ (x.datatype == xsd_boolean))
	{
		if (this->datatype == xsd_boolean && this->bool_value.value == EffectiveBooleanValue::error_value)
			return ret_femv;
		if (x.datatype == xsd_boolean && x.bool_value.value == EffectiveBooleanValue::error_value)
			return ret_femv;

		ret_femv.bool_value = EffectiveBooleanValue::true_value;
		return ret_femv;
	}

	if (xsd_integer <= this->datatype && this->datatype <= xsd_double && xsd_integer <= x.datatype && x.datatype <= xsd_double)
	{
		this->getSameNumericType(x);

		if (this->datatype == xsd_integer && x.datatype == xsd_integer)
			ret_femv.bool_value = (this->int_value != x.int_value);
		if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value != x.flt_value);
		if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value != x.flt_value);
		if (this->datatype == xsd_double && x.datatype == xsd_double)
			ret_femv.bool_value = (this->dbl_value != x.dbl_value);

		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value != x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value != x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value != x.dt_value);
		return ret_femv;
	}

	ret_femv.bool_value = (this->term_value != x.term_value);
	return ret_femv;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator < (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value < x.bool_value);
		return ret_femv;
	}

	if (xsd_integer <= this->datatype && this->datatype <= xsd_double && xsd_integer <= x.datatype && x.datatype <= xsd_double)
	{
		this->getSameNumericType(x);

		if (this->datatype == xsd_integer && x.datatype == xsd_integer)
			ret_femv.bool_value = (this->int_value < x.int_value);
		if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value < x.flt_value);
		if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value < x.flt_value);
		if (this->datatype == xsd_double && x.datatype == xsd_double)
			ret_femv.bool_value = (this->dbl_value < x.dbl_value);

		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value < x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value < x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value < x.dt_value);
		return ret_femv;
	}

	return ret_femv;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator <= (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value <= x.bool_value);
		return ret_femv;
	}

	if (xsd_integer <= this->datatype && this->datatype <= xsd_double && xsd_integer <= x.datatype && x.datatype <= xsd_double)
	{
		this->getSameNumericType(x);

		if (this->datatype == xsd_integer && x.datatype == xsd_integer)
			ret_femv.bool_value = (this->int_value <= x.int_value);
		if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value <= x.flt_value);
		if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value <= x.flt_value);
		if (this->datatype == xsd_double && x.datatype == xsd_double)
			ret_femv.bool_value = (this->dbl_value <= x.dbl_value);

		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value <= x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value <= x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value <= x.dt_value);
		return ret_femv;
	}

	return ret_femv;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator > (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value > x.bool_value);
		return ret_femv;
	}

	if (xsd_integer <= this->datatype && this->datatype <= xsd_double && xsd_integer <= x.datatype && x.datatype <= xsd_double)
	{
		this->getSameNumericType(x);

		if (this->datatype == xsd_integer && x.datatype == xsd_integer)
			ret_femv.bool_value = (this->int_value > x.int_value);
		if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value > x.flt_value);
		if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value > x.flt_value);
		if (this->datatype == xsd_double && x.datatype == xsd_double)
			ret_femv.bool_value = (this->dbl_value > x.dbl_value);

		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value > x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value > x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value > x.dt_value);
		return ret_femv;
	}

	return ret_femv;
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::FilterEvaluationMultitypeValue::operator >= (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value >= x.bool_value);
		return ret_femv;
	}

	if (xsd_integer <= this->datatype && this->datatype <= xsd_double && xsd_integer <= x.datatype && x.datatype <= xsd_double)
	{
		this->getSameNumericType(x);

		if (this->datatype == xsd_integer && x.datatype == xsd_integer)
			ret_femv.bool_value = (this->int_value >= x.int_value);
		if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value >= x.flt_value);
		if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value >= x.flt_value);
		if (this->datatype == xsd_double && x.datatype == xsd_double)
			ret_femv.bool_value = (this->dbl_value >= x.dbl_value);

		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value >= x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value >= x.str_value);
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value >= x.dt_value);
		return ret_femv;
	}

	return ret_femv;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

Varset GeneralEvaluation::TempResult::getAllVarset()
{
	return this->id_varset + this->str_varset;
}

void GeneralEvaluation::TempResult::release()
{
	for (int i = 0; i < (int)this->result.size(); i++)
	{
		delete[] result[i].id;
		vector<string>().swap(result[i].str);
	}
}

int GeneralEvaluation::TempResult::compareRow(const ResultPair &x, const int x_id_cols, const vector<int> &x_pos,
		  	  	  	  	  	  	  	  	  	  const ResultPair &y, const int y_id_cols, const vector<int> &y_pos)
{
	for (int i = 0; i < min((int)x_pos.size(), (int)y_pos.size()); i++)
	{
		int px = x_pos[i], py = y_pos[i];

		if (px < x_id_cols && py < y_id_cols)
		{
			if (x.id[px] < y.id[py])
				return -1;
			if (x.id[px] > y.id[py])
				return 1;
		}

		if (px >= x_id_cols && py >= y_id_cols)
		{
			if (x.str[px - x_id_cols] < y.str[py - y_id_cols])
				return -1;
			if (x.str[px - x_id_cols] > y.str[py - y_id_cols])
				return 1;
		}
	}

	return 0;
}

void GeneralEvaluation::TempResult::sort(int l, int r, const vector<int> &this_pos)
{
	int i = l, j = r;
	ResultPair m = this->result[(l + r) / 2];

	int this_id_cols = this->id_varset.getVarsetSize();
	do
	{
		while (compareRow(this->result[i], this_id_cols, this_pos, m, this_id_cols, this_pos) == -1)	i++;
		while (compareRow(m, this_id_cols, this_pos, this->result[j], this_id_cols, this_pos) == -1)	j--;
		if (i <= j)
		{
			swap(this->result[i].id, this->result[j].id);
			swap(this->result[i].str, this->result[j].str);
			i++;
			j--;
		}
	}
	while (i <= j);

	if (l < j)	sort(l, j, this_pos);
	if (i < r)	sort(i, r, this_pos);
}

int GeneralEvaluation::TempResult::findLeftBounder(const vector<int> &this_pos, const ResultPair &x, const int x_id_cols, const vector<int> &x_pos) const
{
	int l = 0, r = (int)this->result.size() - 1;
	if (r == -1)
		return -1;

	int this_id_cols = this->id_varset.getVarsetSize();
	while (l < r)
	{
		int m = (l + r) / 2;
		if (compareRow(this->result[m], this_id_cols, this_pos, x, x_id_cols, x_pos) >= 0)		r = m;
		else l = m + 1;
	}

	if (compareRow(this->result[l], this_id_cols, this_pos, x, x_id_cols, x_pos) == 0)
		return l;
	else
		return -1;
}

int GeneralEvaluation::TempResult::findRightBounder(const vector<int> &this_pos, const ResultPair &x, const int x_id_cols, const vector<int> &x_pos) const
{
	int l = 0, r = (int)this->result.size() - 1;

	if (r == -1)
		return -1;

	int this_id_cols = this->id_varset.getVarsetSize();
	while (l < r)
	{
		int m = (l + r) / 2 + 1;
		if (compareRow(this->result[m], this_id_cols, this_pos, x, x_id_cols, x_pos) <= 0)		l = m;
		else r = m - 1;
	}

	if (compareRow(this->result[l], this_id_cols, this_pos, x, x_id_cols, x_pos) == 0)
		return l;
	else
		return -1;
}

void GeneralEvaluation::TempResult::convertId2Str(Varset convert_varset, StringIndex *stringindex, Varset &entity_literal_varset)
{
	int this_id_cols = this->id_varset.getVarsetSize();

	Varset new_id_varset = this->id_varset - convert_varset;
	Varset new_str_varset = this->str_varset + convert_varset;
	int new_id_cols = new_id_varset.getVarsetSize();

	vector<int> this2new_id_pos = this->id_varset.mapTo(new_id_varset);

	for (int i = 0; i < (int)this->result.size(); i++)
	{
		unsigned *v = new unsigned [new_id_cols];

		for (int k = 0; k < this_id_cols; k++)
			if (this2new_id_pos[k] != -1)
			{
				v[this2new_id_pos[k]] = this->result[i].id[k];
			}
			else
			{
				string str;

				if (entity_literal_varset.findVar(this->id_varset.vars[k]))
					stringindex->randomAccess(this->result[i].id[k], &str, true);
				else
					stringindex->randomAccess(this->result[i].id[k], &str, false);

				this->result[i].str.push_back(str);
			}

		delete[] this->result[i].id;
		this->result[i].id = v;
	}

	this->id_varset = new_id_varset;
	this->str_varset = new_str_varset;
}

void GeneralEvaluation::TempResult::doJoin(TempResult &x, TempResult &r)
{
	int this_id_cols = this->id_varset.getVarsetSize();
	int x_id_cols = x.id_varset.getVarsetSize();

	int r_id_cols = r.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this->id_varset.mapTo(r.id_varset);
	vector<int> x2r_id_pos = x.id_varset.mapTo(r.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();
	int x_str_cols = x.str_varset.getVarsetSize();

	int r_str_cols = r.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this->str_varset.mapTo(r.str_varset);
	vector<int> x2r_str_pos = x.str_varset.mapTo(r.str_varset);

	Varset common = this->getAllVarset() * x.getAllVarset();

	if (common.empty())
	{
		for (int i = 0; i < (int)this->result.size(); i++)
			for (int j = 0; j < (int)x.result.size(); j++)
			{
				r.result.push_back(ResultPair());

				if (r_id_cols > 0)
				{
					r.result.back().id = new unsigned [r_id_cols];
					unsigned *v = r.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2r_id_pos[k]] = this->result[i].id[k];
					for (int k = 0; k < x_id_cols; k++)
						v[x2r_id_pos[k]] = x.result[j].id[k];
				}

				if (r_str_cols > 0)
				{
					r.result.back().str.resize(r_str_cols);
					vector<string> &v = r.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2r_str_pos[k]] = this->result[i].str[k];
					for (int k = 0; k < x_str_cols; k++)
						v[x2r_str_pos[k]] = x.result[j].str[k];
				}
			}
	}
	else if (!x.result.empty())
	{
		vector<int> common2this = common.mapTo(this->getAllVarset());
		vector<int> common2x = common.mapTo(x.getAllVarset());
		x.sort(0, (int)x.result.size() - 1, common2x);

		int this_id_cols = this->id_varset.getVarsetSize();
		for (int i = 0; i < (int)this->result.size(); i++)
		{
			int left = x.findLeftBounder(common2x, this->result[i], this_id_cols, common2this);
			if (left == -1)	continue;
			int right = x.findRightBounder(common2x, this->result[i], this_id_cols, common2this);

			for (int j = left; j <= right; j++)
			{
				r.result.push_back(ResultPair());

				if (r_id_cols > 0)
				{
					r.result.back().id = new unsigned [r_id_cols];
					unsigned *v = r.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2r_id_pos[k]] = this->result[i].id[k];
					for (int k = 0; k < x_id_cols; k++)
						v[x2r_id_pos[k]] = x.result[j].id[k];
				}

				if (r_str_cols > 0)
				{
					r.result.back().str.resize(r_str_cols);
					vector<string> &v = r.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2r_str_pos[k]] = this->result[i].str[k];
					for (int k = 0; k < x_str_cols; k++)
						v[x2r_str_pos[k]] = x.result[j].str[k];
				}
			}
		}
	}
}

void GeneralEvaluation::TempResult::doUnion(TempResult &r)
{
	int this_id_cols = this->id_varset.getVarsetSize();

	int r_id_cols = r.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this->id_varset.mapTo(r.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();

	int r_str_cols = r.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this->str_varset.mapTo(r.str_varset);

	for (int i = 0; i < (int)this->result.size(); i++)
	{
		r.result.push_back(ResultPair());

		if (r_id_cols > 0)
		{
			r.result.back().id = new unsigned [r_id_cols];
			unsigned *v = r.result.back().id;

			for (int k = 0; k < this_id_cols; k++)
				v[this2r_id_pos[k]] = this->result[i].id[k];
		}

		if (r_str_cols > 0)
		{
			r.result.back().str.resize(r_str_cols);
			vector<string> &v = r.result.back().str;

			for (int k = 0; k < this_str_cols; k++)
				v[this2r_str_pos[k]] = this->result[i].str[k];
		}
	}
}

void GeneralEvaluation::TempResult::doOptional(vector<bool> &binding, TempResult &x, TempResult &rn, TempResult &ra, bool add_no_binding)
{
	int this_id_cols = this->id_varset.getVarsetSize();
	int x_id_cols = x.id_varset.getVarsetSize();

	int rn_id_cols = rn.id_varset.getVarsetSize();
	vector<int> this2rn_id_pos = this->id_varset.mapTo(rn.id_varset);
	vector<int> x2rn_id_pos = x.id_varset.mapTo(rn.id_varset);

	int ra_id_cols = ra.id_varset.getVarsetSize();
	vector<int> this2ra_id_pos = this->id_varset.mapTo(ra.id_varset);
	vector<int> x2ra_id_pos = x.id_varset.mapTo(ra.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();
	int x_str_cols = x.str_varset.getVarsetSize();

	int rn_str_cols = rn.str_varset.getVarsetSize();
	vector<int> this2rn_str_pos = this->str_varset.mapTo(rn.str_varset);
	vector<int> x2rn_str_pos = x.str_varset.mapTo(rn.str_varset);

	int ra_str_cols = ra.str_varset.getVarsetSize();
	vector<int> this2ra_str_pos = this->str_varset.mapTo(ra.str_varset);
	vector<int> x2ra_str_pos = x.str_varset.mapTo(ra.str_varset);

	Varset common = this->getAllVarset() * x.getAllVarset();

	if (common.empty())
	{
		this->doJoin(x, ra);

		binding.resize((int)this->result.size(), true);
	}
	else if (!x.result.empty())
	{
		vector<int> common2this = common.mapTo(this->getAllVarset());
		vector<int> common2x = common.mapTo(x.getAllVarset());
		x.sort(0, (int)x.result.size() - 1, common2x);

		int this_id_cols = this->id_varset.getVarsetSize();
		for (int i = 0; i < (int)this->result.size(); i++)
		{
			int left = x.findLeftBounder(common2x, this->result[i], this_id_cols, common2this);
			if (left == -1)	continue;
			int right = x.findRightBounder(common2x, this->result[i], this_id_cols, common2this);

			binding[i] = true;

			for (int j = left; j <= right; j++)
			{
				ra.result.push_back(ResultPair());

				if (ra_id_cols > 0)
				{
					ra.result.back().id = new unsigned [ra_id_cols];
					unsigned *v = ra.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2ra_id_pos[k]] = this->result[i].id[k];
					for (int k = 0; k < x_id_cols; k++)
						v[x2ra_id_pos[k]] = x.result[j].id[k];
				}

				if (ra_str_cols > 0)
				{
					ra.result.back().str.resize(ra_str_cols);
					vector<string> &v = ra.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2ra_str_pos[k]] = this->result[i].str[k];
					for (int k = 0; k < x_str_cols; k++)
						v[x2ra_str_pos[k]] = x.result[j].str[k];
				}
			}
		}
	}

	if (add_no_binding)
	{
		for (int i = 0; i < (int)this->result.size(); i++)
			if (!binding[i])
			{
				rn.result.push_back(ResultPair());

				if (rn_id_cols > 0)
				{
					rn.result.back().id = new unsigned [rn_id_cols];
					unsigned *v = rn.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2rn_id_pos[k]] = this->result[i].id[k];
				}

				if (rn_str_cols > 0)
				{
					rn.result.back().str.resize(rn_str_cols);
					vector<string> &v = rn.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2rn_str_pos[k]] = this->result[i].str[k];
				}
			}
	}
}

void GeneralEvaluation::TempResult::doMinus(TempResult &x, TempResult &r)
{
	int this_id_cols = this->id_varset.getVarsetSize();

	int r_id_cols = r.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this->id_varset.mapTo(r.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();

	int r_str_cols = r.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this->str_varset.mapTo(r.str_varset);

	Varset common = this->getAllVarset() * x.getAllVarset();

	if (common.empty())
	{
		for (int i = 0; i < (int)this->result.size(); i++)
		{
			r.result.push_back(ResultPair());

			if (r_id_cols > 0)
			{
				r.result.back().id = new unsigned [r_id_cols];
				unsigned *v = r.result.back().id;

				for (int k = 0; k < this_id_cols; k++)
					v[this2r_id_pos[k]] = this->result[i].id[k];
			}

			if (r_str_cols > 0)
			{
				r.result.back().str.resize(r_str_cols);
				vector<string> &v = r.result.back().str;

				for (int k = 0; k < this_str_cols; k++)
					v[this2r_str_pos[k]] = this->result[i].str[k];
			}
		}
	}
	else if (!x.result.empty())
	{
		vector<int> common2this = common.mapTo(this->getAllVarset());
		vector<int> common2x = common.mapTo(x.getAllVarset());
		x.sort(0, (int)x.result.size() - 1, common2x);

		int this_id_cols = this->id_varset.getVarsetSize();
		for (int i = 0; i < (int)this->result.size(); i++)
		{
			int left = x.findLeftBounder(common2x, this->result[i], this_id_cols, common2this);
			if (left == -1)
			{
				r.result.push_back(ResultPair());

				if (r_id_cols > 0)
				{
					r.result.back().id = new unsigned [r_id_cols];
					unsigned *v = r.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2r_id_pos[k]] = this->result[i].id[k];
				}

				if (r_str_cols > 0)
				{
					r.result.back().str.resize(r_str_cols);
					vector<string> &v = r.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2r_str_pos[k]] = this->result[i].str[k];
				}
			}
		}
	}
}

void GeneralEvaluation::TempResult::getFilterString(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild &child, FilterEvaluationMultitypeValue &femv, ResultPair &row, int id_cols, StringIndex *stringindex)
{
	if (child.node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
	{
		femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::rdf_term;

		if (child.str[0] == '?')
		{
			if (child.pos == -1)
			{
				femv.datatype = FilterEvaluationMultitypeValue::xsd_boolean;
				femv.bool_value = FilterEvaluationMultitypeValue::EffectiveBooleanValue::error_value;

				return;
			}

			if (child.pos < id_cols)
			{
				int id = row.id[child.pos];

				if (child.isel)
					stringindex->randomAccess(id, &femv.term_value, true);
				else
					stringindex->randomAccess(id, &femv.term_value, false);
			}
			else
			{
				femv.term_value = row.str[child.pos - id_cols];
			}
		}
		else
		{
			femv.term_value = child.str;
		}

		if (femv.term_value[0] == '<' && femv.term_value[femv.term_value.length() - 1] == '>')
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::iri;
			femv.str_value = femv.term_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("\"^^<") == string::npos && femv.term_value[femv.term_value.length() - 1] != '>' )
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::literal;
			femv.str_value = femv.term_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#string>") != string::npos)
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_string;
			femv.str_value = femv.term_value.substr(0, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#string>"));
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#boolean>") != string::npos)
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_boolean;

			string value = femv.term_value.substr(0, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#boolean>"));
			if (value == "\"true\"")
				femv.bool_value = GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
			else if (value == "\"false\"")
				femv.bool_value = GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
			else
				femv.bool_value = GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::error_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#integer>") != string::npos)
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_integer;

			string value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#integer>") - 2);
			stringstream ss;
			ss << value;
			ss >> femv.int_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#decimal>") != string::npos)
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_decimal;

			string value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#decimal>") - 2);
			stringstream ss;
			ss << value;
			ss >> femv.flt_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#float>") != string::npos)
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_float;

			string value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#float>") - 2);
			stringstream ss;
			ss << value;
			ss >> femv.flt_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#double>") != string::npos)
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_double;

			string value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#double>") - 2);
			stringstream ss;
			ss << value;
			ss >> femv.dbl_value;
		}

		if (femv.term_value[0] == '"' &&
				(femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#dateTime>") != string::npos || femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#date>") != string::npos))
		{
			femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_datetime;

			string value;
			if (femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#dateTime>") != string::npos)
				value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#dateTime>") - 2);
			if (femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#date>") != string::npos)
				value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#date>") - 2);

			vector <int> date;
			for (int i = 0; i < 6; i++)
				if (value.length() > 0)
				{
					int p = 0;
					stringstream ss;
					while (p < (int)value.length() && '0' <= value[p] && value[p] <= '9')
					{
						ss << value[p];
						p++;
					}

					int x;
					ss >> x;
					date.push_back(x);

					while (p < (int)value.length() && (value[p] < '0' || value[p] > '9'))
						p++;
					value = value.substr(p);
				}
				else
					date.push_back(0);

			femv.dt_value = GeneralEvaluation::FilterEvaluationMultitypeValue::DateTime(date[0], date[1], date[2], date[3], date[4], date[5]);
		}
	}
}

GeneralEvaluation::FilterEvaluationMultitypeValue
	GeneralEvaluation::TempResult::matchFilterTree(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, ResultPair &row, int id_cols, StringIndex *stringindex)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_boolean;
	ret_femv.bool_value = GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::error_value;

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Not_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		return !x;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Or_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);

		return x || y;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::And_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);

		return x && y;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Equal_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);

		return x == y;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::NotEqual_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);

		return x != y;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Less_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);

		return x < y;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::LessOrEqual_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);

		return x <= y;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Greater_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);

		return x > y;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::GreaterOrEqual_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);

		return x >= y;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_regex_type)
	{
		FilterEvaluationMultitypeValue x, y, z;
		string t, p, f;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);
		if (x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::literal || x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_string)
		{
			t = x.str_value;
			t = t.substr(1, t.rfind('"') - 1);
		}
		else
			return ret_femv;

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);
		if (y.isSimpleLiteral())
		{
			p = y.str_value;
			p = p.substr(1, p.rfind('"') - 1);
		}
		else
			return ret_femv;

		if ((int)filter.child.size() >= 3)
		{
			if (filter.child[2].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
				getFilterString(filter.child[2], z, row, id_cols, stringindex);
			else if (filter.child[2].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
				z = matchFilterTree(filter.child[2].node, row, id_cols, stringindex);
			if (z.isSimpleLiteral())
			{
				f = z.str_value;
				f = f.substr(1, f.rfind('"') - 1);
			}
			else
				return ret_femv;
		}

		RegexExpression re;
		if (!re.compile(p, f))
			ret_femv.bool_value = GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
		else
			ret_femv.bool_value = re.match(t);

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_str_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::literal)
		{
			ret_femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::literal;

			ret_femv.str_value = x.str_value.substr(0, x.str_value.rfind('"') + 1);

			return ret_femv;
		}
		else if (x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::iri)
		{
			ret_femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::literal;

			ret_femv.str_value = "\"" + x.str_value.substr(1, x.str_value.length() - 2) + "\"";

			return ret_femv;
		}
		else if (x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_string)
		{
			ret_femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::literal;

			ret_femv.str_value = x.str_value;

			return ret_femv;
		}
		else
			return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isiri_type ||
		filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isuri_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		ret_femv.bool_value = (x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::iri);

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isliteral_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		ret_femv.bool_value = (x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::literal);

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isnumeric_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		ret_femv.bool_value = (x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_integer ||
							   x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_decimal ||
							   x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_float ||
							   x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_double);

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_lang_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (x.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::literal)
		{
			ret_femv.datatype = GeneralEvaluation::FilterEvaluationMultitypeValue::literal;

			int p = x.str_value.rfind('@');
			if (p != -1)
				ret_femv.str_value = "\"" + x.str_value.substr(p + 1) + "\"";
			else
				ret_femv.str_value = "";

			return ret_femv;
		}
		else
			return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_langmatches_type)
	{
		FilterEvaluationMultitypeValue x, y;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);
		if (!x.isSimpleLiteral())
			return ret_femv;

		if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[1], y, row, id_cols, stringindex);
		else if (filter.child[1].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			y = matchFilterTree(filter.child[1].node, row, id_cols, stringindex);
		if (!y.isSimpleLiteral())
			return ret_femv;

		ret_femv.bool_value = ((x.str_value == y.str_value) || (x.str_value.length() > 0 && y.str_value == "\"*\""));

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_bound_type)
	{
		ret_femv.bool_value = (filter.child[0].pos != -1);

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_in_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		for (int i = 1; i < (int)filter.child.size(); i++)
		{
			FilterEvaluationMultitypeValue y;

			if (filter.child[i].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
				getFilterString(filter.child[i], y, row, id_cols, stringindex);
			else if (filter.child[i].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
				y = matchFilterTree(filter.child[i].node, row, id_cols, stringindex);

			FilterEvaluationMultitypeValue equal = (x == y);
			if (i == 1)
				ret_femv = equal;
			else
				ret_femv = (ret_femv || equal);

			if (ret_femv.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_boolean && ret_femv.bool_value.value == GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value)
				return ret_femv;
		}

		return ret_femv;
	}

	return ret_femv;
}

void GeneralEvaluation::TempResult::doFilter(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, TempResult &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	Varset this_varset = this->getAllVarset();
	filter.mapVarPos2Varset(this_varset, entity_literal_varset);

	int this_id_cols = this->id_varset.getVarsetSize();

	int r_id_cols = r.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this->id_varset.mapTo(r.id_varset);

	int this_str_cols = this->str_varset.getVarsetSize();

	int r_str_cols = r.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this->str_varset.mapTo(r.str_varset);

	for (int i = 0; i < (int)this->result.size(); i++)
	{
		GeneralEvaluation::FilterEvaluationMultitypeValue ret_femv = matchFilterTree(filter, this->result[i], this_id_cols, stringindex);

		if (ret_femv.datatype == GeneralEvaluation::FilterEvaluationMultitypeValue::xsd_boolean && ret_femv.bool_value.value == GeneralEvaluation::FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value)
		{
			r.result.push_back(ResultPair());

			if (r_id_cols > 0)
			{
				r.result.back().id = new unsigned [r_id_cols];
				unsigned *v = r.result.back().id;

				for (int k = 0; k < this_id_cols; k++)
					v[this2r_id_pos[k]] = this->result[i].id[k];
			}

			if (r_str_cols > 0)
			{
				r.result.back().str.resize(r_str_cols);
				vector<string> &v = r.result.back().str;

				for (int k = 0; k < this_str_cols; k++)
					v[this2r_str_pos[k]] = this->result[i].str[k];
			}
		}
	}
}

void GeneralEvaluation::TempResult::print(int no)
{
	this->getAllVarset().print();

	if (no == -1)
		printf("temp result:\n");
	else
		printf("temp result no.%d:\n", no);

	for (int i = 0; i < (int)this->result.size(); i++)
	{
		for (int j = 0; j < this->id_varset.getVarsetSize(); j++)
			printf("%d\t", this->result[i].id[j]);
		for (int j = 0; j < this->str_varset.getVarsetSize(); j++)
			printf("%s\t", this->result[i].str[j].c_str());
		printf("\n");
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void GeneralEvaluation::TempResultSet::release()
{
	for (int i = 0; i < (int)this->results.size(); i++)
		this->results[i].release();
}

int GeneralEvaluation::TempResultSet::findCompatibleResult(Varset &_id_varset, Varset &_str_varset)
{
	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset == _id_varset && this->results[i].str_varset == _str_varset)
			return i;

	this->results.push_back(TempResult());
	this->results.back().id_varset = _id_varset;
	this->results.back().str_varset = _str_varset;

	return (int)this->results.size() - 1;
}

void GeneralEvaluation::TempResultSet::doJoin(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	if (this->results.empty() || x.results.empty())
	{
		this->doUnion(x, r);
		return;
	}

	Varset this_str_varset, x_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		this_str_varset += this->results[i].str_varset;
	for (int i = 0; i < (int)x.results.size(); i++)
		x_str_varset += x.results[i].str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(x_str_varset))
			this->results[i].convertId2Str(this->results[i].id_varset * x_str_varset, stringindex, entity_literal_varset);
	for (int i = 0; i < (int)x.results.size(); i++)
		if (x.results[i].id_varset.hasCommonVar(this_str_varset))
			x.results[i].convertId2Str(x.results[i].id_varset * this_str_varset, stringindex, entity_literal_varset);

	for (int i = 0; i < (int)this->results.size(); i++)
		for (int j = 0; j < (int)x.results.size(); j++)
		{
			Varset id_varset = this->results[i].id_varset + x.results[j].id_varset;
			Varset str_varset = this->results[i].str_varset + x.results[j].str_varset;

			int pos = r.findCompatibleResult(id_varset, str_varset);
			this->results[i].doJoin(x.results[j], r.results[pos]);
		}

	long tv_end = Util::get_cur_time();
	printf("after doJoin, used %ld ms.\n", tv_end - tv_begin);
}

void GeneralEvaluation::TempResultSet::doUnion(TempResultSet &x, TempResultSet &r)
{
	long tv_begin = Util::get_cur_time();

	for (int i = 0; i < (int)this->results.size(); i++)
	{
		int pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);
		this->results[i].doUnion(r.results[pos]);
	}

	for (int i = 0; i < (int)x.results.size(); i++)
	{
		int pos = r.findCompatibleResult(x.results[i].id_varset, x.results[i].str_varset);
		x.results[i].doUnion(r.results[pos]);
	}

	long tv_end = Util::get_cur_time();
	printf("after doUnion, used %ld ms.\n", tv_end - tv_begin);
}

void GeneralEvaluation::TempResultSet::doOptional(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	if (this->results.empty() || x.results.empty())
	{
		this->doUnion(x, r);
		return;
	}

	Varset this_str_varset, x_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		this_str_varset += this->results[i].str_varset;
	for (int i = 0; i < (int)x.results.size(); i++)
		x_str_varset += x.results[i].str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(x_str_varset))
			this->results[i].convertId2Str(this->results[i].id_varset * x_str_varset, stringindex, entity_literal_varset);
	for (int i = 0; i < (int)x.results.size(); i++)
		if (x.results[i].id_varset.hasCommonVar(this_str_varset))
			x.results[i].convertId2Str(x.results[i].id_varset * this_str_varset, stringindex, entity_literal_varset);

	for (int i = 0; i < (int)this->results.size(); i++)
	{
		vector<bool> binding((int)this->results[i].result.size(), false);

		int noadd_pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);

		for (int j = 0; j < (int)x.results.size(); j++)
		{
			Varset add_id_varset = this->results[i].id_varset + x.results[j].id_varset;
			Varset add_str_varset = this->results[i].str_varset + x.results[j].str_varset;

			int add_pos = r.findCompatibleResult(add_id_varset, add_str_varset);
			this->results[i].doOptional(binding, x.results[j], r.results[noadd_pos], r.results[add_pos], j + 1 == (int)x.results.size());
		}
	}

	long tv_end = Util::get_cur_time();
	printf("after doOptional, used %ld ms.\n", tv_end - tv_begin);
}

void GeneralEvaluation::TempResultSet::doMinus(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	if (x.results.empty())
	{
		this->doUnion(x, r);
		return;
	}

	Varset this_str_varset, x_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		this_str_varset += this->results[i].str_varset;
	for (int i = 0; i < (int)x.results.size(); i++)
		x_str_varset += x.results[i].str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(x_str_varset))
			this->results[i].convertId2Str(this->results[i].id_varset * x_str_varset, stringindex, entity_literal_varset);
	for (int i = 0; i < (int)x.results.size(); i++)
		if (x.results[i].id_varset.hasCommonVar(this_str_varset))
			x.results[i].convertId2Str(x.results[i].id_varset * this_str_varset, stringindex, entity_literal_varset);

	for (int i = 0; i < (int)this->results.size(); i++)
	{
		vector<TempResult> temp;

		for (int j = 0; j < (int)x.results.size(); j++)
		{
			if (j == 0 && j + 1 == (int)x.results.size())
			{
				int pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);
				this->results[i].doMinus(x.results[j], r.results[pos]);
			}
			else if (j == 0)
			{
				temp.push_back(TempResult());
				temp[0].id_varset = this->results[i].id_varset;
				temp[0].str_varset = this->results[i].str_varset;
				this->results[i].doMinus(x.results[j], temp[0]);
			}
			else if (j + 1 == (int)x.results.size())
			{
				int pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);
				temp[j - 1].doMinus(x.results[j], r.results[pos]);
			}
			else
			{
				temp.push_back(TempResult());
				temp[j].id_varset = this->results[i].id_varset;
				temp[j].str_varset = this->results[i].str_varset;
				temp[j - 1].doMinus(x.results[j], temp[j]);
			}
		}
	}

	long tv_end = Util::get_cur_time();
	printf("after doMinus, used %ld ms.\n", tv_end - tv_begin);
}

void GeneralEvaluation::TempResultSet::doFilter(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	for (int i = 0; i < (int)this->results.size(); i++)
	{
		int pos = r.findCompatibleResult(this->results[i].id_varset, this->results[i].str_varset);
		this->results[i].doFilter(filter, r.results[pos], stringindex, entity_literal_varset);
	}

	long tv_end = Util::get_cur_time();
	printf("after doFilter, used %ld ms.\n", tv_end - tv_begin);
}

void GeneralEvaluation::TempResultSet::doProjection1(Varset &proj, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
{
	long tv_begin = Util::get_cur_time();

	Varset this_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		this_str_varset += this->results[i].str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(this_str_varset * proj))
			this->results[i].convertId2Str(this->results[i].id_varset * this_str_varset * proj, stringindex, entity_literal_varset);

	Varset r_id_varset, r_str_varset;

	for (int i = 0; i < (int)this->results.size(); i++)
		r_str_varset += this->results[i].str_varset * proj;
	r_id_varset = proj - r_str_varset;

	if (!r.results.empty())
		return;

	r.findCompatibleResult(r_id_varset, r_str_varset);

	int r_id_cols = r_id_varset.getVarsetSize();
	int r_str_cols = r_str_varset.getVarsetSize();

	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset.hasCommonVar(r_id_varset) || this->results[i].str_varset.hasCommonVar(r_str_varset))
		{
			vector<int> r2this_id_pos = r_id_varset.mapTo(this->results[i].id_varset);
			vector<int> r2this_str_pos = r_str_varset.mapTo(this->results[i].str_varset);

			for (int j = 0; j < (int)this->results[i].result.size(); j++)
			{
				r.results[0].result.push_back(TempResult::ResultPair());

				if (r_id_cols > 0)
				{
					r.results[0].result.back().id = new unsigned [r_id_cols];
					unsigned *v = r.results[0].result.back().id;

					for (int k = 0; k < r_id_cols; k++)
						if (r2this_id_pos[k] == -1)
							v[k] = INVALID;
						else
							v[k] = this->results[i].result[j].id[r2this_id_pos[k]];
				}

				if (r_str_cols > 0)
				{
					r.results[0].result.back().str.resize(r_str_cols);
					vector<string> &v = r.results[0].result.back().str;

					for (int k = 0; k < r_str_cols; k++)
						if (r2this_str_pos[k] != -1)
							v[k] = this->results[i].result[j].str[r2this_str_pos[k]];
				}
			}
		}

	long tv_end = Util::get_cur_time();
	printf("after doProjection, used %ld ms.\n", tv_end - tv_begin);
}

void GeneralEvaluation::TempResultSet::doDistinct1(TempResultSet &r)
{
	long tv_begin = Util::get_cur_time();

	if ((int)this->results.size() != 1)
		return;

	TempResult &this_results0 = this->results[0];

	if (!r.results.empty())
		return;

	r.findCompatibleResult(this_results0.id_varset, this_results0.str_varset);
	TempResult &r_results0 = r.results[0];

	int this_id_cols = this_results0.id_varset.getVarsetSize();

	int r_id_cols = r_results0.id_varset.getVarsetSize();
	vector<int> this2r_id_pos = this_results0.id_varset.mapTo(r_results0.id_varset);

	int this_str_cols = this_results0.str_varset.getVarsetSize();

	int r_str_cols = r_results0.str_varset.getVarsetSize();
	vector<int> this2r_str_pos = this_results0.str_varset.mapTo(r_results0.str_varset);

	if (!this_results0.result.empty())
	{
		vector<int> r2this = r_results0.getAllVarset().mapTo(this_results0.getAllVarset());
		this_results0.sort(0, (int)this_results0.result.size() - 1, r2this);

		int this_result0_id_cols = this_results0.id_varset.getVarsetSize();
		for (int i = 0; i < (int)this_results0.result.size(); i++)
			if (i == 0 || TempResult::compareRow(this_results0.result[i], this_result0_id_cols, r2this, this_results0.result[i - 1], this_result0_id_cols, r2this) != 0)
			{
				r_results0.result.push_back(TempResult::ResultPair());

				if (r_id_cols > 0)
				{
					r_results0.result.back().id = new unsigned [r_id_cols];
					unsigned *v = r_results0.result.back().id;

					for (int k = 0; k < this_id_cols; k++)
						v[this2r_id_pos[k]] = this_results0.result[i].id[k];
				}

				if (r_str_cols > 0)
				{
					r_results0.result.back().str.resize(r_str_cols);
					vector<string> &v = r_results0.result.back().str;

					for (int k = 0; k < this_str_cols; k++)
						v[this2r_str_pos[k]] = this_results0.result[i].str[k];
				}
			}
	}

	long tv_end = Util::get_cur_time();
	printf("after doDistinct, used %ld ms.\n", tv_end - tv_begin);
}

void GeneralEvaluation::TempResultSet::print()
{
	printf("total temp result: %d\n", (int)this->results.size());
	for (int i = 0; i < (int)this->results.size(); i++)
		this->results[i].print(i);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

GeneralEvaluation::TempResultSet* GeneralEvaluation::semanticBasedQueryEvaluation(QueryTree::GroupPattern &grouppattern)
{
	TempResultSet* result = new TempResultSet();

	grouppattern.initPatternBlockid();

	for (int i = 0; i < (int)grouppattern.sub_grouppattern.size(); i++)
		if (grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
		{
			int st = i;
			while (st > 0 && (grouppattern.sub_grouppattern[st - 1].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type || grouppattern.sub_grouppattern[st - 1].type == QueryTree::GroupPattern::SubGroupPattern::Union_type))
				st--;

			for (int j = st; j < i; j++)
				if (grouppattern.sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
				{
					if (grouppattern.sub_grouppattern[i].pattern.subject_object_varset.hasCommonVar(grouppattern.sub_grouppattern[j].pattern.subject_object_varset))
						grouppattern.mergePatternBlockID(i, j);
				}

			if (i + 1 == (int)grouppattern.sub_grouppattern.size() ||
				(grouppattern.sub_grouppattern[i + 1].type != QueryTree::GroupPattern::SubGroupPattern::Pattern_type && grouppattern.sub_grouppattern[i + 1].type != QueryTree::GroupPattern::SubGroupPattern::Union_type))
			{
				SPARQLquery sparql_query;
				vector<vector<string> > encode_varset;

				for (int j = st; j <= i; j++)
					if (grouppattern.sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
						if (!grouppattern.sub_grouppattern[j].pattern.varset.empty())
						{
							if (grouppattern.getRootPatternBlockID(j) == j)		//root node
							{
								sparql_query.addBasicQuery();
								Varset occur;

								for (int k = st; k <= i; k++)
									if (grouppattern.sub_grouppattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
										if (grouppattern.getRootPatternBlockID(k) == j)
										{
											sparql_query.addTriple(Triple(grouppattern.sub_grouppattern[k].pattern.subject.value,
																		  grouppattern.sub_grouppattern[k].pattern.predicate.value,
																		  grouppattern.sub_grouppattern[k].pattern.object.value));

											occur += grouppattern.sub_grouppattern[k].pattern.varset;
										}

								encode_varset.push_back(occur.vars);

								printf("select vars: ");
								occur.print();

								printf("triple patterns: \n");
								for (int k = st; k <= i; k++)
									if (grouppattern.sub_grouppattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
										if (grouppattern.getRootPatternBlockID(k) == j)
										{
											printf("%s\t%s\t%s\n", 	grouppattern.sub_grouppattern[k].pattern.subject.value.c_str(),
																	grouppattern.sub_grouppattern[k].pattern.predicate.value.c_str(),
																	grouppattern.sub_grouppattern[k].pattern.object.value.c_str()
											);
										}
							}
						}

				long tv_begin = Util::get_cur_time();
				sparql_query.encodeQuery(this->kvstore, encode_varset);
				long tv_encode = Util::get_cur_time();
				printf("after Encode, used %ld ms.\n", tv_encode - tv_begin);

				this->strategy.handle(sparql_query);
				long tv_handle = Util::get_cur_time();
				printf("after Handle, used %ld ms.\n", tv_handle - tv_encode);

				//collect and join the result of each BasicQuery
				for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
				{
					TempResultSet *temp = new TempResultSet();
					temp->results.push_back(TempResult());

					temp->results[0].id_varset = Varset(encode_varset[j]);
					int varnum = (int)encode_varset[j].size();

					vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
					int basicquery_result_num = (int)basicquery_result.size();

					temp->results[0].result.reserve(basicquery_result_num);
					for (int k = 0; k < basicquery_result_num; k++)
					{
						unsigned *v = new unsigned [varnum];
						memcpy(v, basicquery_result[k], sizeof(int) * varnum);
						temp->results[0].result.push_back(TempResult::ResultPair());
						temp->results[0].result.back().id = v;
					}

					if (result->results.empty())
					{
						delete result;
						result = temp;
					}
					else
					{
						TempResultSet *new_result = new TempResultSet();
						result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);

						temp->release();
						result->release();
						delete temp;
						delete result;

						result = new_result;
					}
				}
			}
		}
		else if (grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Union_type)
		{
			TempResultSet *sub_result = new TempResultSet();

			for (int j = 0; j < (int)grouppattern.sub_grouppattern[i].unions.size(); j++)
			{
				TempResultSet *temp = semanticBasedQueryEvaluation(grouppattern.sub_grouppattern[i].unions[j]);

				if (sub_result->results.empty())
				{
					delete sub_result;
					sub_result = temp;
				}
				else
				{
					TempResultSet *new_result = new TempResultSet();
					sub_result->doUnion(*temp, *new_result);

					temp->release();
					sub_result->release();
					delete temp;
					delete sub_result;

					sub_result = new_result;
				}
			}

			if (result->results.empty())
			{
				delete result;
				result = sub_result;
			}
			else
			{
				TempResultSet *new_result = new TempResultSet();
				result->doJoin(*sub_result, *new_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);

				sub_result->release();
				result->release();
				delete sub_result;
				delete result;

				result = new_result;
			}
		}
		else if (grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type || grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Minus_type)
		{
			TempResultSet *temp = semanticBasedQueryEvaluation(grouppattern.sub_grouppattern[i].optional);

			{
				TempResultSet *new_result = new TempResultSet();

				if (grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
					result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);
				else if (grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Minus_type)
					result->doMinus(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
			}
		}
		else if (grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
		{
			TempResultSet *new_result = new TempResultSet();
			result->doFilter(grouppattern.sub_grouppattern[i].filter.root, *new_result, this->stringindex, grouppattern.grouppattern_subject_object_maximal_varset);

			result->release();
			delete result;

			result = new_result;
		}
		else if (grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Bind_type)
		{
			TempResultSet *temp = new TempResultSet();
			temp->results.push_back(TempResult());

			temp->results[0].str_varset = grouppattern.sub_grouppattern[i].bind.varset;

			temp->results[0].result.push_back(TempResult::ResultPair());
			temp->results[0].result[0].str.push_back(grouppattern.sub_grouppattern[i].bind.str);

			{
				TempResultSet *new_result = new TempResultSet();
				result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
			}
		}

	return result;
}

bool GeneralEvaluation::expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &grouppattern, deque<QueryTree::GroupPattern> &queue)
{
	int first_union_pos = -1;

	for (int i = 0; i < (int)grouppattern.sub_grouppattern.size(); i++)
		if (grouppattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Union_type)
		{
			first_union_pos = i;
			break;
		}

	if (first_union_pos == -1)
		return false;

	for (int i = 0; i < (int)grouppattern.sub_grouppattern[first_union_pos].unions.size(); i++)
	{
		QueryTree::GroupPattern new_grouppattern;

		for (int j = 0; j < first_union_pos; j++)
			new_grouppattern.sub_grouppattern.push_back(grouppattern.sub_grouppattern[j]);

		for (int j = 0; j < (int)grouppattern.sub_grouppattern[first_union_pos].unions[i].sub_grouppattern.size(); j++)
			new_grouppattern.sub_grouppattern.push_back(grouppattern.sub_grouppattern[first_union_pos].unions[i].sub_grouppattern[j]);

		for (int j = first_union_pos + 1; j < (int)grouppattern.sub_grouppattern.size(); j++)
			new_grouppattern.sub_grouppattern.push_back(grouppattern.sub_grouppattern[j]);

		queue.push_back(new_grouppattern);
	}

	return true;
}

GeneralEvaluation::TempResultSet* GeneralEvaluation::rewritingBasedQueryEvaluation(int dep)
{
	deque<QueryTree::GroupPattern> queue;
	queue.push_back(this->rewriting_evaluation_stack[dep]);
	vector<QueryTree::GroupPattern> grouppattern_union;

	while (!queue.empty())
	{
		if (!this->expanseFirstOuterUnionGroupPattern(queue.front(), queue))
			grouppattern_union.push_back(queue.front());
		queue.pop_front();
	}

	TempResultSet *result = new TempResultSet();

	for (int i = 0; i < (int)grouppattern_union.size(); i++)
	{
		this->rewriting_evaluation_stack[dep] = grouppattern_union[i];
		QueryTree::GroupPattern *grouppattern = &this->rewriting_evaluation_stack[dep];
		grouppattern->getVarset();

		for (int j = 0; j < 80; j++)			printf("=");	printf("\n");
		grouppattern->print(dep);
		for (int j = 0; j < 80; j++)			printf("=");	printf("\n");

		TempResultSet *sub_result = new TempResultSet();

		QueryTree::GroupPattern triplepattern;
		int grouppattern_triple_num = 0;
		for (int j = 0; j < (int)grouppattern->sub_grouppattern.size(); j++)
			if (grouppattern->sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
			{
				triplepattern.addOnePattern(QueryTree::GroupPattern::Pattern(
					QueryTree::GroupPattern::Pattern::Element(grouppattern->sub_grouppattern[j].pattern.subject.value),
					QueryTree::GroupPattern::Pattern::Element(grouppattern->sub_grouppattern[j].pattern.predicate.value),
					QueryTree::GroupPattern::Pattern::Element(grouppattern->sub_grouppattern[j].pattern.object.value)
				));
				grouppattern_triple_num++;
			}
		triplepattern.getVarset();

		for (int j = 0; j < dep; j++)
		{
			QueryTree::GroupPattern &parrent_grouppattern = this->rewriting_evaluation_stack[j];

			for (int k = 0; k < (int)parrent_grouppattern.sub_grouppattern.size(); k++)
				if (parrent_grouppattern.sub_grouppattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
					if (triplepattern.grouppattern_subject_object_maximal_varset.hasCommonVar(parrent_grouppattern.sub_grouppattern[k].pattern.subject_object_varset))
					{
						triplepattern.addOnePattern(QueryTree::GroupPattern::Pattern(
							QueryTree::GroupPattern::Pattern::Element(parrent_grouppattern.sub_grouppattern[k].pattern.subject.value),
							QueryTree::GroupPattern::Pattern::Element(parrent_grouppattern.sub_grouppattern[k].pattern.predicate.value),
							QueryTree::GroupPattern::Pattern::Element(parrent_grouppattern.sub_grouppattern[k].pattern.object.value)
						));
					}
		}
		triplepattern.getVarset();

		//get useful varset
		Varset useful = this->query_tree.getResultProjectionVarset() + this->query_tree.getGroupByVarset();
		if (!this->query_tree.checkProjectionAsterisk())
		{
			for (int j = 0; j < dep; j++)
			{
				QueryTree::GroupPattern &parrent_grouppattern = this->rewriting_evaluation_stack[j];

				for (int k = 0; k < (int)parrent_grouppattern.sub_grouppattern.size(); k++)
				{
					if (parrent_grouppattern.sub_grouppattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
						useful += parrent_grouppattern.sub_grouppattern[k].pattern.varset;
					else if (parrent_grouppattern.sub_grouppattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
						useful += parrent_grouppattern.sub_grouppattern[k].filter.varset;
				}
			}

			for (int j = 0; j < (int)grouppattern->sub_grouppattern.size(); j++)
			{
				if (grouppattern->sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
					useful += grouppattern->sub_grouppattern[j].optional.grouppattern_resultset_maximal_varset;
				else if (grouppattern->sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
					useful += grouppattern->sub_grouppattern[j].filter.varset;
			}
		}

		SPARQLquery sparql_query;
		vector<vector<string> > encode_varset;

		//get connected block
		triplepattern.initPatternBlockid();

		for (int j = 0; j < (int)triplepattern.sub_grouppattern.size(); j++)
			if (triplepattern.sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
			{
				for (int k = 0; k < j; k++)
					if (triplepattern.sub_grouppattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
						if (triplepattern.sub_grouppattern[j].pattern.subject_object_varset.hasCommonVar(triplepattern.sub_grouppattern[k].pattern.subject_object_varset))
							triplepattern.mergePatternBlockID(j, k);
			}

		//each connected block is a BasicQuery
		for (int j = 0; j < (int)triplepattern.sub_grouppattern.size(); j++)
			if (triplepattern.sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
				if (triplepattern.getRootPatternBlockID(j) == j)
				{
					sparql_query.addBasicQuery();
					Varset occur;

					for (int k = 0; k < (int)triplepattern.sub_grouppattern.size(); k++)
						if (triplepattern.sub_grouppattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
							if (triplepattern.getRootPatternBlockID(k) == j)
							{
								sparql_query.addTriple(Triple(
									triplepattern.sub_grouppattern[k].pattern.subject.value,
									triplepattern.sub_grouppattern[k].pattern.predicate.value,
									triplepattern.sub_grouppattern[k].pattern.object.value
								));

								if (j < grouppattern_triple_num)
									occur += triplepattern.sub_grouppattern[k].pattern.varset;
							}

					//reduce return result vars
					if (!this->query_tree.checkProjectionAsterisk() && useful.hasCommonVar(occur))
						useful = useful * occur;
					else
						useful = occur;

					encode_varset.push_back(useful.vars);

					printf("select vars: ");
					useful.print();

					printf("triple patterns: \n");
					for (int k = 0; k < (int)triplepattern.sub_grouppattern.size(); k++)
						if (triplepattern.sub_grouppattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
							if (triplepattern.getRootPatternBlockID(k) == j)
							{
								printf("%s\t%s\t%s\n", 	triplepattern.sub_grouppattern[k].pattern.subject.value.c_str(),
														triplepattern.sub_grouppattern[k].pattern.predicate.value.c_str(),
														triplepattern.sub_grouppattern[k].pattern.object.value.c_str()
								);
							}
				}

		long tv_begin = Util::get_cur_time();
		sparql_query.encodeQuery(this->kvstore, encode_varset);
		long tv_encode = Util::get_cur_time();
		printf("after Encode, used %ld ms.\n", tv_encode - tv_begin);

		if (dep > 0)
			this->strategy.handle(sparql_query, &this->result_filter);
		else
			this->strategy.handle(sparql_query);
		long tv_handle = Util::get_cur_time();
		printf("after Handle, used %ld ms.\n", tv_handle - tv_encode);

		//collect and join the result of each BasicQuery
		for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
		{
			TempResultSet *temp = new TempResultSet();
			temp->results.push_back(TempResult());

			temp->results[0].id_varset = Varset(encode_varset[j]);
			int varnum = (int)encode_varset[j].size();

			vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
			int basicquery_result_num = (int)basicquery_result.size();

			temp->results[0].result.reserve(basicquery_result_num);
			for (int k = 0; k < basicquery_result_num; k++)
			{
				unsigned *v = new unsigned [varnum];
				memcpy(v, basicquery_result[k], sizeof(int) * varnum);
				temp->results[0].result.push_back(TempResult::ResultPair());
				temp->results[0].result.back().id = v;
			}

			if (sub_result->results.empty())
			{
				delete sub_result;
				sub_result = temp;
			}
			else
			{
				TempResultSet *new_result = new TempResultSet();
				sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);

				temp->release();
				sub_result->release();
				delete temp;
				delete sub_result;

				sub_result = new_result;
			}
		}

		for (int j = 0; j < (int)grouppattern->sub_grouppattern.size(); j++)
			if (grouppattern->sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Bind_type)
			{
				TempResultSet *temp = new TempResultSet();
				temp->results.push_back(TempResult());

				temp->results[0].str_varset = grouppattern->sub_grouppattern[j].bind.varset;

				temp->results[0].result.push_back(TempResult::ResultPair());
				temp->results[0].result[0].str.push_back(grouppattern->sub_grouppattern[j].bind.str);

				TempResultSet *new_result = new TempResultSet();
				sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);

				temp->release();
				sub_result->release();
				delete temp;
				delete sub_result;

				sub_result = new_result;
			}

		for (int j = 0; j < (int)grouppattern->sub_grouppattern.size(); j++)
			if (grouppattern->sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
				if (!grouppattern->sub_grouppattern[j].filter.done && grouppattern->sub_grouppattern[j].filter.varset.belongTo(grouppattern->grouppattern_resultset_minimal_varset))
				{
					grouppattern->sub_grouppattern[j].filter.done = true;

					TempResultSet *new_result = new TempResultSet();
					sub_result->doFilter(grouppattern->sub_grouppattern[j].filter.root, *new_result, this->stringindex, grouppattern->grouppattern_subject_object_maximal_varset);

					sub_result->release();
					delete sub_result;

					sub_result = new_result;
				}

		if (!sub_result->results[0].result.empty())
		{
			bool has_changed = false;

			for (int j = 0; j < (int)grouppattern->sub_grouppattern.size(); j++)
			if (grouppattern->sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
			{
				if (!has_changed)
				{
					this->result_filter.changeResultHashTable(sparql_query, 1);
					has_changed = true;
				}

				if ((int)this->rewriting_evaluation_stack.size() == dep + 1)
				{
					this->rewriting_evaluation_stack.push_back(QueryTree::GroupPattern());
					grouppattern = &this->rewriting_evaluation_stack[dep];
				}

				this->rewriting_evaluation_stack[dep + 1] = grouppattern->sub_grouppattern[j].optional;

				TempResultSet *temp = rewritingBasedQueryEvaluation(dep + 1);

				TempResultSet *new_result = new TempResultSet();
				sub_result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);

				temp->release();
				sub_result->release();
				delete temp;
				delete sub_result;

				sub_result = new_result;
			}

			if (has_changed)
				this->result_filter.changeResultHashTable(sparql_query, -1);
		}

		for (int j = 0; j < (int)grouppattern->sub_grouppattern.size(); j++)
			if (grouppattern->sub_grouppattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
				if (!grouppattern->sub_grouppattern[j].filter.done)
				{
					grouppattern->sub_grouppattern[j].filter.done = true;

					TempResultSet *new_result = new TempResultSet();
					sub_result->doFilter(grouppattern->sub_grouppattern[j].filter.root, *new_result, this->stringindex, grouppattern->grouppattern_subject_object_maximal_varset);

					sub_result->release();
					delete sub_result;

					sub_result = new_result;
				}

		if (result->results.empty())
		{
			delete result;
			result = sub_result;
		}
		else
		{
			TempResultSet *new_result = new TempResultSet();
			result->doUnion(*sub_result, *new_result);

			sub_result->release();
			result->release();
			delete sub_result;
			delete result;

			result = new_result;
		}
	}

	return result;
}

void GeneralEvaluation::getFinalResult(ResultSet &ret_result)
{
	if (this->temp_result == NULL)
		return;

	if (this->query_tree.getQueryForm() == QueryTree::Select_Query)
	{
		Varset useful = this->query_tree.getResultProjectionVarset() + this->query_tree.getGroupByVarset();

		if (this->query_tree.checkProjectionAsterisk())
		{
			for (int i = 0 ; i < (int)this->temp_result->results.size(); i++)
				useful += this->temp_result->results[i].getAllVarset();
		}

		{
			TempResultSet *new_temp_result = new TempResultSet();

			this->temp_result->doProjection1(useful, *new_temp_result, this->stringindex, this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset);

			this->temp_result->release();
			delete this->temp_result;

			this->temp_result = new_temp_result;
		}

		if (this->query_tree.checkAtLeastOneAggregateFunction() || !this->query_tree.getGroupByVarset().empty())
		{
			vector<QueryTree::ProjectionVar> &proj = this->query_tree.getProjection();

			TempResultSet *new_temp_result = new TempResultSet();
			new_temp_result->results.push_back(TempResult());

			TempResult &result0 = this->temp_result->results[0];
			TempResult &new_result0 = new_temp_result->results[0];

			for (int i = 0; i < (int)proj.size(); i++)
				if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
				{
					if (result0.id_varset.findVar(proj[i].var))
						new_result0.id_varset.addVar(proj[i].var);
					else if (result0.str_varset.findVar(proj[i].var))
						new_result0.str_varset.addVar(proj[i].var);
				}
				else
					new_result0.str_varset.addVar(proj[i].var);

			vector<int> proj2temp((int)proj.size(), -1);
			for (int i = 0; i < (int)proj.size(); i++)
				if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
					proj2temp[i] = Varset(proj[i].var).mapTo(result0.getAllVarset())[0];
				else if (proj[i].aggregate_var != "*")
					proj2temp[i] = Varset(proj[i].aggregate_var).mapTo(result0.getAllVarset())[0];

			vector<int> proj2new = this->query_tree.getProjectionVarset().mapTo(new_result0.getAllVarset());

			int result0_size = (int)result0.result.size();
			vector<int> group2temp;

			if (!this->query_tree.getGroupByVarset().empty())
			{
				group2temp = this->query_tree.getGroupByVarset().mapTo(result0.getAllVarset());
				result0.sort(0, result0_size - 1, group2temp);
			}

			TempResultSet *temp_result_distinct = NULL;
			vector<int> group2distinct;

			for (int i = 0; i < (int)proj.size(); i++)
				if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type && proj[i].distinct && proj[i].aggregate_var == "*")
				{
					temp_result_distinct = new TempResultSet();

					this->temp_result->doDistinct1(*temp_result_distinct);
					group2distinct = this->query_tree.getGroupByVarset().mapTo(temp_result_distinct->results[0].getAllVarset());
					temp_result_distinct->results[0].sort(0, (int)temp_result_distinct->results[0].result.size() - 1, group2distinct);

					break;
				}

			int result0_id_cols = result0.id_varset.getVarsetSize();
			int new_result0_id_cols = new_result0.id_varset.getVarsetSize();
			int new_result0_str_cols = new_result0.str_varset.getVarsetSize();
			for (int begin = 0; begin < result0_size;)
			{
				int end;
				if (group2temp.empty())
					end = result0_size - 1;
				else
					end = result0.findRightBounder(group2temp, result0.result[begin], result0_id_cols, group2temp);

				new_result0.result.push_back(TempResult::ResultPair());
				new_result0.result.back().id = new unsigned [new_result0_id_cols];
				new_result0.result.back().str.resize(new_result0_str_cols);

				for (int i = 0; i < new_result0_id_cols; i++)
					new_result0.result.back().id[i] = INVALID;

				for (int i = 0; i < (int)proj.size(); i++)
					if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
					{
						if (proj2temp[i] < result0_id_cols)
							new_result0.result.back().id[proj2new[i]] = result0.result[begin].id[proj2temp[i]];
						else
							new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = result0.result[begin].str[proj2temp[i] - result0_id_cols];
					}
					else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type)
					{
						int count = 0;

						if (proj[i].aggregate_var != "*")
						{
							if (proj[i].distinct)
							{
								if (proj2temp[i] < result0_id_cols)
								{
									set<int> count_set;
									for (int j = begin; j <= end; j++)
										if(result0.result[j].id[proj2temp[i]] != INVALID)
											count_set.insert(result0.result[j].id[proj2temp[i]]);
									count = (int)count_set.size();
								}
								else
								{
									set<string> count_set;
									for (int j = begin; j <= end; j++)
										if (result0.result[j].str[proj2temp[i] - result0_id_cols].length() > 0)
											count_set.insert(result0.result[j].str[proj2temp[i] - result0_id_cols]);
									count = (int)count_set.size();
								}
							}
							else
							{
								if (proj2temp[i] < result0_id_cols)
								{
									for (int j = begin; j <= end; j++)
										if(result0.result[j].id[proj2temp[i]] != INVALID)
											count++;
								}
								else
								{
									for (int j = begin; j <= end; j++)
										if (result0.result[j].str[proj2temp[i] - result0_id_cols].length() > 0)
											count++;
								}
							}
						}
						else
						{
							if (proj[i].distinct)
							{
								count = temp_result_distinct->results[0].findRightBounder(group2distinct, result0.result[begin], result0_id_cols, group2temp) -
										temp_result_distinct->results[0].findLeftBounder(group2distinct, result0.result[begin], result0_id_cols, group2temp) + 1;
							}
							else
							{
								count = end - begin + 1;
							}
						}

						stringstream ss;
						ss << "\"";
						ss << count;
						ss << "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
						ss >> new_result0.result.back().str[proj2new[i] - new_result0_id_cols];
					}

				begin = end + 1;
			}

			if (temp_result_distinct != NULL)
			{
				temp_result_distinct->release();
				delete temp_result_distinct;
			}

			this->temp_result->release();
			delete this->temp_result;

			this->temp_result = new_temp_result;
		}

		//temp_result --> ret_result

		if (this->query_tree.getProjectionModifier() == QueryTree::Modifier_Distinct)
		{
			TempResultSet *new_temp_result = new TempResultSet();

			this->temp_result->doDistinct1(*new_temp_result);

			this->temp_result->release();
			delete this->temp_result;

			this->temp_result = new_temp_result;
		}

		TempResult &result0 = this->temp_result->results[0];
		Varset ret_result_varset;

		if (this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjectionVarset().empty())
		{
			ret_result.select_var_num = result0.getAllVarset().getVarsetSize();
			ret_result.setVar(result0.getAllVarset().vars);
			ret_result_varset = result0.getAllVarset();
		}
		else
		{
			ret_result.select_var_num = this->query_tree.getProjectionVarset().getVarsetSize();
			ret_result.setVar(this->query_tree.getProjectionVarset().vars);
			ret_result_varset = this->query_tree.getProjectionVarset();
		}

		ret_result.ansNum = (int)result0.result.size();
		ret_result.setOutputOffsetLimit(this->query_tree.getOffset(), this->query_tree.getLimit());

#ifdef STREAM_ON
		long long ret_result_size = (long long)ret_result.ansNum * (long long)ret_result.select_var_num * 100 / Util::GB;
    	if(Util::memoryLeft() < ret_result_size || !this->query_tree.getOrderVarVector().empty())
		{
			ret_result.setUseStream();
			printf("set use Stream\n");
		}
#endif

		if (!ret_result.checkUseStream())
		{
			ret_result.answer = new string* [ret_result.ansNum];
			for (unsigned i = 0; i < ret_result.ansNum; i++)
				ret_result.answer[i] = NULL;
		}
		else
		{
			vector <unsigned> keys;
			vector <bool> desc;
			for (int i = 0; i < (int)this->query_tree.getOrderVarVector().size(); i++)
			{
				int var_id = Varset(this->query_tree.getOrderVarVector()[i].var).mapTo(ret_result_varset)[0];
				if (var_id != -1)
				{
					keys.push_back(var_id);
					desc.push_back(this->query_tree.getOrderVarVector()[i].descending);
				}
			}
			ret_result.openStream(keys, desc);
		}

		vector<int> proj2temp = ret_result_varset.mapTo(result0.getAllVarset());
		int id_cols = result0.id_varset.getVarsetSize();

		for (unsigned i = 0; i < ret_result.ansNum; i++)
		{
			if (!ret_result.checkUseStream())
			{
				ret_result.answer[i] = new string [ret_result.select_var_num];
			}

			for (int j = 0; j < ret_result.select_var_num; j++)
			{
				if (proj2temp[j] < id_cols)
				{
					unsigned ans_id = result0.result[i].id[proj2temp[j]];

					if (!ret_result.checkUseStream())
					{
						ret_result.answer[i][j] = "";
						if (ans_id != INVALID)
						{
							if (this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset.findVar(result0.id_varset.vars[proj2temp[j]]))
								this->stringindex->addRequest(ans_id, &ret_result.answer[i][j], true);
							else
								this->stringindex->addRequest(ans_id, &ret_result.answer[i][j], false);
						}
					}
					else
					{
						string ans_str = "";
						if (ans_id != INVALID)
						{
							if (this->query_tree.getGroupPattern().grouppattern_subject_object_maximal_varset.findVar(result0.id_varset.vars[proj2temp[j]]))
								this->stringindex->randomAccess(ans_id, &ans_str, true);
							else
								this->stringindex->randomAccess(ans_id, &ans_str, false);
						}
						ret_result.writeToStream(ans_str);
					}
				}
				else
				{
					if (!ret_result.checkUseStream())
						ret_result.answer[i][j] = result0.result[i].str[proj2temp[j] - id_cols];
					else
						ret_result.writeToStream(result0.result[i].str[proj2temp[j] - id_cols]);
				}
			}
		}
		if (!ret_result.checkUseStream())
		{
			this->stringindex->trySequenceAccess();
		}
		else
		{
			ret_result.resetStream();
		}
	}
	else if (this->query_tree.getQueryForm() == QueryTree::Ask_Query)
	{
		ret_result.select_var_num = 1;
		ret_result.setVar(vector<string>(1, "?__ask_retval"));
		ret_result.ansNum = 1;

		if (!ret_result.checkUseStream())
		{
			ret_result.answer = new string* [ret_result.ansNum];
			ret_result.answer[0] = new string[ret_result.select_var_num];

			ret_result.answer[0][0] = "false";
			for (int i = 0; i < (int)this->temp_result->results.size(); i++)
				if (!this->temp_result->results[i].result.empty())
					ret_result.answer[0][0] = "true";
		}
	}

	this->releaseResult();
}

void GeneralEvaluation::releaseResult()
{
	if (this->temp_result == NULL)
		return;

	this->temp_result->release();
	delete this->temp_result;
	this->temp_result = NULL;
}

void GeneralEvaluation::prepareUpdateTriple(QueryTree::GroupPattern &update_pattern, TripleWithObjType *&update_triple, unsigned &update_triple_num)
{
	update_pattern.getVarset();

	if (update_triple != NULL)
	{
		delete[] update_triple;
		update_triple = NULL;
	}

	if (this->temp_result == NULL)
		return;

	update_triple_num = 0;

	for (int i = 0; i < (int)update_pattern.sub_grouppattern.size(); i++)
		if (update_pattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
		{
			for (int j = 0 ; j < (int)this->temp_result->results.size(); j++)
				if (update_pattern.sub_grouppattern[i].pattern.varset.belongTo(this->temp_result->results[j].getAllVarset()))
					update_triple_num += this->temp_result->results[j].result.size();
		}

	update_triple  = new TripleWithObjType[update_triple_num];

	int update_triple_count = 0;
	for (int i = 0; i < (int)update_pattern.sub_grouppattern.size(); i++)
		if (update_pattern.sub_grouppattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
		{
			for (int j = 0 ; j < (int)this->temp_result->results.size(); j++)
			{
				int id_cols = this->temp_result->results[j].id_varset.getVarsetSize();

				if (update_pattern.sub_grouppattern[i].pattern.varset.belongTo(this->temp_result->results[j].getAllVarset()))
				{
					int subject_id = -1, predicate_id = -1, object_id = -1;
					if (update_pattern.sub_grouppattern[i].pattern.subject.value[0] == '?')
						subject_id = Varset(update_pattern.sub_grouppattern[i].pattern.subject.value).mapTo(this->temp_result->results[j].getAllVarset())[0];
					if (update_pattern.sub_grouppattern[i].pattern.predicate.value[0] == '?')
						predicate_id = Varset(update_pattern.sub_grouppattern[i].pattern.predicate.value).mapTo(this->temp_result->results[j].getAllVarset())[0];
					if (update_pattern.sub_grouppattern[i].pattern.object.value[0] == '?')
						object_id = Varset(update_pattern.sub_grouppattern[i].pattern.object.value).mapTo(this->temp_result->results[j].getAllVarset())[0];

					string subject, predicate, object;
					TripleWithObjType::ObjectType object_type;

					if (subject_id == -1)
						subject = update_pattern.sub_grouppattern[i].pattern.subject.value;
					if (predicate_id == -1)
						predicate = update_pattern.sub_grouppattern[i].pattern.predicate.value;
					if (object_id == -1)
						object = update_pattern.sub_grouppattern[i].pattern.object.value;

					for (int k = 0; k < (int)this->temp_result->results[j].result.size(); k++)
					{
						if (subject_id != -1)
						{
							if (subject_id < id_cols)
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[subject_id], &subject, true);
							else
								subject = this->temp_result->results[j].result[k].str[subject_id - id_cols];
						}

						if (predicate_id != -1)
						{
							if (predicate_id < id_cols)
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[predicate_id], &predicate, false);
							else
								predicate = this->temp_result->results[j].result[k].str[predicate_id - id_cols];
						}

						if (object_id != -1)
						{
							if (object_id < id_cols)
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[object_id], &object, true);
							else
								object = this->temp_result->results[j].result[k].str[object_id - id_cols];
						}

						if (object[0] == '<')
							object_type = TripleWithObjType::Entity;
						else
							object_type = TripleWithObjType::Literal;

						update_triple[update_triple_count++] = TripleWithObjType(subject, predicate, object, object_type);
					}
				}
			}
		}
}
