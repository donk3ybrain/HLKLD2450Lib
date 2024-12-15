#ifndef PSRAMALLOCATOR_H
#define PSRAMALLOCATOR_H

#include <esp_heap_caps.h>
#include <memory>

template <typename T>
class PSRAMAllocator {
public:
    using value_type = T;

    PSRAMAllocator() noexcept = default;

    template <typename U>
    constexpr PSRAMAllocator(const PSRAMAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        void* ptr = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM);
        if (!ptr) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t) noexcept {
        heap_caps_free(p);
    }
};

template <typename T, typename U>
bool operator==(const PSRAMAllocator<T>&, const PSRAMAllocator<U>&) { return true; }

template <typename T, typename U>
bool operator!=(const PSRAMAllocator<T>&, const PSRAMAllocator<U>&) { return false; }

#endif // PSRAMALLOCATOR_H
