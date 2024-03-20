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

void EvalMultitypeValue::convert2Type(EvalMultitypeValue::DataType to_type)
{
	if (this->datatype == xsd_integer && to_type == xsd_decimal)
		this->flt_value = this->int_value;
	else if (this->datatype == xsd_integer && to_type == xsd_float)
		this->flt_value = this->int_value;
	else if (this->datatype == xsd_integer && to_type == xsd_long)
		this->long_value = this->int_value;
	else if (this->datatype == xsd_integer && to_type == xsd_double)
		this->dbl_value = this->int_value;
	else if (this->datatype == xsd_decimal && to_type == xsd_double)
		this->dbl_value = this->flt_value;
	else if (this->datatype == xsd_decimal && to_type == xsd_long)
		this->long_value = this->flt_value;
	else if (this->datatype == xsd_long && to_type == xsd_double)
		this->dbl_value = this->long_value;
	else if (this->datatype == xsd_float && to_type == xsd_double)
		this->dbl_value = this->flt_value;
	else
		return;
	this->datatype = to_type;
}

void EvalMultitypeValue::getSameNumericType (EvalMultitypeValue &x)
{
	DataType to_type = max(this->datatype, x.datatype);
	if (this->datatype != to_type)
		this->convert2Type(to_type);
	if (x.datatype != to_type)
		x.convert2Type(to_type);
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
		else if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value == x.flt_value);
		else if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value == x.flt_value);
		else if (this->datatype == xsd_long && x.datatype == xsd_long)
			ret_femv.bool_value = (this->long_value == x.long_value);
		else if (this->datatype == xsd_double && x.datatype == xsd_double)
			ret_femv.bool_value = (this->dbl_value == x.dbl_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	// if (this->isSimpleLiteral() && x.isSimpleLiteral())
	if (this->datatype == literal && x.datatype == literal)
	{
		// Include those with lang tags; require lang tags to be equal
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
		else if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value != x.flt_value);
		else if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value != x.flt_value);
		else if (this->datatype == xsd_long && x.datatype == xsd_long)
			ret_femv.bool_value = (this->long_value != x.long_value);
		else if (this->datatype == xsd_double && x.datatype == xsd_double)
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
		else if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value < x.flt_value);
		else if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value < x.flt_value);
		else if (this->datatype == xsd_long && x.datatype == xsd_long)
			ret_femv.bool_value = (this->long_value < x.long_value);
		else if (this->datatype == xsd_double && x.datatype == xsd_double)
			ret_femv.bool_value = (this->dbl_value < x.dbl_value);

		ret_femv.deduceTermValue();
		return ret_femv;
	}

	// TODO: isSimpleLiteral seems not robust
	// if (this->isSimpleLiteral() && x.isSimpleLiteral())
	if (this->datatype == literal && x.datatype == literal)
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

	if (this->datatype == iri && x.datatype == iri)
	{
		ret_femv.bool_value = (this->str_value < x.str_value);

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
		else if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value <= x.flt_value);
		else if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value <= x.flt_value);
		else if (this->datatype == xsd_long && x.datatype == xsd_long)
			ret_femv.bool_value = (this->long_value <= x.long_value);
		else if (this->datatype == xsd_double && x.datatype == xsd_double)
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

	if (this->datatype == iri && x.datatype == iri)
	{
		ret_femv.bool_value = (this->str_value <= x.str_value);

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
		else if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value > x.flt_value);
		else if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value > x.flt_value);
		else if (this->datatype == xsd_long && x.datatype == xsd_long)
			ret_femv.bool_value = (this->long_value > x.long_value);
		else if (this->datatype == xsd_double && x.datatype == xsd_double)
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

	if (this->datatype == iri && x.datatype == iri)
	{
		ret_femv.bool_value = (this->str_value > x.str_value);

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
		else if (this->datatype == xsd_decimal && x.datatype == xsd_decimal)
			ret_femv.bool_value = (this->flt_value >= x.flt_value);
		else if (this->datatype == xsd_float && x.datatype == xsd_float)
			ret_femv.bool_value = (this->flt_value >= x.flt_value);
		else if (this->datatype == xsd_long && x.datatype == xsd_long)
			ret_femv.bool_value = (this->long_value >= x.long_value);
		else if (this->datatype == xsd_double && x.datatype == xsd_double)
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

	if (this->datatype == iri && x.datatype == iri)
	{
		ret_femv.bool_value = (this->str_value >= x.str_value);

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
	else if (datatype == xsd_long)
	{
		ret_femv.datatype = xsd_long;
		ret_femv.flt_value = long_value + x.long_value;
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
	else if (datatype == xsd_long)
	{
		ret_femv.datatype = xsd_long;
		ret_femv.long_value = -long_value;
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

	if (datatype == xsd_datetime && x.datatype == xsd_datetime) {
		std::tm t1, t2;
		t1.tm_year = dt_value.date[0] - 1900;
		t1.tm_mon = dt_value.date[1] - 1;
		t1.tm_mday = dt_value.date[2];
		t1.tm_hour = dt_value.date[3];
		t1.tm_min = dt_value.date[4];
		t1.tm_sec = dt_value.date[5];
		t2.tm_year = x.dt_value.date[0] - 1900;
		t2.tm_mon = x.dt_value.date[1] - 1;
		t2.tm_mday = x.dt_value.date[2];
		t2.tm_hour = x.dt_value.date[3];
		t2.tm_min = x.dt_value.date[4];
		t2.tm_sec = x.dt_value.date[5];
		std::time_t time1 = std::mktime(&t1);
		std::time_t time2 = std::mktime(&t2);

		std::chrono::system_clock::time_point tp1 = std::chrono::system_clock::from_time_t(time1);
		std::chrono::system_clock::time_point tp2 = std::chrono::system_clock::from_time_t(time2);

		// Compute the duration between the two time points
		std::chrono::duration<double> duration = tp1 - tp2;

		// Extract the individual components
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration) % 60;
		auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration) % 60;
		auto hours = std::chrono::duration_cast<std::chrono::hours>(duration) % 24;
		auto days = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<86400>>>(duration);

		ret_femv.datatype = xsd_datetime;
		ret_femv.dt_value.date[0] = 0;
		ret_femv.dt_value.date[1] = 0;
		ret_femv.dt_value.date[2] = days.count();
		ret_femv.dt_value.date[3] = hours.count();
		ret_femv.dt_value.date[4] = minutes.count();
		ret_femv.dt_value.date[5] = seconds.count();
		return ret_femv;
	}
	
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
	else if (datatype == xsd_long)
	{
		ret_femv.datatype = xsd_long;
		ret_femv.long_value = long_value - x.long_value;
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
	else if (datatype == xsd_long)
	{
		ret_femv.datatype = xsd_long;
		ret_femv.long_value = long_value * x.long_value;
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
	else if (datatype == xsd_long)
	{
		ret_femv.datatype = xsd_long;
		ret_femv.long_value = long_value / x.long_value;
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
	else if (datatype == xsd_long)
		term_value = "\"" + to_string(long_value) + "\"^^<http://www.w3.org/2001/XMLSchema#long>";
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
	else if (datatype == xsd_datetime)
	{
		stringstream ss;
		// 2002-10-10T12:00:00Z
		ss << '\"' << dt_value.date[0] << '-';
		if (dt_value.date[1] < 10)
			ss << '0';
		ss << dt_value.date[1] << '-';
		if (dt_value.date[2] < 10)
			ss << '0';
		ss << dt_value.date[2] << 'T';
		if (dt_value.date[3] < 10)
			ss << '0';
		ss << dt_value.date[3] << ':';
		if (dt_value.date[4] < 10)
			ss << '0';
		ss << dt_value.date[4] << ':';
		if (dt_value.date[5] < 10)
			ss << '0';
		ss << dt_value.date[5] << "\"^^<http://www.w3.org/2001/XMLSchema#dateTime>";
		ss >> term_value;
	} else
		term_value = str_value;
}

void EvalMultitypeValue::deduceTypeValue()
{
	datatype = EvalMultitypeValue::xsd_boolean;
	bool_value = EvalMultitypeValue::EffectiveBooleanValue::error_value;
	if (term_value.empty())
		return;
	char startC = term_value[0];
	if (startC == '<') {
		datatype = EvalMultitypeValue::iri;
		str_value = term_value;
	} else if (startC == '"') {
		size_t sufPos = term_value.find("\"^^<"), termLen = term_value.size();
		if (sufPos == string::npos) {
			datatype = EvalMultitypeValue::literal;
			str_value = term_value;
			// if (term_value.back() == '"')
			// 	str_value = term_value;
			// else
			// 	str_value = term_value.substr(0, term_value.find('"', 1) + 1);
		} else {
			string sufStr = term_value.substr(sufPos + 4, termLen - sufPos - 5);
			string baseUrl = "http://www.w3.org/2001/XMLSchema#";
			size_t baseLen = baseUrl.size();
			if (sufStr.substr(0, baseLen) != baseUrl) {
				datatype = EvalMultitypeValue::rdf_term;
				return;
			}
			sufStr = sufStr.substr(baseLen);
			string contentStr = term_value.substr(0, sufPos + 1);
			size_t contentLen = contentStr.size();
			if (sufStr == "string") {
				datatype = EvalMultitypeValue::xsd_string;
				str_value = contentStr;
			} else if (sufStr == "boolean") {
				if (contentStr == "\"true\"")
					bool_value = EvalMultitypeValue::EffectiveBooleanValue::true_value;
				else if (contentStr == "\"false\"")
					bool_value = EvalMultitypeValue::EffectiveBooleanValue::false_value;
			} else if (sufStr == "dateTime" || sufStr == "date") {
				datatype = EvalMultitypeValue::xsd_datetime;
				vector<float> date;
				size_t p = 0, nextP = 0;
				bool decimal = false;
				for (int i = 0; i < 6; i++) {
					while (p < contentLen && (contentStr[p] < '0' || contentStr[p] > '9'))
						p++;
					nextP = p;
					decimal = false;
					while (nextP < contentLen 
						&& (('0' <= contentStr[nextP] && contentStr[nextP] <= '9') 
						|| (i == 5 && !decimal && contentStr[nextP] == '.'))) {
						if (contentStr[nextP] == '.')
							decimal = true;
						nextP++;
					}
					string sub_value =  contentStr.substr(p, nextP - p + 1);
					std::cout << i << " -> "<< p << "," << nextP << "," << sub_value << endl;
					// fix bug: if nextP == p substr is empty, stof will throw invalid_argument
					if (nextP <= p) 
						break;
					std::cout << stof(sub_value) << endl;
					date.emplace_back(stof(contentStr.substr(p, nextP - p + 1)));
					p = nextP;
					if (p >= contentLen && i < 5)
						return;
				}
				dt_value = EvalMultitypeValue::DateTime(date);
			} else {
				contentStr = contentStr.substr(1, contentLen - 2);
				if (sufStr == "integer") {
					datatype = EvalMultitypeValue::xsd_integer;
					int_value = stoi(contentStr);
				} else if (sufStr == "long") {
					datatype = EvalMultitypeValue::xsd_long;
					long_value = stoll(contentStr);
				} else if (sufStr == "decimal") {
					datatype = EvalMultitypeValue::xsd_decimal;
					flt_value = stof(contentStr);
				} else if (sufStr == "float") {
					datatype = EvalMultitypeValue::xsd_float;
					flt_value = stof(contentStr);
				} else if (sufStr == "double") {
					datatype = EvalMultitypeValue::xsd_double;
					dbl_value = stod(contentStr);
				}
			}
		}
	}
}

string EvalMultitypeValue::getLangTag()
{
	if (datatype == EvalMultitypeValue::literal)
	{
		int p = str_value.rfind('@');
		if (p != -1)
			return "\"" + str_value.substr(p + 1) + "\"";
		else
			return "";
	}
	return "";
}

bool EvalMultitypeValue::argCompatible(EvalMultitypeValue &x)
{
	if (datatype == EvalMultitypeValue::xsd_string)
	{
		if ((x.datatype == EvalMultitypeValue::xsd_string)
			|| (x.datatype == EvalMultitypeValue::literal && x.isSimpleLiteral()))
			return true;
	}
	else if (datatype == EvalMultitypeValue::literal)
	{
		if (x.datatype == EvalMultitypeValue::xsd_string)
			return true;
		else if (x.datatype == EvalMultitypeValue::literal \
			&& (x.isSimpleLiteral() || getLangTag() == x.getLangTag()))
			return true;
	}

	return false;
}

string EvalMultitypeValue::getStrContent()
{
	if (datatype == EvalMultitypeValue::xsd_string || datatype == EvalMultitypeValue::literal)
	{
		if (str_value[0] != '"')
			return "";
		int p = str_value.rfind('"');
		if (p == 0)
			return "";
		return str_value.substr(1, p - 1);
	}

	return "";
}

string EvalMultitypeValue::getRep()
{
	string ret;
	if (datatype == EvalMultitypeValue::rdf_term || datatype == EvalMultitypeValue::iri)
		ret = term_value;
	else if (datatype == EvalMultitypeValue::xsd_string || datatype == EvalMultitypeValue::literal)
		ret = str_value;
	else if (datatype == EvalMultitypeValue::xsd_boolean)
	{
		if (bool_value.getValue() == 0 || bool_value.getValue() == 2)
			ret = "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
		else
			ret = "\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
	}
	else if (datatype == EvalMultitypeValue::xsd_integer)
		ret = "\"" + to_string(int_value) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
	else if (datatype == EvalMultitypeValue::xsd_long)
		ret = "\"" + to_string(int_value) + "\"^^<http://www.w3.org/2001/XMLSchema#long>";
	else if (datatype == EvalMultitypeValue::xsd_decimal)
		ret = "\"" + to_string(flt_value) + "\"^^<http://www.w3.org/2001/XMLSchema#decimal>";
	else if (datatype == EvalMultitypeValue::xsd_float)
		ret = "\"" + to_string(flt_value) + "\"^^<http://www.w3.org/2001/XMLSchema#float>";
	else if (datatype == EvalMultitypeValue::xsd_double)
		ret = "\"" + to_string(dbl_value) + + "\"^^<http://www.w3.org/2001/XMLSchema#double>";
	else if (datatype == EvalMultitypeValue::xsd_datetime)
	{
		ret = "\"" + to_string(dt_value.date[0]) + "-" + to_string(dt_value.date[1]) + "-" + \
			to_string(dt_value.date[2]) + "T" + to_string(dt_value.date[3]) + ":" + \
			to_string(dt_value.date[4]) + ":" + to_string(dt_value.date[5]) + \
			"\"^^<http://www.w3.org/2001/XMLSchema#dateTime>";
	}
	return ret;
}