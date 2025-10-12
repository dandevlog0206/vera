#include "../core/coredefs.h"

#include <type_traits>
#include <vector>
#include <cstdlib>

#ifdef _MSC_VER
#define aligned_alloc(alignment, size) _aligned_malloc(size, alignment)
#define aligned_free(ptr)              _aligned_free(ptr)
#else
#define aligned_free(ptr) free(ptr)
#endif

VERA_NAMESPACE_BEGIN

class StackAllocator
{
public:
	using size_type          = size_t;
	using difference_type    = std::ptrdiff_t;

	VERA_INLINE StackAllocator() VERA_NOEXCEPT :
		m_pages(),
		m_page_size(512),
		m_current_page(-1),
		m_offset() {}

	VERA_INLINE StackAllocator(size_type page_size) VERA_NOEXCEPT :
		m_pages(),
		m_page_size(page_size),
		m_current_page(-1),
		m_offset() {
		VERA_ASSERT_MSG(8 <= page_size && (page_size & (page_size - 1)) == 0,
			"page size must be power of 2 greater or equal than 8");
	}

	VERA_INLINE ~StackAllocator()
	{
		for (auto& page : m_pages)
			aligned_free(page.data);
	}

	template <class T>
	VERA_NODISCARD VERA_INLINE T* allocate(size_t elem_count = 1)
	{
		static_assert(std::is_standard_layout_v<T> || std::is_trivial_v<T>,
			"cannot allocate non-pod type");

		VERA_ASSERT_MSG(elem_count != 0, "cannot allocate with zero element count");

		return reinterpret_cast<T*>(allocateImpl(elem_count * sizeof(T), alignof(T)));
	}

	VERA_INLINE void clear(size_t offset = 0) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(offset < m_offset, "desired offset must be smaller than current offset");
		
		if (offset == 0) {
			for (auto& page : m_pages)
				aligned_free(page.data);
			
			m_pages.clear();
			m_current_page = -1;
			m_offset       = 0;
		} else {
			size_t current_offset = 0;

			for (size_t i = 0; i < m_pages.size(); ++i) {
				Page& target_page = m_pages[i];

				current_offset += target_page.allocated;
			
				if (offset <= current_offset) {
					for (size_t j = i + 1; j < m_pages.size(); ++j)
						_aligned_free(m_pages[j].data);
					m_pages.resize(i + 1);

					target_page.used = target_page.allocated - (current_offset - offset);
					
					m_current_page = i;
					m_offset       = offset;
					break;
				}
			}
		}
	}

	VERA_NODISCARD VERA_INLINE size_t offset() const VERA_NOEXCEPT
	{
		return m_offset;
	}

	VERA_NODISCARD VERA_INLINE size_t sizeInUse() const VERA_NOEXCEPT
	{
		size_t used = 0;

		for (const auto& page : m_pages)
			used += page.used;

		return used;
	}

	VERA_NODISCARD VERA_INLINE size_t totalSize() const VERA_NOEXCEPT
	{
		size_t allocated = 0;

		for (const auto& page : m_pages)
			allocated += page.allocated;

		return allocated;
	}

	VERA_NODISCARD VERA_INLINE float usageRatio() const VERA_NOEXCEPT
	{
		size_t used      = 0;
		size_t allocated = 0;
		
		for (const auto& page : m_pages) {
			used      += page.used;
			allocated += page.allocated;
		}

		return static_cast<float>(used) / static_cast<float>(allocated);
	}

private:
	void* allocateImpl(size_t size, size_t alignment)
	{
		VERA_ASSERT_MSG(std::popcount(alignment), "alignment must be power of 2");

		if (m_current_page == -1) {
			Page& new_page = m_pages.emplace_back();
			new_page.beginOffset = 0;
			new_page.used        = size;
			new_page.pageCount   = (size - 1) / m_page_size + 1;
			new_page.allocated   = new_page.pageCount * m_page_size;
			new_page.data        = aligned_alloc(std::max(alignment, 8ull), new_page.allocated);

			m_current_page = 0;
			m_offset       = size;

			return new_page.data;
		} else {
			Page&  last_page   = m_pages[m_current_page];
			size_t last_ptr    = reinterpret_cast<size_t>(last_page.data) + last_page.used;
			size_t padding     = alignment - last_ptr % alignment;
			size_t padded_size = size + (padding == alignment ? 0 : padding);

			if (last_page.used + padded_size < last_page.allocated) {
				last_page.used += padded_size;
				m_offset       += padded_size;

				return reinterpret_cast<void*>(last_ptr);
			}

			size_t begin_offset  = last_page.beginOffset + last_page.allocated;
			size_t new_alignment = std::max(alignment, 8ull);
			size_t page_count    = (size - 1) / m_page_size + 1;

			VERA_ASSERT((page_count * m_page_size) % new_alignment == 0);

			Page& new_page = m_pages.emplace_back();
			new_page.beginOffset = begin_offset;
			new_page.used        = size;
			new_page.pageCount   = page_count;
			new_page.allocated   = page_count * m_page_size;
			new_page.data        = aligned_alloc(new_alignment, new_page.allocated);

			m_current_page += 1;
			m_offset       += new_page.beginOffset + size;

			return new_page.data;
		}
	}

private:
	struct Page
	{
		size_t beginOffset;
		size_t used;
		size_t pageCount;
		size_t allocated;
		void*  data;
	};

	std::vector<Page> m_pages;
	size_t            m_page_size;
	int64_t           m_current_page;
	size_t            m_offset;
};

VERA_NAMESPACE_END