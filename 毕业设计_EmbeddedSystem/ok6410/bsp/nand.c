/******************************************************************************\
*
\******************************************************************************/

/*

Nandflash:ChipType= MLC  ChipName=samsung-K9LBG08U0D
NandFlash:name=NAND 4GiB 1,8V 8-bit,id=d7, pagesize=4096 ,
chipsize=2048 MB,erasesize=524288 oobsize=128
sector size 512 KiB

6410是从nand flash的前四页，每页读2KB共8KB到steppingstone中运行

nand:
        2K      2K
    +-------+-------+
  0 |   A   |   B   |
    +-------+-------+
  1 |   B   |   C   |
    +-------+-------+
  2 |   C   |   D   |
    +-------+-------+
  3 |   D   |   E   |
    +-------+-------+
  4 |   E   |   F   |
    +-------+-------+
  5 |   G   |   H   |
    +-------+-------+
    |      ...      |
    +---------------+


以page（页）为单位进行读写，以block（块）为单位进行擦除。每一页中又分为
main区和spare区，main区用于正常数据的存储，spare区用于存储一些附加信息，
如块好坏的标记、块的逻辑地址、页内数据的ECC校验和等。(4K + 218 Byte)

K9GAG08X0D:
一页(page): 4K + 218
一块(block): (4K + 218) * 128 pages = 512K + 27.25K
整个设备(device): (4K + 218) * 128 pages * 4096 Blocks
                  = (512K + 27.25K) * 4096
                  = 2157 MB = 2157 * 8 = 17256 Mbits
                  
main容量为: 512K * 4096 = 2048 M
即总共有2*1024*1024=524288 pages
4096 * 128 = 0x80000 pages


如何知道我自己芯片的block大小?
page - 4(K)
block - 128 pages * 4(K) = 512(K)
device - 8192 blocks * 512(K) = 4096(MB)
总共有8192 * 128 = 0x100000 pages = 1048576 pages
即0 ~ 1048575

一个block有128 page，擦除时是擦除一个block，即擦除第0 page ~ 第127 page。
现擦除第二个block，即第128 page开始:
int page = start / 4096;
所以start = 0x80000
因为是擦除第128 page ~ 第255 page，所以读取第127、128、129、254、255、256 page的数据，
如果第128、129、254、255 page的数据全为F，第256 page的数据不全为F，则可判断应该
是1个block含128 pages。

向第1048575 page写数据，再读出来(OK)
所以nand地址范围为0x0 ~ 0xFFFFFFFF，即4GB，有1048576 pages

1048576 * 4 * 1024
4 * 1024 * 1024 * 1024


5个周期:
1st: A0 ~ A7
2nd: A8 ~ A12
3rd: A13 ~ A20
4th: A21 ~ A28
5th: A29 ~ A31


首先需要写入读ID命令(0x90)，然后再写入0x00地址，并等待芯片就绪，
就可以读取到一共五个周期的芯片ID，第一个周期为厂商ID，第二个周期为设备ID，
第三个周期至第五个周期包括了一些具体的该芯片信息。

Nand Flash的每一页有两区：main区和spare区，main区用于存储正常的数据，
spare区用于存储其他附加信息，其中就包括ECC校验码。当我们在写入数据的时候，
我们就计算这一页数据的ECC校验码，然后把校验码存储到spare区的特定位置中，
在下次读取这一页数据的时候，同样我们也计算ECC校验码，
然后与spare区中的ECC校验码比较，如果一致则说明读取的数据正确，
如果不一致则不正确。ECC的算法较为复杂，好在S3C2440能够硬件产生ECC校验码，
这样就省去了不少的麻烦事。S3C2440既可以产生main区的ECC校验码，
也可以产生spare区的ECC校验码。因为K9F2G08U0A是8位IO口，因此S3C2440
共产生4个字节的main区ECC码和2个字节的spare区ECC码。在这里我们规定，
在每一页的spare区的第0个地址到第3个地址存储main区ECC，
第4个地址和第5个地址存储spare区ECC。

产生ECC校验码的过程为：在读取或写入哪个区的数据之前，先解锁该区的ECC，
以便产生该区的ECC。在读取或写入完数据之后，再锁定该区的ECC，
这样系统就会把产生的ECC码保存到相应的寄存器中。

main区的ECC保存到NFMECC0/1中（因为K9F2G08U0A是8位IO口，
因此这里只用到了NFMECC0），spare区的ECC保存到NFSECC中。

对于读操作来说，我们还要继续读取spare区的相应地址内容，
以得到上次写操作时所存储的main区和spare区的ECC，
并把这些数据分别放入NFMECCD0/1和NFSECCD的相应位置中。
最后我们就可以通过读取NFESTAT0/1（因为K9F2G08U0A是8位IO口，
因此这里只用到了NFESTAT0）中的低4位来判断读取的数据是否正确，
其中第0位和第1位为main区指示错误，第2位和第3位为spare区指示错误。



http://blog.csdn.net/cp1300/article/details/7769100
http://blog.csdn.net/canjiangsu/article/details/6162677
http://blog.csdn.net/girlkoo/article/details/8115849

*/


