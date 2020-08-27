#pragma once
/********************************************************************************************
*							ObjReader.h														*
*		ObjReader类读取Obj文件并写入自定义二进制文件.mbo，这样可以避免反复读取.obj文件耗时的情况	*
*	由于ObjReader类对.obj格式的文件要求比较严格，如果出现不能正确加载的现象，						*
*	请检查是否出现下面这些情况，否则需要自行修改.obj/.mtl文件，或者给ObjReader实现更多的功能：		*
*																							*
*	使用了/将下一行的内容连接在一起表示一行														*
*	存在索引为负数																			*
*	使用了类似1//2这样的顶点（即不包含纹理坐标的顶点）											*
*	使用了绝对路径的文件引用																	*
*	相对路径使用了.和..两种路径格式																*
*	若.mtl材质文件不存在，则内部会使用默认材质值													*
*	若.mtl内部没有指定纹理文件引用，需要另外自行加载纹理											*
*	f的顶点数不为3(网格只能以三角形构造，即一个f的顶点数只能为3)									*
********************************************************************************************/


#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <locale>
#include <filesystem>
#include "Vertex.h"
#include "light.h"


struct ObjPart {
	Material material;							// 材质
	std::vector<VertexPosNormalTangentTex> vertices;  // 顶点集合
	std::vector<WORD> indices16;				// 顶点数不超过65535时使用
	std::vector<DWORD> indices32;				// 顶点数超过65535时使用
	std::wstring texStrDiffuse;
	std::wstring normalMap;
	ObjPart() = default;
};

class ObjReader
{
public:

	// 有指定.mbo文件时直接读取.mbo， 否则得先都.obj
	bool Read(const wchar_t* mboFilename, const wchar_t* objFileName);

	bool ReadObj(const wchar_t* objFileName);
	bool ReadMbo(const wchar_t* mboFileName);
	bool WriteMbo(const wchar_t* mboFileName);

public:
	std::vector<ObjPart> objParts;
	DirectX::XMFLOAT3 vMin, vMax; // AABB盒双顶点

private:
	void AddVertex(const VertexPosNormalTangentTex& vertex, DWORD vpi, DWORD vti, DWORD vni);

	// 储存 v/vt/vn 顶点信息
	std::unordered_map<std::wstring, DWORD> vertexCache;

};


class MtlReader {
public:
	bool ReadMtl(const wchar_t* mtlFileName);

public:
	std::map<std::wstring, Material> materials;
	std::map<std::wstring, std::wstring> mapKaStrs;
	std::map<std::wstring, std::wstring> mapKdStrs;
	std::map<std::wstring, std::wstring> mapKsStrs;
	std::map<std::wstring, std::wstring> mapDStrs;
	std::map<std::wstring, std::wstring> mapBumpStrs;
	std::map<std::wstring, std::wstring> BumpStrs;
};
