#pragma once

#include "../math/vector_types.h"
#include <string_view>
#include <vector>

VERA_NAMESPACE_BEGIN

class ModelLoader
{
public:
	ModelLoader() = default;
	ModelLoader(std::string_view path);
	~ModelLoader();

	void load(std::string_view path);

	void clear();

	bool empty() const;

	std::vector<float3> vertices;
	std::vector<float3> normals;
	std::vector<float2> uvs;
};

VERA_NAMESPACE_END