#include "s3c6410.h"
#include "uart.h"
#include "nand.h"


/*
 *
 */
void
init_nand(void)
{
	/* 1. Set MEM_SYS_CFG[5:0] to NFCON */
	MEM_SYS_CFG = 0;

	/*
	 * 2. Set the NFCONF register
	 * TACLS = 0; TWRPH0 = 3; TWRPH1 = 1
	 * MsgLength = 512-byte, ECCType = 4-bit ECC
	 */
	NFCONF = (NFCONF & 0xBC7F888F) | 0x01000310;

	/*
	 * 3. Set the NFCONT and enable NAND Flash controller
	 * Disable lock-tight and soft lock
	 * Init main aera ECC value, clear MainECCLock
	 * _XX00_XXXX_XXXX_1111_X111
	 */
	NFCONT = (NFCONT & 0xFFFCFF08) | 0xF7;


	/* Reset nand */

	/* 1. Enable the nand chip select */
	NFCONT &= ~2;

	/* 2. Send the reset command */
	NFCMMD = 0xFF;

	/* 3. Wait for ready */
	while (1) {
		if (NFSTAT & 1)
			break;
	}

	/* 4. Disable the nand chip select */
	NFCONT |= 2;
}


void
copy_nand_to_sdram(unsigned int start_addr, unsigned int *dest_addr,
                   unsigned int length)
{
    unsigned int count, page, column, size;
    int i;
    
    count = 0;
    // get the page which start_addr in
    page = start_addr / PAGE_SIZE;
    // get the address in the page
    column = start_addr - page * PAGE_SIZE;
    
    // enable the nand chip select
    NFCONT &= ~2;
    
    // begin to copy data
    while (1) {
        // if copy the bootloader, page size = 2048
        if (page >= 0 && page < 4)
            size = 2048;
        // default page size
        else
            size = PAGE_SIZE;
        
        // next page
        if (column >= size) {
            ++page;
            column -= size;
        }

        // send the 1st READ command
        NFCMMD = 0;
        
        // send the column address
        NFADDR = column & 0xFF;
        NFADDR = (column >> 8) & 0x1F;
        // send the page address
        NFADDR = page & 0xFF;
        NFADDR = (page >> 8) & 0xFF;
        NFADDR = (page >> 16) & 0xFF;
        
        // send the 2nd READ command
        NFCMMD = 0x30;
        
        // wait for nand ready
        while (1) {
            if (NFSTAT & 1)
                break;
        }
        
        // start to read data
        // read one page(4 KB) data, every time read 4 bytes
        for (i = column; i < size && count < length; i += 4) {
            *dest_addr++ = NFDATA;
            count += 4;
        }
        
        // copy end
        if (count >= length)
            break;
        
        // next page
        ++page;
        column = 0;
    }
    
    // disable the nand chip select
    NFCONT |= 2;
}



void nand_read_ID(void)
{
    // 1. enable the nand chip select
    NFCONT &= ~2;
    
    // 2. send Read ID command
    NFCMMD = 0x90;
    
    // 3. send address
    NFADDR = 0;
    
    // 4. wait for nand ready
    while (1) {
        if (NFSTAT & 1) break;
    }
    
    // 5. read data
    // we just need 6 bytes to store the nand ID
    // 厂商代码
    // 设备代码
    unsigned int data[3];
    data[0] = NFDATA;
    data[1] = NFDATA;
    data[2] = NFDATA;
    
    // 6. disable the nand chip select
    NFCONT |= 2;
    
    //
    uart_print("The ID is: ");
    uart_print_hex(data[2]);
    uart_print_hex(data[1]);
    uart_print_hex(data[0]);
    uart_puts("");
    
    return;
}


