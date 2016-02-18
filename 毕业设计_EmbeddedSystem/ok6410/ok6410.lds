SECTIONS
{
	. = 0x50000000;

	. = ALIGN(4);
	.text :
	{
		start.o
		exceptions.o
		clock.o
		sdram.o
		nand.o
		boot.o
		timer.o
		led.o
		arm_s.o
		core.o
		task.o
		time.o
		app.o
		* (.text)
	}

	. = ALIGN(4);
	.rodata :
	{
		* (.rodata)
	}

	. = ALIGN(4);
	.data :
	{
		* (.data)
	}

	. = ALIGN(4);
	bss_start = . ;
	.bss :
	{
		* (.bss)
		* (.common)
	}
	bss_end = . ;
}
