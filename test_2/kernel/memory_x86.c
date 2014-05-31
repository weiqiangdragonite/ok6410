/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


typedef unsigned char u8;
typedef unsigned int u32;

#define OS_MAX_MEM_NUM	10

#define NUM_BLOCK		128
#define BLOCK_LEN		128


/*
 * struct os_mem
 */
struct os_mem {
	void	*mem_addr;
	void	*mem_free_list;
	u32	block_len;
	u32	num_blocks;
	u32	num_free;
};

struct os_mem	*os_mem_free_list;
struct os_mem	os_mem_table[OS_MAX_MEM_NUM];


struct os_mem	*mem_buf;
u8		memory[NUM_BLOCK][BLOCK_LEN];



void init_mem_list(void);
struct os_mem *create_mem(void *addr, u32 num_blocks, u32 block_len);
void *get_mem(struct os_mem *mem_ptr, u32 size);
void free_mem(struct os_mem *mem_ptr, void *block_ptr);



int
main(void)
{
	init_mem_list();

	mem_buf = create_mem(memory, NUM_BLOCK, BLOCK_LEN);

	printf("memory address = %p\n\n", memory);

	int i;
	for (i = 0; i < 10; ++i)
		printf("mem[%d] = %p\n", i, *(void **) memory[i]);
	puts("");

	u8 *pointer[10];

	pointer[0] = get_mem(mem_buf, 130);
	printf("pointer = %p\n", pointer[0]);
	printf("free block num = %d\n", mem_buf->num_free);

	free_mem(mem_buf, pointer[0]);
	printf("release - free block num = %d\n", mem_buf->num_free);

	printf("\n\t----------\n\n");

	// free block: 126
	// use 2 blocks
	pointer[0] = get_mem(mem_buf, 130);
	printf("pointer[0] = %p\n", pointer[0]);
	printf("free block num = %d\n\n", mem_buf->num_free);

	// free block: 123
	// use 3 blocks
	pointer[1] = get_mem(mem_buf, 300);
	printf("pointer[1] = %p\n", pointer[1]);
	printf("free block num = %d\n\n", mem_buf->num_free);

	// free block: 118
	// use 5 blocks
	pointer[2] = get_mem(mem_buf, 600);
	printf("pointer = %p\n", pointer[2]);
	printf("free block num = %d\n\n", mem_buf->num_free);

	// free block: 123
	free_mem(mem_buf, pointer[2]);
	printf("release - free block num = %d\n\n", mem_buf->num_free);

	// free block: 126
	free_mem(mem_buf, pointer[1]);
	printf("release - free block num = %d\n\n", mem_buf->num_free);

	// free block: 125
	// use 1 block
	pointer[1] = get_mem(mem_buf, 100);
	printf("pointer[0] = %p\n", pointer[0]);
	printf("free block num = %d\n\n", mem_buf->num_free);

	// free block: 126
	free_mem(mem_buf, pointer[1]);
	printf("release - free block num = %d\n\n", mem_buf->num_free);

	// free block: 128
	free_mem(mem_buf, pointer[0]);
	printf("release - free block num = %d\n\n", mem_buf->num_free);

	puts("");
	for (i = 0; i < 10; ++i)
		printf("mem[%d] = %p\n", i, *(void **) memory[i]);

	return 0;
}


void
init_mem_list(void)
{
	int i;
	struct os_mem	*mem_ptr;

	/* Clear the memory table */
	memset(os_mem_table, 0, sizeof(os_mem_table));

	/* Link memory table to list */
	for (i = 0; i < OS_MAX_MEM_NUM - 1; ++i) {
		mem_ptr = &os_mem_table[i];
		mem_ptr->mem_free_list = &os_mem_table[i + 1];
	}
	/* The last memory table link to NULL */
	mem_ptr = &os_mem_table[i];
	mem_ptr->mem_free_list = NULL;

	os_mem_free_list = os_mem_table;
}

