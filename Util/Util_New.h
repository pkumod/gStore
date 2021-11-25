#pragma once
/*=============================================================================
# Filename: Util_New.h
# Author: liwenjie
# Mail: liwenjiehn@pku.edu.cn
# Created Date:2021-8-3
# Last Modified: 2021-8-3 15:48:28
# Description:
1. firstly written by liwenjie
2. common macros, functions, classes, etc
=============================================================================*/
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;
class Util_New
{
public:
	//check the file or Dir is exist or not
	static bool checkFileOrDirIsExist(const string& name);
	static string getCurrentRootPath();
};