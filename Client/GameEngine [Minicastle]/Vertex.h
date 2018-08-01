#pragma once
#include "MathHelper.h"

struct PNTVertex
{
	XMFLOAT3 m_Position;
	XMFLOAT3 m_Normal;
	XMFLOAT2 m_UV;

	bool operator==(const PNTVertex& rhs) const
	{
		uint32_t position;
		uint32_t normal;
		uint32_t uv;

		XMVectorEqualR(&position, XMLoadFloat3(&(this->m_Position)), XMLoadFloat3(&rhs.m_Position));
		XMVectorEqualR(&normal, XMLoadFloat3(&(this->m_Normal)), XMLoadFloat3(&rhs.m_Normal));
		XMVectorEqualR(&uv, XMLoadFloat2(&(this->m_UV)), XMLoadFloat2(&rhs.m_UV));

		return XMComparisonAllTrue(position) && XMComparisonAllTrue(normal) && XMComparisonAllTrue(uv);
	}
};

struct VertexBlendingInfo
{
	unsigned int m_BlendingIndex;
	double m_BlendingWeight;

	VertexBlendingInfo():
		m_BlendingIndex(0),
		m_BlendingWeight(0.0)
	{}

	bool operator < (const VertexBlendingInfo& rhs)
	{
		return (m_BlendingWeight > rhs.m_BlendingWeight);
	}
};

struct PNTIWVertex
{
	XMFLOAT3 m_Position;
	XMFLOAT3 m_Normal;
	XMFLOAT2 m_UV;
	std::vector<VertexBlendingInfo> m_VertexBlendingInfos;

	void SortBlendingInfoByWeight()
	{
		std::sort(m_VertexBlendingInfos.begin(), m_VertexBlendingInfos.end());
	}

	bool operator==(const PNTIWVertex& rhs) const
	{
		bool sameBlendingInfo = true;

		// We only compare the blending info when there is blending info
		if(!(m_VertexBlendingInfos.empty() && rhs.m_VertexBlendingInfos.empty()))
		{
			// Each vertex should only have 4 index-weight blending info pairs
			for (unsigned int i = 0; i < 4; ++i)
			{
				if (m_VertexBlendingInfos[i].m_BlendingIndex != rhs.m_VertexBlendingInfos[i].m_BlendingIndex ||
					abs(m_VertexBlendingInfos[i].m_BlendingWeight - rhs.m_VertexBlendingInfos[i].m_BlendingWeight) > 0.001)
				{
					sameBlendingInfo = false;
					break;
				}
			}
		}
		
		bool result1 = MathHelper::CompareVector3WithEpsilon(m_Position, rhs.m_Position);
		bool result2 = MathHelper::CompareVector3WithEpsilon(m_Normal, rhs.m_Normal);
		bool result3 = MathHelper::CompareVector2WithEpsilon(m_UV, rhs.m_UV);

		return result1 && result2 && result3 && sameBlendingInfo;
	}
};
struct PNTIWVertexVector
{
	std::vector<PNTIWVertex> m_Vertices;
};