struct os_mem *
create_mem(void *addr, u32 num_blocks, u32 block_len)
{
	//cpsr_t	cpsr;
	/* Link to next block address */
	void	**link_ptr;
	/* Block pointer */
	u8	*block_ptr;
	u32	loops;
	u32	i;
	struct os_mem	*mem_ptr;

	if (addr == NULL) {
		//errno = ERR_MEM_INVALID_ADDR;
		return NULL;
	}

	if (block_len < 4) {
		//errno = ;
		return NULL;
	}

	//enter_critical();
	/* Get the free block address */
	mem_ptr = os_mem_free_list;

	if (os_mem_free_list != NULL)
		os_mem_free_list = os_mem_free_list->mem_free_list;

	//exit_critical();

	if (mem_ptr == NULL) {
		//errno = ERR_MEM_INVALID_PART;
		return NULL;
	}


	/* link_ptr = &addr[0][0] */
	link_ptr = addr;
	/* block_ptr = &addr[0][0] */
	block_ptr = (u8 *) addr;
	loops = num_blocks - 1;

	//printf("link_ptr address = %p\n", &link_ptr);
	//printf("block_ptr address = %p\n", &block_ptr);
	//printf("memory(%p) start at %p\n\n", memory, addr);

	for (i = 0; i < loops; ++i) {
		// block_ptr is a pointer which point to the block address
		//printf("block[%d] = %p\n", i, block_ptr);

		/* block_ptr = &addr[1][0], [2][0] ... */
		/* point to following block */
		block_ptr += block_len;

		// link_ptr is a pointer of pointer, which first point to addr
		// now in every loops:
		// *link_ptr is the same as addr[0][0], addr[1][0]
		// so they will be the same as block_ptr,
		// that means they point to next block address
		/* addr[0][0], [1][0], ... = block_ptr */
		*link_ptr = block_ptr;
		//printf("*link_ptr(addr: %p) = %p\n", &(*link_ptr), *link_ptr);

		/* link_ptr = &addr[0][0], [1][0], [2][0] ... */
		/* link_ptr = next block address */
		link_ptr = (void **) block_ptr;
	}
	/* Last block point to NULL */
	*link_ptr = NULL;

	mem_ptr->mem_addr = addr;
	mem_ptr->mem_free_list = addr;
	mem_ptr->num_free = num_blocks;
	mem_ptr->num_blocks = num_blocks;
	mem_ptr->block_len = block_len;

	return mem_ptr;
}

/*
 *
 */
void *
get_mem(struct os_mem *mem_ptr, u32 size)
{
	//cpsr_t	cpsr;
	void	*block_ptr;
	
	if (mem_ptr == NULL) {
		//errno = ERR_MEM_INVALID_PTR;
		return NULL;
	}

	//enter_critical();

/*
	// See if there any free memory blocks
	if (mem_ptr->num_free > 0) {
		// Point to the start free block
		block_ptr = mem_ptr->mem_free_list;
		// mem_free_list = next block addr
		mem_ptr->mem_free_list = *(void **) block_ptr;
		--(mem_ptr->num_free);

		exit_critical();
		return block_ptr;
	}
*/

	/**/
	int need_blocks;
	need_blocks = (int) (size / mem_ptr->block_len) + 1;
	if (mem_ptr->num_free > need_blocks) {
		/* Point to the start free block */
		block_ptr = mem_ptr->mem_free_list;
		/* mem_free_list = next n blocks address */
		mem_ptr->mem_free_list = *(void **) ((u8 *) block_ptr + \
			(need_blocks - 1) * mem_ptr->block_len);

		//printf("mem_ptr->mem_free_list = %p\n",
		//	mem_ptr->mem_free_list);

		mem_ptr->num_free -= need_blocks;

		//exit_critical();
		return block_ptr;
	}

	//exit_critical();
	//errno = ERR_MEM_NO_FREE_BLOCK;
	return NULL;
}

void
free_mem(struct os_mem *mem_ptr, void *block_ptr)
{
	//cpsr_t cpsr;

	if (mem_ptr == NULL)
		return;

	if (block_ptr == NULL)
		return;

	//enter_critical();

	/* Make sure all blocks not already returned */
	if (mem_ptr->num_free >= mem_ptr->num_blocks) {
		//exit_critical();
		return;
	}

	/* block_ptr point to the next free block address */
	//*(void **) block_ptr = mem_ptr->mem_free_list;
	//mem_ptr->mem_free_list = block_ptr;



	void *free_block;
	int return_blocks = 0;
	void **link_ptr;

	/* free block address */
	free_block = mem_ptr->mem_free_list;
	mem_ptr->mem_free_list = block_ptr;
	
	link_ptr = mem_ptr->mem_free_list;

	//printf("block_ptr addr = %p\n", block_ptr);
	//printf("free_block = %p\n", free_block);
	//printf("link_ptr = %p\n\n", link_ptr);

	/**/
	while (1) {
		//printf("block[%d] = %p\n", return_blocks, block_ptr);
		block_ptr += mem_ptr->block_len;

		if (block_ptr == free_block)
			break;

		*link_ptr = block_ptr;

		link_ptr = (void **) block_ptr;

		++return_blocks;
	}

	mem_ptr->num_free += return_blocks + 1;

	//exit_critical();	
}
