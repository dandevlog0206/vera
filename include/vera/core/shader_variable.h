#include "texture.h"
#include "../math/vector_types.h"
#include "../math/matrix_types.h"
#include <string_view>

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

class ShaderVariable
{
	friend class ShaderStorage;
	ShaderVariable(ShaderParameterImpl* impl);
	ShaderVariable(
		ShaderParameterImpl*  impl,
		const ReflectionNode* node);
	ShaderVariable(
		ShaderParameterImpl*  impl,
		const ReflectionNode* node,
		uint32_t              array_idx);
	ShaderVariable(
		ShaderParameterImpl*  impl,
		const ReflectionNode* node,
		uint32_t              array_idx,
		uint32_t              offset);
public:
	ShaderVariable() = default;

	VERA_NODISCARD bool isRoot() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isPushConstant() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isDescriptor() const VERA_NOEXCEPT;
	VERA_NODISCARD DescriptorType getDescriptorType() const VERA_NOEXCEPT;

	VERA_NODISCARD ShaderVariable operator[](std::string_view name) VERA_NOEXCEPT;
	VERA_NODISCARD ShaderVariable operator[](uint32_t idx) VERA_NOEXCEPT;

	VERA_NODISCARD ShaderVariable at(std::string_view name);
	VERA_NODISCARD ShaderVariable at(uint32_t idx);

	void operator=(obj<Sampler> sampler);
	void operator=(obj<TextureView> texture_view);
	void operator=(obj<BufferView> buffer_view);
	void operator=(obj<Buffer> buffer);

	template <class T>
	// requires requires(ShaderVariable& s, const T& t) { s.setValue(t); }
	void operator=(const T& value)
	{
		setValue(value);
	}

	void setSampler(obj<Sampler> sampler);
	void setTextureView(
		obj<TextureView> texture_view,
		TextureLayout    layout = TextureLayout::Undefined);
	void setBufferView(obj<BufferView> buffer_view);
	void setBuffer(
		obj<Buffer> buffer,
		size_t      offset = 0,
		size_t      range  = 0);

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
	void setValue(const float2x2& value);
	void setValue(const float2x3& value);
	void setValue(const float2x4& value);
	void setValue(const float3x2& value);
	void setValue(const float3x3& value);
	void setValue(const float3x4& value);
	void setValue(const float4x2& value);
	void setValue(const float4x3& value);
	void setValue(const float4x4& value);
	void setValue(const double2x2& value);
	void setValue(const double2x3& value);
	void setValue(const double2x4& value);
	void setValue(const double3x2& value);
	void setValue(const double3x3& value);
	void setValue(const double3x4& value);
	void setValue(const double4x2& value);
	void setValue(const double4x3& value);
	void setValue(const double4x4& value);

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	ShaderParameterImpl*  m_impl;
	const ReflectionNode* m_node;
	uint32_t              m_array_idx;
	uint32_t              m_offset;
};

VERA_NAMESPACE_END