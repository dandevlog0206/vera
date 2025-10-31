#pragma once

#include "../core/intrusive_ptr.h"

VERA_NAMESPACE_BEGIN
VERA_SCENE_NAMESPACE_BEGIN

enum class AttributeType VERA_ENUM
{
	Unknown,
	Camera,
	Light,
	Skeleton,
	Animation,
	Mesh,
};

class Attribute : public ManagedObject
{
public:
	virtual ~Attribute() {};

	VERA_NODISCARD virtual AttributeType getType() const VERA_NOEXCEPT = 0;
};

VERA_SCENE_NAMESPACE_END
VERA_NAMESPACE_END
