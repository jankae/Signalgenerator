#include "FreeRTOS.h"
#include <cstdio>

#include "logging.h"

void * operator new(size_t size)
{
//	printf("New: allocating %d bytes\n", size);
	return pvPortMalloc(size);
}

void * operator new[](size_t size)
{
//	printf("New: allocating %d bytes\n", size);
	return pvPortMalloc(size);
}

void operator delete(void* ptr)
{
//	printf("Delete: freeing pointer: %p\n", ptr);
    vPortFree(ptr);
}

void operator delete[](void* ptr)
{
//	printf("Delete: freeing pointer: %p\n", ptr);
    vPortFree(ptr);
}

extern "C" void __cxa_pure_virtual() {
	CRIT_ERROR("Pure virtual");
	while (1);
}

