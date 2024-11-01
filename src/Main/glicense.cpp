#pragma onece
#include "../Util/Util.h"
#include "../Util/License.h"

using namespace std;

int main(int argc, char** argv) {
    Util util;

    std::string productName = util.getConfigureValue("product_name");
    std::string version = util.getConfigureValue("version");
    std::string lic_path = "new.lic";

    std:string msg;
    struct LicenseInfo licenseInfo;
    licenseInfo.product = productName;
    licenseInfo.version = version;

    LicenseHelper licenseHelper;
    bool rt = licenseHelper.validLicense(licenseInfo, lic_path);
    SLOG_DEBUG("license valid result: " + to_string(rt) + ", msg: " + licenseInfo.desc);
    SLOG_DEBUG("license info:" << nlohmann::json(licenseInfo).dump(4));
}