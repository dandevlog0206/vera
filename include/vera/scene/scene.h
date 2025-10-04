#pragma once

#include "axis_system.h"
#include "node.h"

VERA_NAMESPACE_BEGIN
VERA_SCENE_NAMESPACE_BEGIN

class SceneSettings
{
	friend class Scene;
public:
	SceneSettings() = default;
	~SceneSettings() = default;

	const AxisSystem& getAxisSystem() const;

private:
	AxisSystem m_axis_system;
};

class Scene : public ManangedObject
{
	Scene(std::string_view name);
public:
	static obj<Scene> create(std::string_view name);
	~Scene();

	VERA_NODISCARD ref<Node> getRootNode();

	const SceneSettings& getGlobalSettings() const;

private:
	obj<Node>           m_root_node;
	SceneSettings       m_settings;
};

VERA_SCENE_NAMESPACE_END
VERA_NAMESPACE_END