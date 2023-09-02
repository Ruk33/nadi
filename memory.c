#include <assert.h> // assert
#include <stdlib.h> // malloc
#include <string.h> // memset
#include "memory.h"

static void *reserved_memory = 0;
static unsigned long long reserved_memory_size = 0;

void memory_reserve(unsigned long long how_much)
{
    reserved_memory = malloc(how_much);
    assert(reserved_memory);
    reserved_memory_size = how_much;
}

void *memory_get(unsigned long long how_much)
{
    static unsigned long long tail = 0;
    assert(reserved_memory);
    assert(reserved_memory_size >= how_much);
    if (tail + how_much > reserved_memory_size)
        tail = 0;
    memset(reserved_memory + tail, 0, how_much);
    tail += how_much;
    return reserved_memory + (tail - how_much);
}
