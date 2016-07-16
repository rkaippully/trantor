/*
    Trantor Operating System

    Copyright (C) 2014 Raghu Kaippully
*/

#include "mm.h"
#include "ints.h"
#include "mutex.h"

/*
    Dynamic memory allocation for the kernel
*/

/*
    Kernel heap start and end points. KERNEL_HEAP_START is set up by vmm_init() in mm.c.
*/
void* const KERNEL_HEAP_START;
void* const KERNEL_HEAP_END = (void*)0xffc00000;

/*
    We use a simple memory allocation scheme.
*/

void* next_page_to_alloc = 0;

typedef struct kmalloc_header {
    uint32_t size;
    bool in_use;
    struct kmalloc_header *next, *prev;
} kmalloc_header;

kmalloc_header *mem_list_head = 0, *mem_list_tail = 0;

/*
    This function checks a block if it has any remining space for future allocations. If
    there is at least 32 bytes free, we split the block. Otherwise, we just return.

    Returns the newly created block in case of a split, otherwise returns the original
    block.
*/
static kmalloc_header* split_block_with_extra_space(kmalloc_header* const block,
    uint32_t size)
{
    int remaining = block->size - size;
    if (remaining - sizeof(kmalloc_header) >= 32) {
        // Set up a new block
        kmalloc_header* new_block = (void*)block + sizeof(kmalloc_header) + size;
        new_block->size = remaining - sizeof(kmalloc_header);
        new_block->in_use = false;
        new_block->prev = block;
        new_block->next = block->next;
        if (new_block->next != 0)
            new_block->next->prev = new_block;
        block->next = new_block;
        return new_block;
    } else {
        return block;
    }
}

static void* _kmalloc(uint32_t size)
{
    // Find a block in the list that will fit the size
    kmalloc_header* ptr = mem_list_head;
    while (ptr != 0) {
        if (!ptr->in_use && ptr->size >= size)
            break;
        ptr = ptr->next;
    }

    // Do we need to allocate more space?
    if (ptr == 0) {
        if (next_page_to_alloc == 0)
            next_page_to_alloc = KERNEL_HEAP_START;

        // Did we run out of address space?
        if (next_page_to_alloc + size + sizeof(kmalloc_header) >= KERNEL_HEAP_END)
            return 0;

        // The memory block will be located here
        kmalloc_header* block = next_page_to_alloc;

        // Allocate as many pages as needed
        for (uint32_t s = 0; s < size + sizeof(kmalloc_header); s += PAGE_SIZE) {
            alloc_virt_page(next_page_to_alloc);
            next_page_to_alloc += PAGE_SIZE;
        }

        if (mem_list_head == 0) {
            mem_list_head = block;
            mem_list_head->prev = 0;
        }
        if (mem_list_tail != 0) {
            mem_list_tail -> next = block;
            block->prev = mem_list_tail;
        }
        mem_list_tail = block;
        block->next = 0;

        // Mark the block in use and set the size
        block->in_use = true;
        block->size = size;

        /*
            Check if any remaining space in the last allocated page is enough for another
            block. We create another block only if we have at least 32 bytes free
        */
        kmalloc_header* last_block = split_block_with_extra_space(block, size);
        mem_list_tail = last_block;

        return (void*)block + sizeof(kmalloc_header);
    } else {
        // We found a block with enough space at ptr. Split it if needed
        split_block_with_extra_space(ptr, size);

        return (void*)ptr + sizeof(kmalloc_header);
    }
}

static mutex_t kmalloc_mutex;

void* kmalloc(uint32_t size)
{
    acquire_mutex(&kmalloc_mutex);
    void* addr = _kmalloc(size);
    release_mutex(&kmalloc_mutex);
    return addr;
}

void kfree(void* ptr)
{
    acquire_mutex(&kmalloc_mutex);

    kmalloc_header* block = ptr - sizeof(kmalloc_header);
    block->in_use = false;

    // Can we merge the block with prev and/or next?
    kmalloc_header* prev = block->prev;
    kmalloc_header* next = block->next;
    if (prev != 0 && prev->in_use == false) {
        prev->size += block->size + sizeof(kmalloc_header);
        prev->next = next;
        if (next != 0)
            next->prev = prev;
        else
            mem_list_tail = prev;
        block = prev;
    }
    if (next != 0 && next->in_use == false) {
        block->size += next->size + sizeof(kmalloc_header);
        block->next = next->next;
        if (next->next != 0)
            next->next->prev = block;
        else
            mem_list_tail = block;
    }

    release_mutex(&kmalloc_mutex);
}
