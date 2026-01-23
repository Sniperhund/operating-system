#include "bitmap.h"
#include "x86/memory/heap.h"
#include <string.h>
#include <error.h>
#include <limits.h>

#define BITS_PER_WORD (sizeof(uint32_t) * CHAR_BIT)
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

/*Bitmap::~Bitmap() {
    Heap::free(m_bitmap);
}*/

uint32_t Bitmap::init(size_t size) {
    m_bitmap = (uint32_t*)Heap::alloc(size / BITS_PER_WORD);

    if (!m_bitmap)
        return E_NOMEM;
    
    m_size = size;

    memset(m_bitmap, 0, m_size / BITS_PER_WORD);

    return 0;
}

void Bitmap::mark(size_t n) {
    m_bitmap[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void Bitmap::unmark(size_t n) {
    m_bitmap[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

bool Bitmap::isMarked(size_t n) {
    uint32_t bit = m_bitmap[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
    return bit != 0;
}
