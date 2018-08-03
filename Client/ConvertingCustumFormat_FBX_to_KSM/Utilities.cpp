#include "stdafx.h"
#include "Utilities.h"

FbxAMatrix Utilities::GetGeometryTransformation(FbxNode* inNode)
{
	if (!inNode)
	{
		throw std::exception("Null for mesh geometry");
	}

	const FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

std::string Utilities::GetFileName(const std::string& inInput)
{
	std::string seperator("\\");
	unsigned int pos = inInput.find_last_of(seperator);
	if (pos != std::string::npos)
	{
		return inInput.substr(pos + 1);
	}
	else
	{
		return inInput;
	}
}

std::string Utilities::RemoveSuffix(const std::string& inInput)
{
	std::string seperator(".");
	unsigned int pos = inInput.find_last_of(seperator);
	if (pos != std::string::npos)
	{
		return inInput.substr(0, pos);
	}
	else
	{
		return inInput;
	}
}