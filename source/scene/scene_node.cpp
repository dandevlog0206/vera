#include "../../include/vera/scene/node.h"

#include "../../include/vera/core/exception.h"

VERA_NAMESPACE_BEGIN
VERA_SCENE_NAMESPACE_BEGIN

Node::Node(std::string_view name) :
	m_name(name) {}

obj<Node> Node::create(std::string_view name)
{
	return obj<Node>(new Node(name));
}

Node::~Node()
{
	for (auto& [name, child] : m_childs) {
		child->m_parent = nullptr;
	}
	m_childs.clear();
	m_parent = nullptr;
}

VERA_NODISCARD const std::string& Node::getName() const VERA_NOEXCEPT
{
	return m_name;
}

void Node::addChild(obj<Node> node)
{
	if (!node)
		throw Exception("vr::scene::Node::addChild: node is null");
	if (node->m_parent)
		throw Exception("vr::scene::Node::addChild: node already has a parent");

	auto& node_name       = node->m_name;
	auto [iter, inserted] = m_childs.emplace(node_name, std::move(node));

	if (!inserted)
		throw Exception("vr::scene::Node::addChild: node with name '{}' already exists", node_name);

	iter->second->m_parent  = this;
	iter->second->m_my_iter = iter;
}

ref<Node> Node::emplaceChild(std::string_view name)
{
	auto new_node = Node::create(name);
	
	addChild(new_node);

	return new_node;
}

obj<Node> Node::removeChild(std::string_view name)
{
	auto iter = m_childs.find(name);

	if (iter == m_childs.cend())
		throw Exception("vr::scene::Node::removeChild: node with name '{}' not found", name);

	auto obj = std::move(iter->second);

	m_childs.erase(iter);

	obj->m_parent  = nullptr;
	obj->m_my_iter = {};

	return obj;
}

VERA_NODISCARD size_t Node::getChildCount() const VERA_NOEXCEPT
{
	return m_childs.size();
}

VERA_NODISCARD bool Node::hasChild(std::string_view name) const VERA_NOEXCEPT
{
	return m_childs.find(name) != m_childs.cend();
}

VERA_NODISCARD ref<Node> Node::getFirstChild() VERA_NOEXCEPT
{
	if (m_childs.empty())
		return ref<Node>();

	return m_childs.begin()->second;
}

VERA_NODISCARD ref<Node> Node::getChild(std::string_view name) VERA_NOEXCEPT
{
	auto iter = m_childs.find(name);

	if (iter == m_childs.cend())
		return ref<Node>();
	
	return iter->second;
}

VERA_NODISCARD ref<Node> Node::getSibling(std::string_view name) VERA_NOEXCEPT
{
	if (!m_parent)
		return ref<Node>();
	
	return m_parent->getChild(name);
}

VERA_NODISCARD ref<Node> Node::getNextSibling(std::string_view name) VERA_NOEXCEPT
{
	if (!m_parent)
		return ref<Node>();

	ChildMap::iterator next;

	if (name.empty())
		next = std::next(m_my_iter);
	else
		next = m_parent->m_childs.find(name);

	return next == m_parent->m_childs.cend() ? ref<Node>() : next->second;
}

VERA_NODISCARD ref<Node> Node::getParent() VERA_NOEXCEPT
{
	return m_parent ? m_parent : ref<Node>();
}

void Node::addAttribute(obj<Attribute> attribute)
{
	if (!attribute)
		throw Exception("vr::scene::Node::addAttribute: attribute is null");
	
	m_attributes.push_back(std::move(attribute));
}

VERA_NODISCARD size_t Node::getAttributeCount() const VERA_NOEXCEPT
{
	return m_attributes.size();
}

VERA_NODISCARD ref<Attribute> Node::getAttribute(size_t index) VERA_NOEXCEPT
{
	if (index >= m_attributes.size())
		return ref<Attribute>();
	
	return m_attributes[index];
}

VERA_NODISCARD const TransformDesc3D& Node::getTransform() const VERA_NOEXCEPT
{
	return m_transform_desc;
}

void Node::setTransform(const TransformDesc3D& desc) VERA_NOEXCEPT
{
	m_transform_desc = desc;
	m_transform      = Transform3D(desc).getMatrix();
}

VERA_NODISCARD const float4x4& Node::getTransformMatrix() const VERA_NOEXCEPT
{
	return m_transform;
}

VERA_SCENE_NAMESPACE_END
VERA_NAMESPACE_END
