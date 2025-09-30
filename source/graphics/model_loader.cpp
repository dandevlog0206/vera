#include "../../include/vera/graphics/model_loader.h"

#include "../../include/vera/core/exception.h"
#include <filesystem>
#include <fstream>
#include <sstream>

#ifndef _MSC_VER
#define sscanf_s sscanf
#endif

VERA_NAMESPACE_BEGIN

static std::string read_str_data(std::string_view path, std::ios_base::openmode mode = {})
{
	std::ifstream file(path.data(), mode | std::ios_base::ate);
	std::string   result;

	if (!file.is_open())
		throw Exception("failed to open model file named " + std::string(path));

	result.resize(file.tellg());
	file.seekg(0, std::ios_base::beg);

	file.read(result.data(), result.size());

	return result;
}

static void load_obj(
	std::string_view     path,
	std::vector<float3>& out_vertices,
	std::vector<float3>& out_normals,
	std::vector<float2>& out_uvs
) {
	std::ifstream file(path.data());

	if (!file.is_open())
		throw Exception("failed to open model file named " + std::string(path));

	std::vector<float3> vertices;
	std::vector<float3> normals;
	std::vector<float2> uvs;

	std::string line;
	std::string line_header;
	while (std::getline(file, line)) {
		std::istringstream ss(line);
		ss >> line_header;

		if (line_header == "v") {
			auto& v = vertices.emplace_back();
			ss >> v.x >> v.z >> v.y;
		} else if (line_header == "vt") {
			auto& uv = uvs.emplace_back();
			ss >> uv.x >> uv.y;
			uv.y = 1.f - uv.y;
		} else if (line_header == "vn") {
			auto& n = normals.emplace_back();
			ss >> n.x >> n.z >> n.y;
		} else if (line_header == "f") {
			int v0_idx, v1_idx, v2_idx;
			int n0_idx, n1_idx, n2_idx;
			int uv0_idx, uv1_idx, uv2_idx;

			if (sscanf_s(line.c_str(),
				"f %d/%d/%d %d/%d/%d %d/%d/%d", 
					&v0_idx, &uv0_idx, &n0_idx,
					&v1_idx, &uv1_idx, &n1_idx,
					&v2_idx, &uv2_idx, &n2_idx) != 9) 
				throw Exception("invalid file format");

			out_vertices.push_back(vertices[v0_idx - 1]);
			out_vertices.push_back(vertices[v1_idx - 1]);
			out_vertices.push_back(vertices[v2_idx - 1]);
			out_normals.push_back(normals[n0_idx - 1]);
			out_normals.push_back(normals[n1_idx - 1]);
			out_normals.push_back(normals[n2_idx - 1]);
			out_uvs.push_back(uvs[uv0_idx - 1]);
			out_uvs.push_back(uvs[uv1_idx - 1]);
			out_uvs.push_back(uvs[uv2_idx - 1]);
		} else {
			continue;
		}
	}
}

ModelLoader::ModelLoader(std::string_view path)
{
	load(path);
}

ModelLoader::~ModelLoader()
{
	clear();
}

void ModelLoader::load(std::string_view path)
{
	auto ext = std::filesystem::path(path).extension();

	if (ext == ".obj") {
		load_obj(
			path,
			vertices,
			normals,
			uvs);
	} else {
		throw Exception("unsupported file extension " + std::string(ext.generic_string()));
	}
}

void ModelLoader::clear()
{
	vertices.clear();
	normals.clear();
	uvs.clear();
}

bool ModelLoader::empty() const
{
	return vertices.empty() && normals.empty() && uvs.empty();
}

VERA_NAMESPACE_END