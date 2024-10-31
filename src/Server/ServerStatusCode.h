#pragma once

namespace server
{
    enum StatusCode
    {
        StatusOK = 0,
        StatusNotFound,
        // compress 
        StatusCompressError,
        StatusCompressNotSupport,
        StatusNoComrpess,
        StatusUncompressError,
        StatusUncompressNotSupport,
        StatusNoUncomrpess,

        // File
        StatusFileRangeInvalid,
        StatusFileReadError,
        StatusFileWriteError,

        // Json
        StatusJsonInvalid,
        
        StatusProxyError,

        // Route
        StatusRouteVerbNotImplment,
        StatusRouteNotFound,

        // API
        StatusMethodTypeUnsupport = 1000,
        StatusAuthenticationFailed,
        StatusCheckPrivilegeFailed,
        StatusParamIsIllegal,
        StatusOperationConditionsAreNotSatisfied,
        StatusOperationFailed,
        StatusAddPrivilegeFaied,
        StatusLossOfLock,
        StatusTranscationManageFailed,
        StatusOperationUndefined = 1100,
        StatusIPBlocked,
        StatusLicenseExpired,
        StatusLicenseInvalid,
    };

    const char* error_code_to_str(int code);
}