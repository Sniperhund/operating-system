#pragma once

#include <stddef.h>
#include <stdint.h>

class Bitmap {
public:
    // Deconstructors can't be used since it's freestanding (atleast until I get support for it)
    //~Bitmap();

    uint32_t init(size_t size);

    void mark(size_t n);
    void unmark(size_t n);
    bool isMarked(size_t n);

private:
    uint32_t* m_bitmap = nullptr;
    uint32_t m_size = 0;
};