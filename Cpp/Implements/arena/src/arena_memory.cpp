#include "arena_memory.hpp"

void* ArenaBuffer::allocate(std::size_t size, std::size_t alignment) {
    assert((alignment & (alignment - 1)) == 0 && "alignment must be power of two");
    assert(alignment != 0);

    std::size_t aligned = (m_offset + alignment - 1) & ~(alignment - 1);

    if (aligned + size > m_buffer.size()) {
        return nullptr;
    }

    m_offset = aligned + size;
    return m_buffer.data() + aligned;
}

void ArenaBuffer::run_destructors_to(std::size_t m) {
    while (!m_destructors.empty() && m_destructors.back().ptr >= m_buffer.data() + m) {
        m_destructors.back().fn(m_destructors.back().ptr);
        m_destructors.pop_back();
    }
}