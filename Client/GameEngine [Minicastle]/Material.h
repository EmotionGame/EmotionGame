#pragma once
#include "Vertex.h"

class Material
{
public:
	bool Lambert;
	bool Phong;

	std::string mName;
	XMFLOAT3 mAmbient;
	XMFLOAT3 mDiffuse;
	XMFLOAT3 mEmissive;
	/***** Phong 추가 *****/
	XMFLOAT3 mSpecular;
	XMFLOAT3 mReflection;
	double mSpecularPower;
	double mShininess;
	double mReflectionFactor;
	/***** Phong 추가 *****/
	double mTransparencyFactor;

	std::string mDiffuseMapName;
	std::string mEmissiveMapName;
	std::string mGlossMapName;
	std::string mNormalMapName;
	std::string mSpecularMapName;

	std::string mDiffuseMapRelativeName;
	std::string mEmissiveMapRelativeName;
	std::string mGlossMapRelativeName;
	std::string mNormalMapRelativeName;
	std::string mSpecularMapRelativeName;
};

struct MaterialUnorderedMap
{
	std::unordered_map<unsigned int, Material*> m_MaterialLookUp;
};

struct MaterialIndexUMap
{
	std::unordered_map<unsigned int, unsigned int> m_MaterialIndexUMap;
};