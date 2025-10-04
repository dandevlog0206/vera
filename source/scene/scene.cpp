#include "../../include/vera/scene/scene.h"

VERA_NAMESPACE_BEGIN
VERA_SCENE_NAMESPACE_BEGIN

const AxisSystem& SceneSettings::getAxisSystem() const
{
	return m_axis_system;
}

Scene::Scene(std::string_view name)
{
	m_root_node = Node::create(name);

}

obj<Scene> Scene::create(std::string_view name)
{
	return obj<Scene>(new Scene(name));
}

Scene::~Scene()
{
}

VERA_NODISCARD ref<Node> Scene::getRootNode()
{
	return m_root_node;
}

const SceneSettings& Scene::getGlobalSettings() const
{
	return m_settings;
}

VERA_SCENE_NAMESPACE_END
VERA_NAMESPACE_END
