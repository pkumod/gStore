#include "License.h"

using namespace std;
using namespace nlohmann;

LicenseHelper::LicenseHelper() {}
LicenseHelper::~LicenseHelper() {}

std::string LicenseHelper::rsa_pub_decrypt(const std::string &cipherText)
{
  std::string strRet;
  RSA *rsa = RSA_new();
  BIO *keybio = BIO_new_mem_buf((unsigned char *)pub_key.c_str(), -1);
  rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);

  int len = RSA_size(rsa);
  char *decryptedText = new char[len + 1]{};

  int ret = RSA_public_decrypt(cipherText.length(), (const unsigned char *)cipherText.c_str(), (unsigned char *)decryptedText, rsa, RSA_PKCS1_PADDING);
  if (ret >= 0)
    strRet = std::string(decryptedText, ret);

  delete[] decryptedText;
  BIO_free_all(keybio);
  RSA_free(rsa);

  return strRet;
}

vector<char> LicenseHelper::hexToBytes(const std::string &hex)
{
  vector<char> bytes;
  for (size_t i = 0; i < hex.length(); i += 2)
  {
    std::string byteString = hex.substr(i, 2);
    unsigned char byte = static_cast<char>(stoi(byteString, nullptr, 16));
    bytes.push_back(byte);
  }
  return bytes;
}

std::string LicenseHelper::removeSpaces(const std::string &str)
{
  std::string result;
  for (char ch : str)
  {
    if (ch != ' ')
    {
      result += ch;
    }
  }
  return result;
}

std::string LicenseHelper::base64Encode(const vector<char> &data)
{
  std::string base64String;

  BIO *memBio = BIO_new(BIO_s_mem());
  BIO *base64Bio = BIO_new(BIO_f_base64());
  BIO *bio = BIO_push(base64Bio, memBio);

  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

  BIO_write(bio, data.data(), static_cast<int>(data.size()));
  BIO_flush(bio);

  BUF_MEM *bufferPtr = nullptr;
  BIO_get_mem_ptr(bio, &bufferPtr);
  if (bufferPtr != nullptr && bufferPtr->data != nullptr && bufferPtr->length > 0)
  {
    base64String.assign(bufferPtr->data, bufferPtr->data + bufferPtr->length);
  }

  BIO_free_all(bio);

  return base64String;
}

std::string LicenseHelper::base64Decode(const std::string &encodedData)
{
  BIO *bio = BIO_new(BIO_f_base64());
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

  BIO *bmem = BIO_new_mem_buf(encodedData.data(), static_cast<int>(encodedData.length()));
  bio = BIO_push(bio, bmem);

  const int bufferSize = encodedData.length();
  vector<char> buffer(bufferSize);

  int decodedLength = BIO_read(bio, buffer.data(), bufferSize);

  std::string decodedData(buffer.data(), decodedLength);

  BIO_free_all(bio);

  return decodedData;
}

std::string LicenseHelper::loadBinaryFile(const std::string &filename)
{
  ifstream inputFile(filename, ios::binary);
  vector<char> fileData;

  if (inputFile)
  {
    inputFile.seekg(0, ios::end);
    streampos fileSize = inputFile.tellg();
    inputFile.seekg(0, ios::beg);

    fileData.resize(fileSize);

    inputFile.read(fileData.data(), fileSize);

    if (!inputFile)
    {
      SLOG_ERROR("Error reading file: " << filename);
      fileData.clear();
    }
  }
  else
  {
    SLOG_ERROR("Failed to open file: " << filename);
  }

  std::string base64String = base64Encode(fileData);

  return base64String;
}

std::string LicenseHelper::loadHexFile(const std::string &filename)
{
  ifstream inputFile(filename);
  std::string base64String;

  if (inputFile.is_open())
  {
    std::string hexString;
    getline(inputFile, hexString);

    std::string hexStringNoSpaces = removeSpaces(hexString);

    vector<char> binaryData = hexToBytes(hexStringNoSpaces);

    base64String = base64Encode(binaryData);
  }
  else
  {
    SLOG_ERROR("Failed to open the file.");
  }
  return base64String;
}

bool LicenseHelper::getCPUID(std::string &cpuid)
{
  FILE *pipe = popen("dmidecode -t processor |grep ID |sort -u|awk -F': ' '{print $2}'", "r");
  if (!pipe)
  {
    SLOG_ERROR("Failed to run command: dmidecode -t processor |grep ID |sort -u|awk -F': ' '{print $2}'");
    return false;
  }
  char buffer[512];
  bool rt = false;
  if (fgets(buffer, sizeof(buffer), pipe) != NULL)
  {
    cpuid = string(buffer);
    cpuid.erase(std::remove(cpuid.begin(), cpuid.end(), '\n'), cpuid.end());
    cpuid.erase(std::remove(cpuid.begin(), cpuid.end(), '\r'), cpuid.end());
    cpuid.erase(std::remove(cpuid.begin(), cpuid.end(), ' '), cpuid.end());
    rt = true;
  }
  pclose(pipe);
  return rt;
}

