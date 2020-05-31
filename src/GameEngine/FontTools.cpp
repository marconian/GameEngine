#include "pch.h"

#include<iostream>
#include<fstream>
#include<vector>
#include<filesystem>

#include "FontTools.h"

using namespace std;
using namespace std::filesystem;

const path _getFontsDir()
{
    WCHAR p[MAX_PATH];
    GetModuleFileName(NULL, p, MAX_PATH);

    const path uri = path(wstring(p)).parent_path().append("Fonts");
    return uri;
}

path GetFontsPath(const string name, const size_t size)
{
    path fullName = _getFontsDir();
    fullName.append(name + "_" + std::to_string(size));
    fullName.concat(".spritefont");

    return fullName;
}