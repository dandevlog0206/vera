#include "../../include/vera/scene/mesh_attribute.h"

VERA_NAMESPACE_BEGIN
VERA_SCENE_NAMESPACE_BEGIN

MeshAttribute::MeshAttribute()
{

}

obj<MeshAttribute> MeshAttribute::create()
{
	return obj<MeshAttribute>(new MeshAttribute());
}

MeshAttribute::~MeshAttribute()
{
}

VERA_NODISCARD AttributeType MeshAttribute::getType() const VERA_NOEXCEPT
{
	return AttributeType::Mesh;
}

VERA_NODISCARD const std::vector<float3>& MeshAttribute::getVertices() const VERA_NOEXCEPT
{
	return m_vertices;
}

void MeshAttribute::setVertices(const std::vector<float3>& vertices) VERA_NOEXCEPT
{
	m_vertices = vertices;
}

void MeshAttribute::setVertices(std::vector<float3>&& vertices) VERA_NOEXCEPT
{
	m_vertices = std::move(vertices);
}

VERA_NODISCARD const std::vector<uint32_t>& MeshAttribute::getIndices() const VERA_NOEXCEPT
{
	return m_indices;
}

void MeshAttribute::setIndices(const std::vector<uint32_t>& indices) VERA_NOEXCEPT
{
	m_indices = indices;
}

void MeshAttribute::setIndices(std::vector<uint32_t>&& indices) VERA_NOEXCEPT
{
	m_indices = std::move(indices);
}

VERA_NODISCARD const std::vector<float2>& MeshAttribute::getUVs() const VERA_NOEXCEPT
{
	return m_uvs;
}

void MeshAttribute::setUVs(const std::vector<float2>& uvs) VERA_NOEXCEPT
{
	m_uvs = uvs;
}

void MeshAttribute::setUVs(std::vector<float2>&& uvs) VERA_NOEXCEPT
{
	m_uvs = std::move(uvs);
}

VERA_SCENE_NAMESPACE_END
VERA_NAMESPACE_END
