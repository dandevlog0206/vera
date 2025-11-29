#include "texture.h"
#include "../math/vector_types.h"
#include "../math/matrix_types.h"
#include <string_view>
#include <concepts> // Add this include

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class TextureLayout VERA_ENUM;

class Sampler;
class Texture;
class TextureView;
class Buffer;
class BufferView;
class ReflectionNode;
class ShaderParameterImpl;
class ShaderParameterBlockStorage;

// Define a concept that enumerates all valid types for setValue
template<class T>
concept IsShaderPrimitiveValueType =
	std::same_as<T, bool> ||
	std::same_as<T, int8_t> || std::same_as<T, uint8_t> ||
	std::same_as<T, int16_t> || std::same_as<T, uint16_t> ||
	std::same_as<T, int32_t> || std::same_as<T, uint32_t> ||
	std::same_as<T, int64_t> || std::same_as<T, uint64_t> ||
	std::same_as<T, float> || std::same_as<T, double> ||
	std::same_as<T, bool2> || std::same_as<T, bool3> || std::same_as<T, bool4> ||
	std::same_as<T, char2> || std::same_as<T, char3> || std::same_as<T, char4> ||
	std::same_as<T, uchar2> || std::same_as<T, uchar3> || std::same_as<T, uchar4> ||
	std::same_as<T, short2> || std::same_as<T, short3> || std::same_as<T, short4> ||
	std::same_as<T, ushort2> || std::same_as<T, ushort3> || std::same_as<T, ushort4> ||
	std::same_as<T, int2> || std::same_as<T, int3> || std::same_as<T, int4> ||
	std::same_as<T, uint2> || std::same_as<T, uint3> || std::same_as<T, uint4> ||
	std::same_as<T, long2> || std::same_as<T, long3> || std::same_as<T, long4> ||
	std::same_as<T, ulong2> || std::same_as<T, ulong3> || std::same_as<T, ulong4> ||
	std::same_as<T, float2> || std::same_as<T, float3> || std::same_as<T, float4> ||
	std::same_as<T, double2> || std::same_as<T, double3> || std::same_as<T, double4> ||
	std::same_as<T, rfloat2x2> || std::same_as<T, rfloat2x3> || std::same_as<T, rfloat2x4> ||
	std::same_as<T, rfloat3x2> || std::same_as<T, rfloat3x3> || std::same_as<T, rfloat3x4> ||
	std::same_as<T, rfloat4x2> || std::same_as<T, rfloat4x3> || std::same_as<T, rfloat4x4> ||
	std::same_as<T, rdouble2x2> || std::same_as<T, rdouble2x3> || std::same_as<T, rdouble2x4> ||
	std::same_as<T, rdouble3x2> || std::same_as<T, rdouble3x3> || std::same_as<T, rdouble3x4> ||
	std::same_as<T, rdouble4x2> || std::same_as<T, rdouble4x3> || std::same_as<T, rdouble4x4> ||
	std::same_as<T, cfloat2x2> || std::same_as<T, cfloat2x3> || std::same_as<T, cfloat2x4> ||
	std::same_as<T, cfloat3x2> || std::same_as<T, cfloat3x3> || std::same_as<T, cfloat3x4> ||
	std::same_as<T, cfloat4x2> || std::same_as<T, cfloat4x3> || std::same_as<T, cfloat4x4> ||
	std::same_as<T, cdouble2x2> || std::same_as<T, cdouble2x3> || std::same_as<T, cdouble2x4> ||
	std::same_as<T, cdouble3x2> || std::same_as<T, cdouble3x3> || std::same_as<T, cdouble3x4> ||
	std::same_as<T, cdouble4x2> || std::same_as<T, cdouble4x3> || std::same_as<T, cdouble4x4>;

class ShaderVariable
{
	friend class ShaderParameter;
	ShaderVariable(
		ShaderParameterImpl*         impl,
		const ReflectionNode*        node,
		ShaderParameterBlockStorage* block,
		uint32_t                     array_idx,
		uint32_t                     offset);
public:
	ShaderVariable() = default;

	VERA_NODISCARD bool isRoot() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isArray() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isStruct() const VERA_NOEXCEPT; // includes block

	VERA_NODISCARD ShaderVariable at(std::string_view name) const;
	VERA_NODISCARD ShaderVariable at(uint32_t idx) const;

