#include "EvalMultitypeValue.h"

using namespace std;

int EvalMultitypeValue::EffectiveBooleanValue::getValue()
{
	if (value == false_value)
		return 0;
	else if (value == true_value)
		return 1;
	else	// error_value
		return 2;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator ! ()
{
	if (this->value == true_value)	return false_value;
	if (this->value == false_value)	return true_value;
	if (this->value == error_value)	return error_value;

	return error_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator || (const EffectiveBooleanValue &x)
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

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator && (const EffectiveBooleanValue &x)
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

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator == (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator != (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator < (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator <= (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return false_value;
	if (this->value == false_value && x.value == true_value)	return true_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator > (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return false_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return false_value;

	return error_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::EffectiveBooleanValue::operator >= (const EffectiveBooleanValue &x)
{
	if (this->value == true_value && x.value == true_value)		return true_value;
	if (this->value == true_value && x.value == false_value)	return true_value;
	if (this->value == false_value && x.value == true_value)	return false_value;
	if (this->value == false_value && x.value == false_value)	return true_value;

	return error_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::DateTime::operator == (const DateTime &x)
{
	if (this->date == x.date)	return EvalMultitypeValue::EffectiveBooleanValue::true_value;
	else	return EvalMultitypeValue::EffectiveBooleanValue::false_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::DateTime::operator != (const DateTime &x)
{
	if (this->date != x.date)	return EvalMultitypeValue::EffectiveBooleanValue::true_value;
	else	return EvalMultitypeValue::EffectiveBooleanValue::false_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::DateTime::operator < (const DateTime &x)
{
	if (this->date < x.date)	return EvalMultitypeValue::EffectiveBooleanValue::true_value;
	else	return EvalMultitypeValue::EffectiveBooleanValue::false_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::DateTime::operator <= (const DateTime &x)
{
	if (this->date <= x.date)	return EvalMultitypeValue::EffectiveBooleanValue::true_value;
	else	return EvalMultitypeValue::EffectiveBooleanValue::false_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::DateTime::operator > (const DateTime &x)
{
	if (this->date > x.date)	return EvalMultitypeValue::EffectiveBooleanValue::true_value;
	else	return EvalMultitypeValue::EffectiveBooleanValue::false_value;
}

EvalMultitypeValue::EffectiveBooleanValue
	EvalMultitypeValue::DateTime::operator >= (const DateTime &x)
{
	if (this->date >= x.date)	return EvalMultitypeValue::EffectiveBooleanValue::true_value;
	else	return EvalMultitypeValue::EffectiveBooleanValue::false_value;
}


bool EvalMultitypeValue::isSimpleLiteral()
{
	if (this->datatype == literal)
	{
		// cout << str_value << endl;
		int length = this->str_value.length();
		if (length >= 2 && this->str_value[0] == '"' && this->str_value[length - 1] == '"')
		{
			// cout << "correct!" << endl;
			return true;
		}
		// else
		// {
		// 	cout << "wrong!" << endl;
		// 	cout << "length: " << length << endl;
		// 	for (int i = 0; i < length; i++)
		// 		cout << "[" << i << "]: " << int(str_value[i]) << endl;
			
		// }
	}
	return false;
}

void EvalMultitypeValue::getSameNumericType (EvalMultitypeValue &x)
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

EvalMultitypeValue
	EvalMultitypeValue::operator !()
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = !this->bool_value;

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator || (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean && x.datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = (this->bool_value || x.bool_value);

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator && (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype != xsd_boolean && x.datatype != xsd_boolean)
		return ret_femv;

	ret_femv.bool_value = (this->bool_value && x.bool_value);

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator == (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value == x.bool_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if ((this->datatype == xsd_boolean) ^ (x.datatype == xsd_boolean))
	{
		if (this->datatype == xsd_boolean && this->bool_value.value == EffectiveBooleanValue::error_value)
			return ret_femv;
		if (x.datatype == xsd_boolean && x.bool_value.value == EffectiveBooleanValue::error_value)
			return ret_femv;

		ret_femv.bool_value = EffectiveBooleanValue::false_value;

		ret_femv.deduceTermValue();
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

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value == x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value == x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value == x.dt_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	ret_femv.bool_value = (this->term_value == x.term_value);

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator != (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value != x.bool_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if ((this->datatype == xsd_boolean) ^ (x.datatype == xsd_boolean))
	{
		if (this->datatype == xsd_boolean && this->bool_value.value == EffectiveBooleanValue::error_value)
			return ret_femv;
		if (x.datatype == xsd_boolean && x.bool_value.value == EffectiveBooleanValue::error_value)
			return ret_femv;

		ret_femv.bool_value = EffectiveBooleanValue::true_value;

		ret_femv.deduceTermValue();
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

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value != x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value != x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value != x.dt_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	ret_femv.bool_value = (this->term_value != x.term_value);

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator < (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value < x.bool_value);

		ret_femv.deduceTermValue();
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

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value < x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value < x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value < x.dt_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator <= (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value <= x.bool_value);

		ret_femv.deduceTermValue();
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

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value <= x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value <= x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value <= x.dt_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator > (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value > x.bool_value);

		ret_femv.deduceTermValue();
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

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value > x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value > x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value > x.dt_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator >= (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (this->datatype == xsd_boolean && x.datatype == xsd_boolean)
	{
		ret_femv.bool_value = (this->bool_value >= x.bool_value);

		ret_femv.deduceTermValue();
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

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->isSimpleLiteral() && x.isSimpleLiteral())
	{
		ret_femv.bool_value = (this->str_value >= x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_string && x.datatype == xsd_string)
	{
		ret_femv.bool_value = (this->str_value >= x.str_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	if (this->datatype == xsd_datetime && x.datatype == xsd_datetime)
	{
		ret_femv.bool_value = (this->dt_value >= x.dt_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator + (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if ((datatype != xsd_integer && datatype != xsd_decimal && datatype != xsd_float && datatype != xsd_double) \
			|| (x.datatype != xsd_integer && x.datatype != xsd_decimal && x.datatype != xsd_float && x.datatype != xsd_double))
		return ret_femv;

	this->getSameNumericType(x);
	if (datatype == xsd_integer)
	{
		ret_femv.datatype = xsd_integer;
		ret_femv.int_value = int_value + x.int_value;
	}
	else if (datatype == xsd_decimal)
	{
		ret_femv.datatype = xsd_decimal;
		ret_femv.flt_value = flt_value + x.flt_value;
	}
	else if (datatype == xsd_float)
	{
		ret_femv.datatype = xsd_float;
		ret_femv.flt_value = flt_value + x.flt_value;
	}
	else if (datatype == xsd_double)
	{
		ret_femv.datatype = xsd_double;
		ret_femv.dbl_value = dbl_value + x.dbl_value;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator - ()
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if (datatype != xsd_integer && datatype != xsd_decimal && datatype != xsd_float && datatype != xsd_double)
		return ret_femv;

	if (datatype == xsd_integer)
	{
		ret_femv.datatype = xsd_integer;
		ret_femv.int_value = -int_value;
	}
	else if (datatype == xsd_decimal)
	{
		ret_femv.datatype = xsd_decimal;
		ret_femv.flt_value = -flt_value;
	}
	else if (datatype == xsd_float)
	{
		ret_femv.datatype = xsd_float;
		ret_femv.flt_value = -flt_value;
	}
	else if (datatype == xsd_double)
	{
		ret_femv.datatype = xsd_double;
		ret_femv.dbl_value = -dbl_value;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator - (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if ((datatype != xsd_integer && datatype != xsd_decimal && datatype != xsd_float && datatype != xsd_double) \
			|| (x.datatype != xsd_integer && x.datatype != xsd_decimal && x.datatype != xsd_float && x.datatype != xsd_double))
		return ret_femv;

	this->getSameNumericType(x);
	if (datatype == xsd_integer)
	{
		ret_femv.datatype = xsd_integer;
		ret_femv.int_value = int_value - x.int_value;
	}
	else if (datatype == xsd_decimal)
	{
		ret_femv.datatype = xsd_decimal;
		ret_femv.flt_value = flt_value - x.flt_value;
	}
	else if (datatype == xsd_float)
	{
		ret_femv.datatype = xsd_float;
		ret_femv.flt_value = flt_value - x.flt_value;
	}
	else if (datatype == xsd_double)
	{
		ret_femv.datatype = xsd_double;
		ret_femv.dbl_value = dbl_value - x.dbl_value;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator * (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if ((datatype != xsd_integer && datatype != xsd_decimal && datatype != xsd_float && datatype != xsd_double) \
			|| (x.datatype != xsd_integer && x.datatype != xsd_decimal && x.datatype != xsd_float && x.datatype != xsd_double))
		return ret_femv;

	this->getSameNumericType(x);
	if (datatype == xsd_integer)
	{
		ret_femv.datatype = xsd_integer;
		ret_femv.int_value = int_value * x.int_value;
	}
	else if (datatype == xsd_decimal)
	{
		ret_femv.datatype = xsd_decimal;
		ret_femv.flt_value = flt_value * x.flt_value;
	}
	else if (datatype == xsd_float)
	{
		ret_femv.datatype = xsd_float;
		ret_femv.flt_value = flt_value * x.flt_value;
	}
	else if (datatype == xsd_double)
	{
		ret_femv.datatype = xsd_double;
		ret_femv.dbl_value = dbl_value * x.dbl_value;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

EvalMultitypeValue
	EvalMultitypeValue::operator / (EvalMultitypeValue &x)
{
	EvalMultitypeValue ret_femv;
	ret_femv.datatype = xsd_boolean;
	ret_femv.bool_value = EffectiveBooleanValue::error_value;

	if ((datatype != xsd_integer && datatype != xsd_decimal && datatype != xsd_float && datatype != xsd_double) \
			|| (x.datatype != xsd_integer && x.datatype != xsd_decimal && x.datatype != xsd_float && x.datatype != xsd_double))
		return ret_femv;

	this->getSameNumericType(x);
	if (datatype == xsd_integer)
	{
		ret_femv.datatype = xsd_integer;
		ret_femv.int_value = int_value / x.int_value;
	}
	else if (datatype == xsd_decimal)
	{
		ret_femv.datatype = xsd_decimal;
		ret_femv.flt_value = flt_value / x.flt_value;
	}
	else if (datatype == xsd_float)
	{
		ret_femv.datatype = xsd_float;
		ret_femv.flt_value = flt_value / x.flt_value;
	}
	else if (datatype == xsd_double)
	{
		ret_femv.datatype = xsd_double;
		ret_femv.dbl_value = dbl_value / x.dbl_value;
	}

	ret_femv.deduceTermValue();
	return ret_femv;
}

void EvalMultitypeValue::deduceTermValue()
{
	if (datatype == xsd_integer)
		term_value = "\"" + to_string(int_value) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
	else if (datatype == xsd_decimal)
		term_value = "\"" + to_string(flt_value) + "\"^^<http://www.w3.org/2001/XMLSchema#decimal>";
	else if (datatype == xsd_float)
		term_value = "\"" + to_string(flt_value) + "\"^^<http://www.w3.org/2001/XMLSchema#float>";
	else if (datatype == xsd_double)
		term_value = "\"" + to_string(dbl_value) + "\"^^<http://www.w3.org/2001/XMLSchema#double>";
	else if (datatype == xsd_boolean)
	{
		if (bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
			term_value = "\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
		else if (bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::false_value)
			term_value = "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
	}
}

void EvalMultitypeValue::deduceTypeValue()
{
	if (term_value[0] == '<' && term_value[term_value.length() - 1] == '>')
	{
		datatype = EvalMultitypeValue::iri;
		str_value = term_value;
	}

	if (term_value[0] == '"' && term_value.find("\"^^<") == string::npos \
		&& term_value[term_value.length() - 1] != '>')
	{
		datatype = EvalMultitypeValue::literal;
		str_value = term_value;

		// Strip extraneous characters after last " for simple literals
		int i = str_value.length() - 1;
		while (str_value[i] != '"')
			i--;
		if (str_value[i + 1] != '@')	// No language tag
			str_value = str_value.substr(0, i + 1);
	}

	if (term_value[0] == '"' && \
		term_value.find("^^<http://www.w3.org/2001/XMLSchema#string>") != string::npos)
	{
		datatype = EvalMultitypeValue::xsd_string;
		str_value = term_value.substr(0, term_value.find("^^<http://www.w3.org/2001/XMLSchema#string>"));
	}

	if (term_value[0] == '"' && \
		term_value.find("^^<http://www.w3.org/2001/XMLSchema#boolean>") != string::npos)
	{
		datatype = EvalMultitypeValue::xsd_boolean;

		string value = term_value.substr(0, term_value.find("^^<http://www.w3.org/2001/XMLSchema#boolean>"));
		if (value == "\"true\"")
			bool_value = EvalMultitypeValue::EffectiveBooleanValue::true_value;
		else if (value == "\"false\"")
			bool_value = EvalMultitypeValue::EffectiveBooleanValue::false_value;
		else
			bool_value = EvalMultitypeValue::EffectiveBooleanValue::error_value;
	}

	if (term_value[0] == '"' && \
		term_value.find("^^<http://www.w3.org/2001/XMLSchema#integer>") != string::npos)
	{
		datatype = EvalMultitypeValue::xsd_integer;

		string value = term_value.substr(1, term_value.find("^^<http://www.w3.org/2001/XMLSchema#integer>") - 2);
		stringstream ss;
		ss << value;
		ss >> int_value;
	}

	if (term_value[0] == '"' && \
		term_value.find("^^<http://www.w3.org/2001/XMLSchema#decimal>") != string::npos)
	{
		datatype = EvalMultitypeValue::xsd_decimal;

		string value = term_value.substr(1, term_value.find("^^<http://www.w3.org/2001/XMLSchema#decimal>") - 2);
		stringstream ss;
		ss << value;
		ss >> flt_value;
	}

	if (term_value[0] == '"' && \
		term_value.find("^^<http://www.w3.org/2001/XMLSchema#float>") != string::npos)
	{
		datatype = EvalMultitypeValue::xsd_float;

		string value = term_value.substr(1, term_value.find("^^<http://www.w3.org/2001/XMLSchema#float>") - 2);
		stringstream ss;
		ss << value;
		ss >> flt_value;
	}

	if (term_value[0] == '"' && \
		term_value.find("^^<http://www.w3.org/2001/XMLSchema#double>") != string::npos)
	{
		datatype = EvalMultitypeValue::xsd_double;

		string value = term_value.substr(1, term_value.find("^^<http://www.w3.org/2001/XMLSchema#double>") - 2);
		stringstream ss;
		ss << value;
		ss >> dbl_value;
	}

	if (term_value[0] == '"' && \
		(term_value.find("^^<http://www.w3.org/2001/XMLSchema#dateTime>") != string::npos || \
		term_value.find("^^<http://www.w3.org/2001/XMLSchema#date>") != string::npos))
	{
		datatype = EvalMultitypeValue::xsd_datetime;

		string value;
		if (term_value.find("^^<http://www.w3.org/2001/XMLSchema#dateTime>") != string::npos)
			value = term_value.substr(1, term_value.find("^^<http://www.w3.org/2001/XMLSchema#dateTime>") - 2);
		if (term_value.find("^^<http://www.w3.org/2001/XMLSchema#date>") != string::npos)
			value = term_value.substr(1, term_value.find("^^<http://www.w3.org/2001/XMLSchema#date>") - 2);

		vector<int> date;
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

		dt_value = EvalMultitypeValue::DateTime(date[0], date[1], date[2], date[3], date[4], date[5]);
	}
}