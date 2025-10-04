#pragma once

#include "../scene/scene.h"
#include "../util/version.h"
#include <vector>
#include <string_view>

VERA_NAMESPACE_BEGIN

enum class AssetFileFormat VERA_ENUM
{
	Unknown,
	FBX,
	GLTF, // not supported yet
	GLB   // not supported yet
};

enum class AssetResult VERA_ENUM
{
	Uninitialized,
	Success,
	FileNotFound,
	UnsupportedFormat,
	InvalidID,
	InvalidMagic,
	InvalidFormat,
	InvalidPadding,
	UnsupportedFeature,
	AllocationFailed,
	UnknownError
};

class AssetLoader
{
	friend struct FBXLoader;
	friend struct GLTFLoader;
	friend struct GLBLoader;
public:
	AssetLoader() VERA_NOEXCEPT;
	AssetLoader(std::string_view path) VERA_NOEXCEPT;
	~AssetLoader();

	AssetResult loadModel(std::string_view path) VERA_NOEXCEPT;

	VERA_NODISCARD obj<scene::Scene> getScene() VERA_NOEXCEPT;

	VERA_NODISCARD AssetResult getResult() const VERA_NOEXCEPT;
	VERA_NODISCARD std::string_view getErrorMessage() const VERA_NOEXCEPT;
	VERA_NODISCARD AssetFileFormat getFileFormat() const VERA_NOEXCEPT;
	VERA_NODISCARD Version getVersion() const VERA_NOEXCEPT;

private:
	obj<scene::Scene> m_scene;
	AssetResult       m_result;
	std::string       m_message;
	AssetFileFormat   m_file_format;
	Version           m_version;
};

VERA_NAMESPACE_END