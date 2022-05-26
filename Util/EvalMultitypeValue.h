#ifndef _UTIL_EVALMULTITYPEVALUE_H
#define _UTIL_EVALMULTITYPEVALUE_H

#include "Util.h"

class EvalMultitypeValue
{
	public:
		class EffectiveBooleanValue
		{
			public:
				enum EBV {true_value, false_value, error_value};
				EBV value;

				EffectiveBooleanValue():value(error_value){}
				EffectiveBooleanValue(bool _value){	if (_value)	value = true_value;	else value = false_value;	}
				EffectiveBooleanValue(EBV _value):value(_value){}

				EffectiveBooleanValue operator ! ();
				EffectiveBooleanValue operator || (const EffectiveBooleanValue &x);
				EffectiveBooleanValue operator && (const EffectiveBooleanValue &x);
				EffectiveBooleanValue operator == (const EffectiveBooleanValue &x);
				EffectiveBooleanValue operator != (const EffectiveBooleanValue &x);
				EffectiveBooleanValue operator < (const EffectiveBooleanValue &x);
				EffectiveBooleanValue operator <= (const EffectiveBooleanValue &x);
				EffectiveBooleanValue operator > (const EffectiveBooleanValue &x);
				EffectiveBooleanValue operator >= (const EffectiveBooleanValue &x);

				int getValue();	// 0 for false, 1 for true, 2 for error
		};

		class DateTime
		{
			// private:
		public:
				std::vector<int> date;
				//year = date[0]
				//month = date[1]
				//day = date[2]
				//hour = date[3]
				//minute = date[4]
				//second = date[5]

			// public:
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

				EffectiveBooleanValue operator == (const DateTime &x);
				EffectiveBooleanValue operator != (const DateTime &x);
				EffectiveBooleanValue operator < (const DateTime &x);
				EffectiveBooleanValue operator <= (const DateTime &x);
				EffectiveBooleanValue operator > (const DateTime &x);
				EffectiveBooleanValue operator >= (const DateTime &x);
		};

		// Treating xsd_long as xsd_integer doesn't work. 8-byte assign to 4-byte causes memory problems
		enum DataType {rdf_term, iri, literal, xsd_string,
			xsd_boolean, xsd_integer, xsd_decimal, xsd_float, xsd_long, xsd_double,
			xsd_datetime};

		DataType datatype;
		std::string term_value;
		std::string str_value;
		EffectiveBooleanValue bool_value;
		int int_value;
		float flt_value;	// xsd_decimal and xsd_float
		double dbl_value;
		long long long_value;
		DateTime dt_value;

		bool isSimpleLiteral();
		void convert2Type(EvalMultitypeValue::DataType to_type);
		void getSameNumericType (EvalMultitypeValue &x);
		EvalMultitypeValue operator !();
		EvalMultitypeValue operator || (EvalMultitypeValue &x);
		EvalMultitypeValue operator && (EvalMultitypeValue &x);
		EvalMultitypeValue operator == (EvalMultitypeValue &x);
		EvalMultitypeValue operator != (EvalMultitypeValue &x);
		EvalMultitypeValue operator < (EvalMultitypeValue &x);
		EvalMultitypeValue operator <= (EvalMultitypeValue &x);
		EvalMultitypeValue operator > (EvalMultitypeValue &x);
		EvalMultitypeValue operator >= (EvalMultitypeValue &x);

		EvalMultitypeValue operator + (EvalMultitypeValue &x);
		EvalMultitypeValue operator - ();
		EvalMultitypeValue operator - (EvalMultitypeValue &x);
		EvalMultitypeValue operator * (EvalMultitypeValue &x);
		EvalMultitypeValue operator / (EvalMultitypeValue &x);

		EvalMultitypeValue():datatype(rdf_term), int_value(0), flt_value(0), dbl_value(0){}

		void deduceTermValue();	// Set term value according to datatype and essential value
		void deduceTypeValue();	// Set datatype and value according to term_value, for numeric & boolean

		std::string getLangTag();	// Return language tag of literal
		bool argCompatible(EvalMultitypeValue &x);	// Check argument compatibility
													// (for string funcs that takes two arguments)
		std::string getStrContent();	// Return string content of literal/xsd_string
		std::string getRep();	// Get the full representation (with suffix)
};

#endif //_UTIL_EVALMULTITYPEVALUE_H