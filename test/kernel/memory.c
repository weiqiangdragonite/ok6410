/*
 *
 */

#include "os.h"
#include "errno.h"


/*
 *
 */
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


/*
 *
 */
struct os_mem *
create_mem(void *addr, u32 num_blocks, u32 block_len)
{
	cpsr_t	cpsr;
	/* Link to next block address */
	void	**link_ptr;
	/* Block pointer */
	u8	*block_ptr;
	u32	loops;
	u32	i;
	struct os_mem	*mem_ptr;

	if (addr == NULL) {
		errno = ERR_MEM_INVALID_ADDR;
		return NULL;
	}

	if (block_len < 4) {
		//errno = ;
		return NULL;
	}

	enter_critical();
	/* Get the free block address */
	mem_ptr = os_mem_free_list;

	if (os_mem_free_list != NULL)
		os_mem_free_list = os_mem_free_list->mem_free_list;

	exit_critical();

	if (mem_ptr == NULL) {
		errno = ERR_MEM_INVALID_PART;
		return NULL;
	}


	/* link_ptr = &addr[0][0] */
	link_ptr = addr;
	/* block_ptr = &addr[0][0] */
	block_ptr = (u8 *) addr;
	loops = num_blocks - 1;

	for (i = 0; i < loops; ++i) {
		/* block_ptr = &addr[1][0], [2][0] ... */
		/* point to following block */
		block_ptr += block_len;

		/* addr[0][0], [1][0], ... = block_ptr */
		*link_ptr = block_ptr;

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
	cpsr_t	cpsr;
	void	*block_ptr;
	
	if (mem_ptr == NULL) {
		errno = ERR_MEM_INVALID_PTR;
		return NULL;
	}

	enter_critical();

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
		/* mem_free_list = next n blocks addr */
		mem_ptr->mem_free_list = *(void **) ((u8 *) block_ptr + \
			(need_blocks - 1) * mem_ptr->block_len);

		mem_ptr->num_free -= need_blocks;

		exit_critical();
		return block_ptr;
	}

	exit_critical();
	errno = ERR_MEM_NO_FREE_BLOCK;
	return NULL;
}


/*
 *
 */
void
free_mem(struct os_mem *mem_ptr, void *block_ptr)
{
	cpsr_t cpsr;

	if (mem_ptr == NULL)
		return;

	if (block_ptr == NULL)
		return;

	enter_critical();

	/* Make sure all blocks not already returned */
	if (mem_ptr->num_free >= mem_ptr->num_blocks) {
		exit_critical();
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

	/* Link the block address back to list */
	while (TRUE) {
		block_ptr += mem_ptr->block_len;

		if (block_ptr == free_block)
			break;

		*link_ptr = block_ptr;
		link_ptr = (void **) block_ptr;

		++return_blocks;
	}

	mem_ptr->num_free += return_blocks + 1;

	exit_critical();	
}
