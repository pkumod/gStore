#include "grpc_status_code.h"
#include <map>

namespace grpc
{

std::map<int, const char *> error_code_table = {
    { StatusOK, "Success" },
    { StatusCompressError, "Compress Error" },
    { StatusCompressNotSupport, "Compress Not Support" },
    { StatusNoComrpess, "No Comrpession" },
    { StatusUncompressError, "Uncompress Error" },
    { StatusUncompressNotSupport, "Uncompress Not Support" },
    { StatusNoUncomrpess, "No Uncomrpess" },
    { StatusNotFound, "404 Not Found" },
    { StatusFileRangeInvalid, "File Range Invalid" },
    { StatusFileReadError, "File Read Error" },
    { StatusFileWriteError, "File Write Error" },
    { StatusJsonInvalid, "Invalid Json Syntax" },
    { StatusProxyError, "Http Proxy Error" },
    { StatusRouteVerbNotImplment, "Route Http Method not implement" },
    { StatusRouteNotFound, "Route Not Found" },

    { StatusMethodTypeUnsupport, "The method type is not support" },
    { StatusAuthenticationFailed, "Authentication Failed" },
    { StatusCheckPrivilegeFailed, "Check Privilege Failed" },
    { StatusParamIsIllegal, "Param is illegal" },
    { StatusOperationConditionsAreNotSatisfied, "The operation conditions are not satisfied" },
    { StatusOperationFailed, "Operation failed" },
    { StatusAddPrivilegeFaied, "Add privilege Failed" },
    { StatusLossOfLock, "Loss of lock" },
    { StatusTranscationManageFailed, "Transcation manage Failed" },
    { StatusOperationUndefined, "The operation is not defined" },
    { StatusIPBlocked, "IP Blocked" },
};
 
const char* error_code_to_str(int code)
{
    auto it = error_code_table.find(code);
    if(it == error_code_table.end())
        return "unknown";
    return it->second;
}
    
} // namespace grpc