void nand_read(unsigned int start_addr, unsigned int end_addr)
{
    unsigned int length = end_addr - start_addr;
    unsigned int count = 0;
    unsigned int page = 0;
    unsigned int column = 0;
    unsigned int data = 0;
    int i, j;
    
    // enable the nand chip select
    NFCONT &= ~2;
    
    while (1) {
        page = start_addr / PAGE_SIZE;
        column = start_addr - page * PAGE_SIZE;
        
        // print some msg
        uart_puts("");
        uart_print("start_addr = ");
        uart_print_hex(start_addr);
        uart_print("  page = ");
        uart_print_hex(page);
        uart_print("  column = ");
        uart_print_hex(column);
        uart_puts("");
        
        // send the 1st READ page command
        NFCMMD = 0;
        
        // send the column address
        NFADDR = column & 0xFF;
        NFADDR = (column >> 8) & 0x1F;
        // send the page address
        NFADDR = page & 0xFF;
        NFADDR = (page >> 8) & 0xFF;
        NFADDR = (page >> 16) & 0xFF;
        
        // send the 2nd READ page command
        NFCMMD = 0x30;
        
        // wait for nand ready
        while (1) {
            if (NFSTAT & 1) break;
        }
        
        // start to read data
        // read one page(4 KB) data, every time read 4 bytes
        for (i = column, j = 0; i < PAGE_SIZE &&
                                        count < length; i += 4, ++j) {
            data = NFDATA;

            // print the addr, every line form 0 ~ F, 4 column
            if (j % 4 == 0) {
                uart_puts("");
                uart_print_hex(start_addr);
                uart_print(" : ");
            }
            uart_print_addr_value(data);
            uart_print("  ");

            start_addr += 4;
            count += 4;
        }
        uart_puts("");
        
        // check read size
        if (count >= length) break; 
    }
    
    // disable the nand chip select
    NFCONT |= 2;
    
    return;
}


// Block Erase 60h D0h
// 在擦除结束前还要判断是否擦除操作成功，
// 另外同样也存在需要判断是否为坏块以及要标注坏块的问题。
// 一块(block): (4K + 218) * 128 pages = 512K + 27.25K
// rNF_IsBadBlock和rNF_MarkBadBlock
void
nand_erase_block(unsigned int start)
{
    // check the block if is bad block
    
    // enable the nand chip select
    NFCONT &= ~2;
    
    // send the erase block command
    NFCMMD = 0x60;
    
    // send the addr
    int page = start / 4096;
    NFADDR = page & 0xFF;
    NFADDR = (page >> 8) & 0xFF;
    NFADDR = (page >> 16) & 0xFF;
    
    //
    NFCMMD = 0xD0;
    
    // wait
    while (1) {
        if (NFSTAT & 1) break;
    }
    
    // disable the nand chip select
    NFCONT |= 2;
    
    // check
    
    return;
}


void
write_to_boot(unsigned int dest, unsigned int length)
{
    unsigned int count = 0;
    unsigned int page = 0;
    unsigned int column = 0;
    int i;

    // enable the nand chip select
    NFCONT &= ~2;
    
    while (1) {
        
        if (page > 0 && page < 5) dest -= 2048;
        
        //uart_printf("\ndest_addr = 0x");
        //uart_print_hex(dest);
        //uart_printf("  count = ");
        //uart_print_int(count);
        //uart_puts("");
    
        // send write command
        NFCMMD = 0x80;
    
        // send the column address
        NFADDR = column & 0xFF;
        NFADDR = (column >> 8) & 0x1F;
        // send the page address
        NFADDR = page & 0xFF;
        NFADDR = (page >> 8) & 0xFF;
        NFADDR = (page >> 16) & 0xFF;
    
    
        // write data
        for (i = column; i < PAGE_SIZE && count < length; i += 4) {
            NFDATA = *(unsigned int *) dest;
            dest += 4;
            if (i < 2048) count += 4;
        }
    
        // 
        NFCMMD = 0x10;
    
        // wait
        while (1) {
            if (NFSTAT & 1) break;
        }
        
        //
        if (count >= length) break;
        ++page;
    
    }
    
    // disable the nand chip select
    NFCONT |= 2;
    
    return;
}

void write_to_nand(unsigned int src, unsigned int dest, unsigned int length)
{
    unsigned int count = 0;
    unsigned int page = dest / PAGE_SIZE;
    unsigned int column = dest - page * PAGE_SIZE;
    int i;

    // enable the nand chip select
    NFCONT &= ~2;
    
    while (1) {
    
        uart_print("\npage = ");
        uart_print_hex(page);
        //uart_printf("  count = ");
        //uart_print_int(count);
        uart_puts("");
    
        // send write command
        NFCMMD = 0x80;
    
        // send the column address
        NFADDR = column & 0xFF;
        NFADDR = (column >> 8) & 0x1F;
        // send the page address
        NFADDR = page & 0xFF;
        NFADDR = (page >> 8) & 0xFF;
        NFADDR = (page >> 16) & 0xFF;
    
    
        // write data
        for (i = column; i < PAGE_SIZE && count < length; i += 4) {
            NFDATA = *(unsigned int *) src;
            src += 4;
            count += 4;
        }
    
        // 
        NFCMMD = 0x10;
    
        // wait
        while (1) {
            if (NFSTAT & 1) break;
        }
        
        //
        if (count >= length) break;
        ++page;
    
    }
    
    // disable the nand chip select
    NFCONT |= 2;
    
    return;
}
