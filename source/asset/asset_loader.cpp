#include "detail/fbx_loader.h"

#include <fstream>
#include <filesystem>

VERA_NAMESPACE_BEGIN

AssetLoader::AssetLoader() VERA_NOEXCEPT :
	m_result(AssetResult::Uninitialized),
	m_file_format(AssetFileFormat::Unknown)
{
	m_scene = scene::Scene::create("scene");
}

AssetLoader::~AssetLoader()
{
}

AssetResult AssetLoader::loadModel(std::string_view path) VERA_NOEXCEPT
{
	ResultMessage<AssetResult> result;

	auto ext = std::filesystem::path(path).extension().string();

	if (ext == ".fbx") {
		result = FBXLoader::load(*this, path);
	} else if (ext == ".gltf") {
		VERA_ASSERT_MSG(false, "gltf is not supported yet");
	} else {
		result = AssetResult::UnsupportedFormat;
	}

	m_result  = result.result();
	m_message = result.what();

	return m_result;
}

obj<scene::Scene> AssetLoader::getScene() VERA_NOEXCEPT
{
	return m_scene;
}

AssetResult AssetLoader::getResult() const VERA_NOEXCEPT
{
	return m_result;
}

std::string_view AssetLoader::getErrorMessage() const VERA_NOEXCEPT
{
	return m_message;
}

AssetFileFormat AssetLoader::getFileFormat() const VERA_NOEXCEPT
{
	return m_file_format;
}

VERA_NAMESPACE_END
