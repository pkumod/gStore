/*=============================================================================
# Filename: TempResult.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-08-14
# Description: implement functions in TempResult.h
=============================================================================*/

#include "TempResult.h"

using namespace std;

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator ! ()
{
	if (this->value == true_value)	return false_value;
	if (this->value == false_value)	return true_value;
	if (this->value == error_value)	return error_value;

	return error_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator || (const EffectiveBooleanValue &x)
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

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator && (const EffectiveBooleanValue &x)
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

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator == (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator != (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator < (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator <= (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator > (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::EffectiveBooleanValue::operator >= (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::DateTime::operator == (const DateTime &x)
{
	if (this->date == x.date)	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::DateTime::operator != (const DateTime &x)
{
	if (this->date != x.date)	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::DateTime::operator < (const DateTime &x)
{
	if (this->date < x.date)	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::DateTime::operator <= (const DateTime &x)
{
	if (this->date <= x.date)	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::DateTime::operator > (const DateTime &x)
{
	if (this->date > x.date)	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}

FilterEvaluationMultitypeValue::EffectiveBooleanValue
	FilterEvaluationMultitypeValue::DateTime::operator >= (const DateTime &x)
{
	if (this->date >= x.date)	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
	else	return FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
}


bool FilterEvaluationMultitypeValue::isSimpleLiteral()
{
	if (this->datatype == literal)
	{
		int length = this->str_value.length();
		if (length >= 2 && this->str_value[0] == '"' && this->str_value[length - 1] == '"')
			return true;
	}
	return false;
}

void FilterEvaluationMultitypeValue::getSameNumericType (FilterEvaluationMultitypeValue &x)
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

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator !()
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = !this->bool_value;
	return ret_femv;
}

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator || (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean && x.datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = (this->bool_value || x.bool_value);
	return ret_femv;
}

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator && (FilterEvaluationMultitypeValue &x)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean && x.datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = (this->bool_value && x.bool_value);
	return ret_femv;
}

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator == (FilterEvaluationMultitypeValue &x)
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

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator != (FilterEvaluationMultitypeValue &x)
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

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator < (FilterEvaluationMultitypeValue &x)
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

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator <= (FilterEvaluationMultitypeValue &x)
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

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator > (FilterEvaluationMultitypeValue &x)
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

FilterEvaluationMultitypeValue
	FilterEvaluationMultitypeValue::operator >= (FilterEvaluationMultitypeValue &x)
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

Varset TempResult::getAllVarset()
{
	return this->id_varset + this->str_varset;
}

void TempResult::release()
{
	for (int i = 0; i < (int)this->result.size(); i++)
	{
		delete[] result[i].id;
		vector<string>().swap(result[i].str);
	}
}

int TempResult::compareRow(const ResultPair &x, const int x_id_cols, const vector<int> &x_pos,
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

void TempResult::sort(int l, int r, const vector<int> &this_pos)
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

int TempResult::findLeftBounder(const vector<int> &this_pos, const ResultPair &x, const int x_id_cols, const vector<int> &x_pos) const
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

int TempResult::findRightBounder(const vector<int> &this_pos, const ResultPair &x, const int x_id_cols, const vector<int> &x_pos) const
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

void TempResult::convertId2Str(Varset convert_varset, StringIndex *stringindex, Varset &entity_literal_varset)
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

void TempResult::doJoin(TempResult &x, TempResult &r)
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

void TempResult::doUnion(TempResult &r)
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

void TempResult::doOptional(vector<bool> &binding, TempResult &x, TempResult &rn, TempResult &ra, bool add_no_binding)
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

void TempResult::doMinus(TempResult &x, TempResult &r)
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

void TempResult::getFilterString(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild &child, FilterEvaluationMultitypeValue &femv, ResultPair &row, int id_cols, StringIndex *stringindex)
{
	if (child.node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
	{
		femv.datatype = FilterEvaluationMultitypeValue::rdf_term;

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
			femv.datatype = FilterEvaluationMultitypeValue::iri;
			femv.str_value = femv.term_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("\"^^<") == string::npos && femv.term_value[femv.term_value.length() - 1] != '>' )
		{
			femv.datatype = FilterEvaluationMultitypeValue::literal;
			femv.str_value = femv.term_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#string>") != string::npos)
		{
			femv.datatype = FilterEvaluationMultitypeValue::xsd_string;
			femv.str_value = femv.term_value.substr(0, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#string>"));
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#boolean>") != string::npos)
		{
			femv.datatype = FilterEvaluationMultitypeValue::xsd_boolean;

			string value = femv.term_value.substr(0, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#boolean>"));
			if (value == "\"true\"")
				femv.bool_value = FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value;
			else if (value == "\"false\"")
				femv.bool_value = FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
			else
				femv.bool_value = FilterEvaluationMultitypeValue::EffectiveBooleanValue::error_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#integer>") != string::npos)
		{
			femv.datatype = FilterEvaluationMultitypeValue::xsd_integer;

			string value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#integer>") - 2);
			stringstream ss;
			ss << value;
			ss >> femv.int_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#decimal>") != string::npos)
		{
			femv.datatype = FilterEvaluationMultitypeValue::xsd_decimal;

			string value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#decimal>") - 2);
			stringstream ss;
			ss << value;
			ss >> femv.flt_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#float>") != string::npos)
		{
			femv.datatype = FilterEvaluationMultitypeValue::xsd_float;

			string value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#float>") - 2);
			stringstream ss;
			ss << value;
			ss >> femv.flt_value;
		}

		if (femv.term_value[0] == '"' && femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#double>") != string::npos)
		{
			femv.datatype = FilterEvaluationMultitypeValue::xsd_double;

			string value = femv.term_value.substr(1, femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#double>") - 2);
			stringstream ss;
			ss << value;
			ss >> femv.dbl_value;
		}

		if (femv.term_value[0] == '"' &&
				(femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#dateTime>") != string::npos || femv.term_value.find("^^<http://www.w3.org/2001/XMLSchema#date>") != string::npos))
		{
			femv.datatype = FilterEvaluationMultitypeValue::xsd_datetime;

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

			femv.dt_value = FilterEvaluationMultitypeValue::DateTime(date[0], date[1], date[2], date[3], date[4], date[5]);
		}
	}
}

FilterEvaluationMultitypeValue
	TempResult::matchFilterTree(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, ResultPair &row, int id_cols, StringIndex *stringindex)
{
	FilterEvaluationMultitypeValue ret_femv;
	ret_femv.datatype = FilterEvaluationMultitypeValue::xsd_boolean;
	ret_femv.bool_value = FilterEvaluationMultitypeValue::EffectiveBooleanValue::error_value;

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
		if (x.datatype == FilterEvaluationMultitypeValue::literal || x.datatype == FilterEvaluationMultitypeValue::xsd_string)
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
			ret_femv.bool_value = FilterEvaluationMultitypeValue::EffectiveBooleanValue::false_value;
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

		if (x.datatype == FilterEvaluationMultitypeValue::literal)
		{
			ret_femv.datatype = FilterEvaluationMultitypeValue::literal;

			ret_femv.str_value = x.str_value.substr(0, x.str_value.rfind('"') + 1);

			return ret_femv;
		}
		else if (x.datatype == FilterEvaluationMultitypeValue::iri)
		{
			ret_femv.datatype = FilterEvaluationMultitypeValue::literal;

			ret_femv.str_value = "\"" + x.str_value.substr(1, x.str_value.length() - 2) + "\"";

			return ret_femv;
		}
		else if (x.datatype == FilterEvaluationMultitypeValue::xsd_string)
		{
			ret_femv.datatype = FilterEvaluationMultitypeValue::literal;

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

		ret_femv.bool_value = (x.datatype == FilterEvaluationMultitypeValue::iri);

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isliteral_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		ret_femv.bool_value = (x.datatype == FilterEvaluationMultitypeValue::literal ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_string ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_boolean ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_integer ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_decimal ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_float ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_double ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_datetime);

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isnumeric_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		ret_femv.bool_value = (x.datatype == FilterEvaluationMultitypeValue::xsd_integer ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_decimal ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_float ||
							   x.datatype == FilterEvaluationMultitypeValue::xsd_double);

		return ret_femv;
	}

	if (filter.oper_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_lang_type)
	{
		FilterEvaluationMultitypeValue x;

		if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type)
			getFilterString(filter.child[0], x, row, id_cols, stringindex);
		else if (filter.child[0].node_type == QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type)
			x = matchFilterTree(filter.child[0].node, row, id_cols, stringindex);

		if (x.datatype == FilterEvaluationMultitypeValue::literal)
		{
			ret_femv.datatype = FilterEvaluationMultitypeValue::literal;

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

			if (ret_femv.datatype == FilterEvaluationMultitypeValue::xsd_boolean && ret_femv.bool_value.value == FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value)
				return ret_femv;
		}

		return ret_femv;
	}

	return ret_femv;
}

void TempResult::doFilter(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, TempResult &r, StringIndex *stringindex, Varset &entity_literal_varset)
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
		FilterEvaluationMultitypeValue ret_femv = matchFilterTree(filter, this->result[i], this_id_cols, stringindex);

		if (ret_femv.datatype == FilterEvaluationMultitypeValue::xsd_boolean && ret_femv.bool_value.value == FilterEvaluationMultitypeValue::EffectiveBooleanValue::true_value)
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

void TempResult::print(int no)
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

void TempResultSet::release()
{
	for (int i = 0; i < (int)this->results.size(); i++)
		this->results[i].release();
}

int TempResultSet::findCompatibleResult(Varset &_id_varset, Varset &_str_varset)
{
	for (int i = 0; i < (int)this->results.size(); i++)
		if (this->results[i].id_varset == _id_varset && this->results[i].str_varset == _str_varset)
			return i;

	this->results.push_back(TempResult());
	this->results.back().id_varset = _id_varset;
	this->results.back().str_varset = _str_varset;

	return (int)this->results.size() - 1;
}

void TempResultSet::doJoin(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
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

void TempResultSet::doUnion(TempResultSet &x, TempResultSet &r)
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

void TempResultSet::doOptional(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
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

void TempResultSet::doMinus(TempResultSet &x, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
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

void TempResultSet::doFilter(QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
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

void TempResultSet::doProjection1(Varset &proj, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset)
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

void TempResultSet::doDistinct1(TempResultSet &r)
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

void TempResultSet::print()
{
	printf("total temp result: %d\n", (int)this->results.size());
	for (int i = 0; i < (int)this->results.size(); i++)
		this->results[i].print(i);
}
