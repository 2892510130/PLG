#include <iostream>
#include <cstddef>
#include <cassert>

class MyAllocator {
private:
    struct Block {
        std::size_t size;
        bool free;
        Block* next;
    };

    static constexpr std::size_t POOL_SIZE = 1024 * 1024;

    alignas(std::max_align_t) char pool[POOL_SIZE];

    Block* head;

public:
    MyAllocator() {
        head = reinterpret_cast<Block*>(pool);
        head->size = POOL_SIZE - sizeof(Block);
        head->free = true;
        head->next = nullptr;
    }

    void* allocate(std::size_t size) {
        if (size == 0) return nullptr;

        // Align requested size
        constexpr std::size_t alignment = alignof(std::max_align_t);
        size = (size + alignment - 1) & ~(alignment - 1);

        Block* current = head;

        // First-fit search
        while (current != nullptr) {
            if (current->free && current->size >= size) {

                // Split block if enough space remains
                if (current->size >= size + sizeof(Block) + alignment) {
                    Block* newBlock =
                        reinterpret_cast<Block*>(
                            reinterpret_cast<char*>(current + 1) + size
                        );

                    newBlock->size =
                        current->size - size - sizeof(Block);
                    newBlock->free = true;
                    newBlock->next = current->next;

                    current->next = newBlock;
                    current->size = size;
                }

                current->free = false;
                return current + 1;
            }

            current = current->next;
        }

        return nullptr; // Out of memory
    }

    void deallocate(void* ptr) {
        if (ptr == nullptr) return;

        Block* block = static_cast<Block*>(ptr) - 1;
        block->free = true;

        // Merge adjacent free blocks
        Block* current = head;

        while (current != nullptr && current->next != nullptr) {
            if (current->free && current->next->free) {
                current->size += sizeof(Block) + current->next->size;
                current->next = current->next->next;
            } else {
                current = current->next;
            }
        }
    }
};

int main() {
    MyAllocator allocator;

    int* a = static_cast<int*>(allocator.allocate(sizeof(int)));
    int* b = static_cast<int*>(allocator.allocate(sizeof(int)));

    if (a && b) {
        *a = 10;
        *b = 20;

        std::cout << *a << " " << *b << std::endl;
    }

    allocator.deallocate(a);
    allocator.deallocate(b);

    return 0;
}