#define LOG_TAG     "devmem"
#define MODULE_NAME "dev"
/* devmem ADDRESS [WIDTH [VALUE]] */
// -r: read and output only the value in hex, with 0x prefix
// -w: write only, no reads before or after, and no output
// or make this behavior default?


#include "osal.h"
#define perror_msg_and_die(msg) ({ __E(msg); exit(0); })
#define show_usage() ({__I("devmem ADDRESS [WIDTH [VALUE]]\n"); exit(0); })

int main(int argc, char **argv)
{
	void *map_base, *virt_addr;
	uint64_t read_result;
	uint64_t writeval = writeval; /* for compiler */
	off_t target;
	unsigned page_size, mapped_size, offset_in_page;
	int fd;
	unsigned width = 8 * sizeof(int);


	/* ADDRESS */
	if (!argv[1])
		show_usage();
	errno = 0;
	target = strtoull(argv[1], NULL, 0); /* allows hex, oct etc */

	/* WIDTH */
	if (argv[2]) {
		if (isdigit(argv[2][0]) || argv[2][1])
			width = atoi(argv[2]);
    
		/* VALUE */
		if (argv[3])
			writeval = strtoull(argv[3], NULL, 0);
	} else { /* argv[2] == NULL */
		/* make argv[3] to be a valid thing to fetch */
		argv--;
	}
	if (errno)
		show_usage(); /* one of strtouXX failed */

	fd = open("/dev/mem", argv[3] ? (O_RDWR | O_SYNC) : (O_RDONLY | O_SYNC));
	mapped_size = page_size = getpagesize();
	offset_in_page = (unsigned)target & (page_size - 1);
	if (offset_in_page + width > page_size) {
		/* This access spans pages.
		 * Must map two pages to make it possible: */
		mapped_size *= 2;
	}
	map_base = mmap(NULL,
			mapped_size,
			argv[3] ? (PROT_READ | PROT_WRITE) : PROT_READ,
			MAP_SHARED,
			fd,
			target & ~(off_t)(page_size - 1));
	if (map_base == MAP_FAILED)
		perror_msg_and_die("mmap");

//	printf("Memory mapped at address %p.\n", map_base);

	virt_addr = (char*)map_base + offset_in_page;

	if (!argv[3]) {
		switch (width) {
		case 8:
			read_result = *(volatile uint8_t*)virt_addr;
			break;
		case 16:
			read_result = *(volatile uint16_t*)virt_addr;
			break;
		case 32:
			read_result = *(volatile uint32_t*)virt_addr;
			break;
		case 64:
			read_result = *(volatile uint64_t*)virt_addr;
			break;
		default:
			perror_msg_and_die("bad width");
		}
//		printf("Value at address 0x%"OFF_FMT"X (%p): 0x%llX\n",
//			target, virt_addr,
//			(unsigned long long)read_result);
		/* Zero-padded output shows the width of access just done */
		printf("0x%0*llX\n", (width >> 2), (unsigned long long)read_result);
	} else {
		switch (width) {
		case 8:
			*(volatile uint8_t*)virt_addr = writeval;
//			read_result = *(volatile uint8_t*)virt_addr;
			break;
		case 16:
			*(volatile uint16_t*)virt_addr = writeval;
//			read_result = *(volatile uint16_t*)virt_addr;
			break;
		case 32:
			*(volatile uint32_t*)virt_addr = writeval;
//			read_result = *(volatile uint32_t*)virt_addr;
			break;
		case 64:
			*(volatile uint64_t*)virt_addr = writeval;
//			read_result = *(volatile uint64_t*)virt_addr;
			break;
		default:
			perror_msg_and_die("bad width");
		}
//		printf("Written 0x%llX; readback 0x%llX\n",
//				(unsigned long long)writeval,
//				(unsigned long long)read_result);
	}

    if (munmap(map_base, mapped_size) == -1)
        perror_msg_and_die("munmap");
    close(fd);

	return EXIT_SUCCESS;
}
