/*
 *
 */

#include "os.h"
#include "errno.h"






/*
 *
 */
struct os_mem *
create_mem(void *addr, u32 num_blocks, u32 block_len)
{
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

	/* Get the moemory start address */
	mem_ptr = (struct os_mem *) os_memory;

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
get_mem(u32 size)
{
	cpsr_t	cpsr;
	void	*block_ptr;
	
	if (os_mem_ptr == NULL) {
		errno = ERR_MEM_INVALID_PTR;
		return NULL;
	}

	if (size > (os_mem_ptr->block_len))
		return NULL;

	enter_critical();

	/* See if there any free memory blocks */
	if (os_mem_ptr->num_free > 0) {
		block_ptr = os_mem_ptr->mem_free_list;
		/* mem_free_list = next block addr */
		os_mem_ptr->mem_free_list = *(void **) block_ptr;
		--(os_mem_ptr->num_free);

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
free_mem(void *block_ptr)
{
	cpsr_t cpsr;

	if (os_mem_ptr == NULL)
		return;

	if (block_ptr == NULL)
		return;

	enter_critical();

	/* Make sure all blocks not already returned */
	if (os_mem_ptr->num_free >= os_mem_ptr->num_blocks) {
		exit_critical();
		return;
	}

	*(void **) block_ptr = os_mem_ptr->mem_free_list;
	os_mem_ptr->mem_free_list = block_ptr;
	++(os_mem_ptr->num_free);

	exit_critical();	
}