bool LicenseHelper::getMacAddress(std::string &mac)
{

  struct ifreq ifr;
  struct ifconf ifc;
  // char sa_data[14];
  // ifr.ifr_hwaddr.sa_data=sa_data;
  char buf[1024];
  int success = 0;

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock == -1)
  {
    return false;
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
  {
    return false;
  }

  struct ifreq *it = ifc.ifc_req;
  const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

  for (; it != end; ++it)
  {
    strcpy(ifr.ifr_name, it->ifr_name);
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
    {
      if (!(ifr.ifr_flags & IFF_LOOPBACK))
      { // don't count loopback
        if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
        {
          success = 1;
          break;
        }
      }
    }
    else
    {
      return false;
    }
  }

  if (success)
  {
    char mac_address[18];
    sprintf(mac_address, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char)ifr.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr.ifr_hwaddr.sa_data[1], (unsigned char)ifr.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr.ifr_hwaddr.sa_data[3], (unsigned char)ifr.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    mac = string(mac_address);
    return true;
  }
  else
  {
    return false;
  }
}

bool LicenseHelper::file_exist(const char *_path)
{
  struct stat buffer;
  return (stat(_path, &buffer) == 0);
}

bool LicenseHelper::validLicense(struct LicenseInfo &licenseInfo, const std::string licenseFile)
{
  SLOG_CORE("license_file:" << licenseFile);
  if (licenseFile.empty() || file_exist(licenseFile.c_str()) == false)
  {
    licenseInfo.isvalid = false;
    licenseInfo.desc = "The license file is not exist";
    return false;
  }
  std::string licenseStr = loadBinaryFile(licenseFile);
  return validLicense(licenseInfo, licenseStr.c_str());
}

bool LicenseHelper::validLicense(struct LicenseInfo &licenseInfo, const char *licenseContent)
{
  SLOG_CORE("content:" << licenseContent);
  std::string decodeStr = base64Decode(licenseContent);
  std::string decStr = rsa_pub_decrypt(decodeStr);
  std::string product, version;
  // 解析JSON字符串
  nlohmann::json data;
  if (nlohmann::json::accept(decStr))
  {
    try
    {
      data = nlohmann::json::parse(decStr);
      SLOG_CORE("parse license success:\n" << data.dump(4));
    }
    catch (const nlohmann::json::exception &e)
    {
      SLOG_ERROR("parse license error: " << e.what());
      licenseInfo.isvalid = false;
      licenseInfo.desc = "the license is not complete!";
      return false;
    }
  }
  else
  {
    SLOG_ERROR("license is not a json string: " << decStr);
    licenseInfo.isvalid = false;
    licenseInfo.desc = "the license is not complete!";
    return false;
  }

  product = "";
  if (data.contains("product"))
  {
    data.at("product").get_to(product);
  }
  if (data.contains("version"))
  {
    data.at("version").get_to(version);
  }
  if (product != licenseInfo.product)
  {
    licenseInfo.isvalid = false;
    licenseInfo.desc = "The product name[" + product + "] does not match";
    return false;
  }
  else if (version != licenseInfo.version)
  {
    licenseInfo.isvalid = false;
    licenseInfo.desc = "The version[" + version + "] is mismatch";
    return false;
  }
  licenseInfo.fromJSON(data);
  licenseInfo.isvalid = true;
  if (!licenseInfo.validDate())
  {
    return false;
  }
  std::string licenseMac;
  if (data.contains("mac"))
  {
    data.at("mac").get_to(licenseMac);
  }
  if (licenseMac.empty())
  {
    // trial license
    licenseInfo.type = "0";
    licenseInfo.isvalid = true;
    licenseInfo.content = licenseContent;
    return true;
  }
  else
  {
    // check MAC and CPUID
    std::string mac;
    if (getMacAddress(mac) == false || licenseMac != mac)
    {
      SLOG_CORE("current mac: " + mac + ", license mac: " + licenseMac);
      licenseInfo.isvalid = false;
      licenseInfo.desc = "The MAC address is mismatch!";
      return false;
    }
    if (data.contains("cpu"))
    {
      std::string licenseCPU;
      std::string cpuid;
      data.at("cpu").get_to(licenseCPU);
      if (getCPUID(cpuid) == false || licenseCPU != cpuid)
      {
        SLOG_CORE("current CPU ID: " + cpuid + ", license CPU ID: " + licenseCPU);
        licenseInfo.isvalid = false;
        licenseInfo.desc = "The CPU ID is mismatch!";
        return false;
      }
    }
    // business license
    licenseInfo.type = "1";
    licenseInfo.isvalid = true;
    licenseInfo.content = licenseContent;
    return true;
  }
}