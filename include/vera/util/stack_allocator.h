#include "../core/coredefs.h"

#include <vector>

VERA_NAMESPACE_BEGIN

template <class T>
class stack_allocator
{
public:
public:
	stack_allocator() = default;
	~stack_allocator() = default;

public:
	using value_type         = T;
	using pointer            = T*;
	using const_pointer      = const T*;
	using void_pointer       = void*;
	using const_void_pointer = const void*;
	using size_type          = size_t;
	using difference_type    = std::ptrdiff_t;

	pointer allocate(size_type ObjectNum, const_void_pointer hint)
	{
		allocate(ObjectNum);
	}

	pointer allocate(size_type ObjectNum)
	{
		return static_cast<pointer>(operator new(sizeof(T) * ObjectNum));
	}

	void deallocate(pointer p, size_type ObjectNum)
	{
		operator delete(p);
	}

	size_type max_size() const
	{
		return std::numeric_limits<size_type>::max() / sizeof(value_type);
	}

	template<typename U, typename... Args>
	void construct(U* p, Args&& ...args) {
		new(p) U(std::forward<Args>(args)...);
	}

	template <typename U>
	void destroy(U* p) {
		p->~U();
	}
};

VERA_NAMESPACE_END