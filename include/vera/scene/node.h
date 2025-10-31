#pragma once

#include "attribute.h"
#include "../graphics/transform3d.h"
#include <string_view>
#include <vector>
#include <string>
#include <map>

VERA_NAMESPACE_BEGIN
VERA_SCENE_NAMESPACE_BEGIN

class Node : public ManagedObject
{
	Node(std::string_view name);
public:
	static obj<Node> create(std::string_view name);
	~Node();

	VERA_NODISCARD const std::string& getName() const VERA_NOEXCEPT;

	void addChild(obj<Node> node);
	ref<Node> emplaceChild(std::string_view name);
	obj<Node> removeChild(std::string_view name);
	VERA_NODISCARD size_t getChildCount() const VERA_NOEXCEPT;
	VERA_NODISCARD bool hasChild(std::string_view name) const VERA_NOEXCEPT;
	VERA_NODISCARD ref<Node> getFirstChild() VERA_NOEXCEPT;
	VERA_NODISCARD ref<Node> getChild(std::string_view name) VERA_NOEXCEPT;
	VERA_NODISCARD ref<Node> getSibling(std::string_view name) VERA_NOEXCEPT;
	VERA_NODISCARD ref<Node> getNextSibling(std::string_view name = {}) VERA_NOEXCEPT;
	VERA_NODISCARD ref<Node> getParent() VERA_NOEXCEPT;

	void addAttribute(obj<Attribute> attribute);
	VERA_NODISCARD size_t getAttributeCount() const VERA_NOEXCEPT;
	VERA_NODISCARD ref<Attribute> getAttribute(size_t index) VERA_NOEXCEPT;

	VERA_NODISCARD const TransformDesc3D& getTransform() const VERA_NOEXCEPT;
	void setTransform(const TransformDesc3D& desc) VERA_NOEXCEPT;
	VERA_NODISCARD const float4x4& getTransformMatrix() const VERA_NOEXCEPT;

private:
	// use std::map to ensure the order of childs
	using ChildMap = std::map<std::string_view, obj<Node>>;

	std::string                 m_name;
	ChildMap                    m_childs;
	ChildMap::iterator          m_my_iter;
	ref<Node>                   m_parent;
	std::vector<obj<Attribute>> m_attributes;
	TransformDesc3D             m_transform_desc;
	float4x4                    m_transform;
};

VERA_SCENE_NAMESPACE_END
VERA_NAMESPACE_END
