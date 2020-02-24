#pragma once

#include<iostream>
#include<fstream>
#include<vector>
#include<filesystem>

using namespace std;
using namespace std::filesystem;

const path _getShaderDir()
{
    WCHAR p[MAX_PATH];
    GetModuleFileName(NULL, p, MAX_PATH);

    const path uri = path(wstring(p)).parent_path().append("Shaders");
    return uri;
}

bool GetShader(string name, D3D12_SHADER_BYTECODE& data) {
    path fullName = _getShaderDir();
    fullName.append(name);
    fullName.concat(".cso");

    //DX:GetShader(fullName.string(), data);

    ifstream file(fullName, ios::out | ios::binary);
    if (!file) {
        cout << "Cannot open file!" << endl;
        return 1;
    }

    file.seekg(0, ios::end);
    size_t size = file.tellg();
    
    file.seekg(0, ios::beg);
    
    vector<uint8_t> buffer;
    
    char ch;
    while (!file.eof())
    {
        file.get(ch);
        buffer.push_back(ch);
    }
    file.close();

    data.BytecodeLength = size;
    data.pShaderBytecode = &buffer[0];

    return true;
}