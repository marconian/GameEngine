#include "pch.h"

#include<iostream>
#include<fstream>
#include<vector>
#include<filesystem>
#include<d3dcompiler.h>

#include "ShaderTools.h"

using namespace std;
using namespace std::filesystem;

const path _getShaderDir()
{
	WCHAR p[MAX_PATH];
	GetModuleFileName(nullptr, p, MAX_PATH);

	const path uri = path(wstring(p)).parent_path().append("Shaders");
	return uri;
}

path GetShaderPath(string name)
{
	path fullName = _getShaderDir();
	fullName.append(name);
	fullName.concat(".cso");

	return fullName;
}

bool GetShader(string name, D3D12_SHADER_BYTECODE& data)
{
	path fullName = GetShaderPath(name);

	ID3DBlob* blob;
	D3DReadFileToBlob(fullName.c_str(), &blob);

	data.BytecodeLength = blob->GetBufferSize();
	data.pShaderBytecode = blob->GetBufferPointer();

	return true;
}
