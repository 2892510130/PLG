#include <iostream>
#include <memory_resource>

#include "arena_memory.hpp"

struct Vec3 {
    float x, y, z;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Widget {
    int id;
    char tag;
    Widget(int i, char t) : id(i), tag(t) {}
};

int main() {
    std::cout << alignof(std::max_align_t) << '\n';
    std::cout << ~(4 - 1) << ", " << ((24 + 4 - 1) & ~(4 - 1)) << '\n';

    ArenaBuffer arena_buffer(256);

    Vec3* my_vec3 = arena_buffer.create<Vec3>(1.0, 2.0, 3.0);
    std::cout << "Used " << arena_buffer.used() << '\n';

    Widget* my_widget = arena_buffer.create<Widget>(13, 'x');
    std::cout << "Used " << arena_buffer.used() << '\n';

    std::cout << my_vec3->x << ", " << my_widget->tag << '\n';

    // std::pmr::monotonic_buffer_resource
}