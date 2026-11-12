#include <vector>
#include <cstddef>
#include <new>
#include <cassert>

using DestructorFn = void(*)(void*);

struct DestructRecord {
    void* ptr;
    DestructorFn fn;
};

class ArenaBuffer {
public:
    ArenaBuffer(std::size_t capacity) { m_buffer.resize(capacity); };
    ~ArenaBuffer() { run_destructors_to(0); }

    void reset() { m_offset = 0; };
    void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t));
    std::size_t used() const { return m_offset; }
    std::size_t remaining() const { return m_buffer.size() - m_offset; }
    void run_destructors_to(std::size_t m);

    template <typename T, typename... Args>
    T* create(Args&&... args) {
        void* p = allocate(sizeof(T), alignof(T));
        if (!p) return nullptr;
        T* obj = new (p) T(std::forward<Args>(args)...);
        if constexpr (!std::is_trivially_destructible_v<T>) {
            m_destructors.push_back({p, [](void* q) { static_cast<T*>(q)->~T(); }});
        }
        return obj;
    }

    std::size_t mark() const { return m_offset; }

    void release_to(std::size_t m) {
        assert(m <= m_offset && "mark is ahead of current offset");
        m_offset = m;
    }

    template <typename T>
    T* alloc() {
        return static_cast<T*>(allocate(sizeof(T), alignof(T)));
    }

    template <typename T>
    T* alloc_array(std::size_t n) {
        static_assert(std::is_trivially_destructible_v<T>,
              "alloc_array: non-trivial T needs explicit construction/destruction");
        return static_cast<T*>(allocate(sizeof(T) * n, alignof(T)));
    }

private:
    std::vector<std::byte> m_buffer;
    std::vector<DestructRecord> m_destructors;
    std::size_t m_offset = 0;
};

class ScopedArena {
public:
    explicit ScopedArena(ArenaBuffer& a) : m_arena(a), m_mark(a.mark()) {}
    ~ScopedArena() { m_arena.release_to(m_mark); }
    ScopedArena(const ScopedArena&) = delete;
    ScopedArena& operator=(const ScopedArena&) = delete;
private:
    ArenaBuffer& m_arena;
    std::size_t m_mark;
};