	VERA_NODISCARD ShaderVariable operator[](std::string_view name) const VERA_NOEXCEPT;
	VERA_NODISCARD ShaderVariable operator[](uint32_t idx) const VERA_NOEXCEPT;

	void setSampler(obj<Sampler> sampler);
	void setTextureView(obj<TextureView> texture_view);
	void setBufferView(obj<BufferView> buffer_view);
	void setBuffer(
		obj<Buffer> buffer,
		size_t      offset = 0,
		size_t      range  = 0);

	void operator=(obj<Sampler> sampler) { setSampler(std::move(sampler)); }
	void operator=(obj<TextureView> texture_view) { setTextureView(std::move(texture_view)); }
	void operator=(obj<BufferView> buffer_view) { setBufferView(std::move(buffer_view)); }
	void operator=(obj<Buffer> buffer) { setBuffer(std::move(buffer)); }

	// primitive types
	void setValue(const bool value);
	void setValue(const int8_t value);
	void setValue(const uint8_t value);
	void setValue(const int16_t value);
	void setValue(const uint16_t value);
	void setValue(const int32_t value);
	void setValue(const uint32_t value);
	void setValue(const int64_t value);
	void setValue(const uint64_t value);
	void setValue(const float value);
	void setValue(const double value);

	// vector types
	void setValue(const bool2& value);
	void setValue(const bool3& value);
	void setValue(const bool4& value);
	void setValue(const char2& value);
	void setValue(const char3& value);
	void setValue(const char4& value);
	void setValue(const uchar2& value);
	void setValue(const uchar3& value);
	void setValue(const uchar4& value);
	void setValue(const short2& value);
	void setValue(const short3& value);
	void setValue(const short4& value);
	void setValue(const ushort2& value);
	void setValue(const ushort3& value);
	void setValue(const ushort4& value);
	void setValue(const int2& value);
	void setValue(const int3& value);
	void setValue(const int4& value);
	void setValue(const uint2& value);
	void setValue(const uint3& value);
	void setValue(const uint4& value);
	void setValue(const long2& value);
	void setValue(const long3& value);
	void setValue(const long4& value);
	void setValue(const ulong2& value);
	void setValue(const ulong3& value);
	void setValue(const ulong4& value);
	void setValue(const float2& value);
	void setValue(const float3& value);
	void setValue(const float4& value);
	void setValue(const double2& value);
	void setValue(const double3& value);
	void setValue(const double4& value);

	// matrix types
	void setValue(const rfloat2x2& value);
	void setValue(const rfloat2x3& value);
	void setValue(const rfloat2x4& value);
	void setValue(const rfloat3x2& value);
	void setValue(const rfloat3x3& value);
	void setValue(const rfloat3x4& value);
	void setValue(const rfloat4x2& value);
	void setValue(const rfloat4x3& value);
	void setValue(const rfloat4x4& value);
	void setValue(const rdouble2x2& value);
	void setValue(const rdouble2x3& value);
	void setValue(const rdouble2x4& value);
	void setValue(const rdouble3x2& value);
	void setValue(const rdouble3x3& value);
	void setValue(const rdouble3x4& value);
	void setValue(const rdouble4x2& value);
	void setValue(const rdouble4x3& value);
	void setValue(const rdouble4x4& value);

	void setValue(const cfloat2x2& value);
	void setValue(const cfloat2x3& value);
	void setValue(const cfloat2x4& value);
	void setValue(const cfloat3x2& value);
	void setValue(const cfloat3x3& value);
	void setValue(const cfloat3x4& value);
	void setValue(const cfloat4x2& value);
	void setValue(const cfloat4x3& value);
	void setValue(const cfloat4x4& value);
	void setValue(const cdouble2x2& value);
	void setValue(const cdouble2x3& value);
	void setValue(const cdouble2x4& value);
	void setValue(const cdouble3x2& value);
	void setValue(const cdouble3x3& value);
	void setValue(const cdouble3x4& value);
	void setValue(const cdouble4x2& value);
	void setValue(const cdouble4x3& value);
	void setValue(const cdouble4x4& value);

	template <IsShaderPrimitiveValueType T>
	void operator=(const T& value)
	{
		setValue(value);
	}

	void setBindless(bool enable);
	VERA_NODISCARD bool isBindless() const VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	ShaderParameterImpl*         m_impl;
	const ReflectionNode*        m_node;
	ShaderParameterBlockStorage* m_block;
	uint32_t                     m_array_idx;
	uint32_t                     m_offset;
};

VERA_NAMESPACE_END