/*
 *
 */

#ifndef MEMORY_H
#define MEMORY_H

//void init_mem_list(void);
struct os_mem *create_mem(void *addr, u32 num_blocks, u32 block_len);
void *get_mem(u32 size);
void free_mem(void *block_ptr);

#endif	/* MEMORY_H */
