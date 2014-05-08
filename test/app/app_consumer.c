/*


*/


int
main(void)
{
	init_os();

	/* Init sys time */
	init_sys_time();




	/* Create the memory buffer */
	mem_buf = create_mem(memory, NUM_BLOCK, BLOCK_LEN);

	start_os();

	return 0;
}

void
producer_task(void *arg)
{
	u8 *buffer;
	while (1) {
		/* 申请一个缓冲区 */
		buffer = (u8 *) get_mem(mem_buf, BLOCK_LEN);

		/* 申请失败，没有空余缓冲区了 */
		if (buffer == NULL) {
			sleep(1);
			continue;
		}

		/* 申请成功，每个字节填1 */
	}
}

void
consumer_task(void *arg)
{
	while (1) {

	}
}
