#include "GeneralEvaluation.h"
#include "TempResult.h"

bool GeneralEvaluation::doHavingAggregateFunction(TempResult &result0, int result0_id_cols, int begin, int end)
{
    CompTreeNode & having = this->query_tree.getHaving();
    std::string funcName, var;
    if (having.children.size() == 2 && having.children[0].children.size() == 1)
    {
        funcName = having.children[0].oprt;
        var = having.children[0].children[0].val;
    }
    if (funcName != "COUNT" && funcName != "MIN" 
        && funcName != "MAX" && funcName != "AVG" 
        && funcName != "SUM")
    {
        SLOG_ERROR("not support this aggregate " << funcName);
        return false;;
    }

    EvalMultitypeValue tmp;
    Varset result0Varset = result0.getAllVarset();
    int count = 0;
    EvalMultitypeValue numeric_sum, numeric_min, numeric_max, datetime_min, datetime_max, res;
    numeric_sum.datatype = EvalMultitypeValue::xsd_integer;
    numeric_sum.int_value = 0;
    numeric_min.datatype = EvalMultitypeValue::xsd_integer;
    numeric_min.int_value = INT_MAX;
    numeric_max.datatype = EvalMultitypeValue::xsd_integer;
    numeric_max.int_value = INT_MIN;
    datetime_min.datatype = EvalMultitypeValue::xsd_datetime;
    datetime_max.datatype = EvalMultitypeValue::xsd_datetime;
    bool numeric = false, datetime = false;
    for (int j = begin; j <= end; j++)
    {
        tmp = result0.doComp(this->query_tree.getHaving().children[0].children[0], result0.result[j], result0_id_cols, kvstore, result0Varset);
        if (tmp.datatype == EvalMultitypeValue::xsd_boolean && tmp.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::error_value)
            continue;
        if (tmp.datatype != EvalMultitypeValue::xsd_integer
            && tmp.datatype != EvalMultitypeValue::xsd_decimal
            && tmp.datatype != EvalMultitypeValue::xsd_float
            && tmp.datatype != EvalMultitypeValue::xsd_double
            && tmp.datatype != EvalMultitypeValue::xsd_long)
        {
            if (funcName == "SUM" || funcName == "AVG")
            {
                SLOG_ERROR("[ERROR] Invalid type for SUM or AVG.");
                continue;
            }
            else if ((funcName == "MIN" || funcName == "MAX") && tmp.datatype != EvalMultitypeValue::xsd_datetime)
            {
                SLOG_ERROR("[ERROR] Invalid type for MIN or MAX.");
                continue;
            }
        }
        if (tmp.datatype == EvalMultitypeValue::xsd_datetime)
            datetime = true;
        else
            numeric = true;
        if (datetime && numeric && funcName != "COUNT")
            return false;

        if (funcName == "SUM" || funcName == "AVG")
        {
            numeric_sum = numeric_sum + tmp;
        }
        else if (funcName == "MIN")
        {
            if (numeric)
            {
                res = numeric_min > tmp;
                if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
                    numeric_min = tmp;
            }
            else if (datetime)
            {
                res = datetime_min > tmp;
                if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
                    datetime_min = tmp;
            }
        }
        else if (funcName == "MAX")
        {
            if (numeric)
            {
                res = numeric_max < tmp;
                if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
                    numeric_max = tmp;
            }
            else if (datetime)
            {
                res = datetime_max < tmp;
                if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
                    datetime_max = tmp;
            }
        }
        count++;
    }

    EvalMultitypeValue x;
    if (funcName == "COUNT")
    {
        x.datatype = EvalMultitypeValue::xsd_integer;
        x.int_value = count;
    }
    else if (funcName == "SUM")
    {
        x = numeric_sum;
    }
    else if (funcName == "MIN")
    {
        if (datetime)
        {
            x = datetime_min;
        }
        else
        {
            x = numeric_min;
        }
    }
    else if (funcName == "MAX")
    {
        if (datetime)
        {
            x = datetime_max;
        }
        else
        {
            x = numeric_max;
        }
    }
    else if (funcName == "AVG")
    {
        tmp.term_value = "\"" + to_string(count) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
		tmp.deduceTypeValue();
        x = numeric_sum / tmp;
    }

    EvalMultitypeValue y = result0.doComp(this->query_tree.getHaving().children[1], result0.result[0], result0_id_cols, kvstore, result0Varset);
    EvalMultitypeValue ret = TempResult::doCompareValue(x, y, having.oprt);
    if (ret.datatype == EvalMultitypeValue::xsd_boolean && ret.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::error_value)
    {
        SLOG_TRACE("data type is error:" << x.term_value);
        return false;
    }
    if (ret.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::false_value)
        return true;
    return false;
}