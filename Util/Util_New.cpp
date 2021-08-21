#include "Util_New.h"
using namespace std;

/// <summary>
/// Check the file or Dir is exist or not.
/// 
/// </summary>
/// <param name="name">the file path or the dir path</param>
/// <returns></returns>
static bool Util_New::checkFileOrDirIsExist(const string& name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}
static string Util_New::getCurrentRootPath()
{
    char* buffer;
    string result;
    

    char pwd[255];
    char* wd;
    //也可以将buffer作为输出参数
    if ((buffer = getcwd(NULL, 0)) == NULL)
    {
        perror("getcwd error");
    }
    else
    {
        printf("%s\n", buffer);
        free(buffer);
    }

    wd = getwd(pwd);
    if (!wd) {
        perror("getcwd");
      
    }
    printf("\ngetwd pwd is %s\n", wd);
    wd = get_current_dir_name();
    if (!wd) {
        perror("getcwd");
        
    }
    printf("\nget_current pwd is %s\n", wd);

    if ((buffer = getcwd(NULL, 0)) == NULL)
    {
        return "";
    }
    else
    {
        result = buffer;
        return result;
    }
}
