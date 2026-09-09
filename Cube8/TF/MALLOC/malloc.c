#include "malloc.h"	    


// Memory pool (4-byte aligned)
__attribute__((aligned(4))) u8 membase[MEM_MAX_SIZE]; // SRAM memory pool
// Memory usage map
u16 memmapbase[MEM_ALLOC_TABLE_SIZE];				// SRAM memory map
// Memory information
const u32 memtblsize = MEM_ALLOC_TABLE_SIZE;		// Memory table size
const u32 memblksize = MEM_BLOCK_SIZE;				// Memory block size
const u32 memsize = MEM_MAX_SIZE;					// Total memory size


// Memory management structure
struct _m_mallco_dev mallco_dev =
{
	mem_init,			// Memory initialization
	mem_perused,		// Memory usage percentage
	membase,			// Memory base address
	memmapbase,			// Memory map status
	0,  				// Memory initialization status
};

// Copy memory
//*des: destination address
//*src: source address
//n: length of memory to be copied (in bytes)
void mymemcpy(void *des, void *src, u32 n)
{
	u8 *xdes = des;
	u8 *xsrc = src;
	while (n--)*xdes++ = *xsrc++;
}

// Set memory
//*s: start address of memory
//c: value to be set
//count: size of memory to be set (in bytes)
void mymemset(void *s, u8 c, u32 count)
{
	u8 *xs = s;
	while (count--)*xs++ = c;
}

// Initialize memory pool
void mem_init(void)
{
	mymemset(mallco_dev.memmap, 0, memtblsize * 2); // Initialize memory status map to 0
	mymemset(mallco_dev.membase, 0, memsize);	// Initialize memory pool to 0
	mallco_dev.memrdy = 1;						// Memory initialization is complete
}

// Get memory usage percentage
// return value: usage percentage (0-100)
u8 mem_perused(void)
{
	u32 used = 0;
	u32 i;
	for (i = 0;i < memtblsize;i++)
	{
		if (mallco_dev.memmap[i])used++;
	}
	return (used * 100) / (memtblsize);
}

// Allocate memory (internal function)
// memx: memory management structure
// size: size of memory to allocate (bytes)
// return value: 0XFFFFFFFF for failure, otherwise, memory offset address
u32 mem_malloc(u32 size)
{
	signed long offset = 0;
	u16 nmemb;	// required number of blocks
	u16 cmemb = 0; // contiguous memory blocks found
	u32 i;
	if (!mallco_dev.memrdy)
		mallco_dev.init();							// not initialized, perform initialization
	if (size == 0)
		return 0XFFFFFFFF;							// no allocation needed
	nmemb = size / memblksize;  					// get number of required memory blocks
	if (size % memblksize)
		nmemb++;
	for (offset = memtblsize - 1;offset >= 0;offset--)	// Iterate through the memory map
	{
		if (!mallco_dev.memmap[offset])
			cmemb++;								// Count contiguous free blocks
		else 
			cmemb = 0;								// Reset count if block is used
		if (cmemb == nmemb)							// Found enough contiguous blocks
		{
			for (i = 0;i < nmemb;i++)  				// Mark blocks as used
			{
				mallco_dev.memmap[offset + i] = nmemb;
			}
			return (offset * memblksize);			// Return offset address
		}
	}
	return 0XFFFFFFFF;								// No suitable memory found
}

// Free memory (internal function)
// offset: memory address offset
// return value: 0 for success, 1 for failure, 2 for out of range
u8 mem_free(u32 offset)
{
	int i;
	if (!mallco_dev.memrdy)							// not initialized, perform initialization
	{
		mallco_dev.init();
		return 1;									// not initialized
	}
	if (offset < memsize)							// Check if offset is within memory range
	{
		int index = offset / memblksize;			// Get memory block index
		int nmemb = mallco_dev.memmap[index];		// Get number of blocks
		for (i = 0;i < nmemb;i++)  					// Clear memory map entries
		{
			mallco_dev.memmap[index + i] = 0;
		}
		return 0;
	}
	else 
		return 2;									// Offset is out of range
}

// Free memory (public function)
// ptr: pointer to memory to be freed
void myfree(void *ptr)
{
	u32 offset;
	if (ptr == NULL)
		return;										// Address is NULL
	offset = (u32)ptr - (u32)mallco_dev.membase;
	mem_free(offset);								// Free memory
}

// Allocate memory (public function)
// size: memory size (bytes)
// return value: pointer to the allocated memory
void *mymalloc(u32 size)
{
	u32 offset;
	offset = mem_malloc(size);
	if (offset == 0XFFFFFFFF)
		return NULL;
	else 
		return (void*)((u32)mallco_dev.membase + offset);
}

// Re-allocate memory (public function)
//*ptr: pointer to the memory
//size: size of memory to be reallocated (bytes)
//return value: pointer to the reallocated memory
void *myrealloc(void *ptr, u32 size)
{
	u32 offset;
	offset = mem_malloc(size);
	if (offset == 0XFFFFFFFF)
		return NULL;
	else
	{
		mymemcpy((void*)((u32)mallco_dev.membase + offset), ptr, size);	// Copy data to the new memory location
		myfree(ptr);  											  	// Free the old memory
		return (void*)((u32)mallco_dev.membase + offset);  			// Return the new memory address
	}
}
