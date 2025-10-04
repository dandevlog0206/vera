#pragma once

#include "attribute.h"
#include "../math/vector_types.h"
#include <vector>

VERA_NAMESPACE_BEGIN
VERA_SCENE_NAMESPACE_BEGIN

class MeshAttribute : public Attribute
{
	MeshAttribute();
public:
	static obj<MeshAttribute> create();
	~MeshAttribute() override;

	VERA_NODISCARD AttributeType getType() const VERA_NOEXCEPT override;

	VERA_NODISCARD const std::vector<float3>& getVertices() const VERA_NOEXCEPT;
	void setVertices(const std::vector<float3>& vertices) VERA_NOEXCEPT;
	void setVertices(std::vector<float3>&& vertices) VERA_NOEXCEPT;
	VERA_NODISCARD const std::vector<uint32_t>& getIndices() const VERA_NOEXCEPT;
	void setIndices(const std::vector<uint32_t>& indices) VERA_NOEXCEPT;
	void setIndices(std::vector<uint32_t>&& indices) VERA_NOEXCEPT;
	VERA_NODISCARD const std::vector<float2>& getUVs() const VERA_NOEXCEPT;
	void setUVs(const std::vector<float2>& uvs) VERA_NOEXCEPT;
	void setUVs(std::vector<float2>&& uvs) VERA_NOEXCEPT;

private:
	std::vector<float3>   m_vertices;
	std::vector<uint32_t> m_indices;
	std::vector<float2>   m_uvs;
};

VERA_SCENE_NAMESPACE_END
VERA_NAMESPACE_END
