#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define REGISTER_VERSION "register version:v1.2  --20160623"

#define MEM_DEV_NAME "/dev/mem"

#define MAX_READ_BYTES       4096
#define DEFAULT_READ_BYTES   4
#define MMAP_SIZE_UNIT       4096

static int mem_dev_fd;
static char *output_file;

static void usage(void)
{
	printf("\n");
	printf("Usage: register [-r <base_addr>] [-w <addr> ] [-v <value>] [-n <read bytes>] [-V]\n");
	printf("  option:\n");
	printf("    -r read the value from <base_addr>, you can use with -n\n");
	printf("    -n read bytes, defualt is 4, usually use with -r\n");
	printf("    -w write <addr>, must use with -v\n");
	printf("    -v value, must use with -w\n");
	printf("    -V version, show the register version\n");
	printf("    -d dump reg print to file \n");
	printf("    -f dump filename \n");
	printf("  tips:\n");
	printf("    the base_addr & addr is hardware address\n");
	printf("\n");
	printf("Example:\n");
	printf("  ./register -r 0x00204000\n");
	printf("  ./register -r 0x00204000 -n 16\n");
	printf("  ./register -w 0x00204000 -v 0x04e2086a\n");
	printf("  ./register -d 0x00204000 -n 16 -f reg.out\n");
	printf("\n");
	printf("if you want know more, please access git.nationalchip.com or mail to huangjb@nationalchip.com\n");
}

static int register_mmap(unsigned int base_addr, unsigned int size, unsigned int *virtual_addr)
{
	unsigned int align_addr;

	align_addr = base_addr / MMAP_SIZE_UNIT * MMAP_SIZE_UNIT;
	size = size + base_addr - align_addr;
	if (size % MMAP_SIZE_UNIT)
		size = (size / MMAP_SIZE_UNIT + 1) * MMAP_SIZE_UNIT;

	mem_dev_fd = open(MEM_DEV_NAME, O_RDWR | O_NDELAY | O_SYNC);
	if (mem_dev_fd < 0)  {
		printf("error: open %s failed.\n", MEM_DEV_NAME);
		exit(EXIT_FAILURE);
	}

	*virtual_addr = (unsigned int)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_dev_fd, align_addr);
	if (MAP_FAILED == (void *)*virtual_addr) {
		printf("error: mmap failed. fd(%d), addr(0x%x), size(%d)\n", mem_dev_fd, align_addr, size);
		exit(EXIT_FAILURE);
	}

	*virtual_addr = *virtual_addr + base_addr - align_addr;

	return 0;
}

static int register_munmap(unsigned int virtual_addr, unsigned int size)
{
	unsigned int align_addr;

	align_addr = virtual_addr / MMAP_SIZE_UNIT * MMAP_SIZE_UNIT;
	size = size + virtual_addr - align_addr;
	if (size % MMAP_SIZE_UNIT) {
		size = (size / MMAP_SIZE_UNIT + 1) * MMAP_SIZE_UNIT;
	}

	if (munmap((void *)align_addr, size) < 0) {
		printf("error: munmap failed. fd(%d), align_addr(0x%x), size(%d)\n", mem_dev_fd, align_addr, size);
		exit(EXIT_FAILURE);
	}

	if (close(mem_dev_fd) < 0) {
		printf("error: %s close failed. fd(%d)\n", MEM_DEV_NAME, mem_dev_fd);
		exit(EXIT_FAILURE);
	}

	return 0;
}

#define WRITE_STR(fd, str, len, flag) \
	do{ \
		int ret; \
		if (flag) \
			ret = write(fd, str, len); \
		else \
			printf("%s", str); \
		ret++; \
	} while(0)

static int read_register(unsigned int base_addr, unsigned int size, int flag)
{
	int i, fd;
	unsigned int virtual_addr;
	char buf[36];

	if (size % 4) {
		size = (size / 4 + 1) * 4;
	}
	if (flag) {
		fd = open(output_file, O_WRONLY|O_CREAT|O_TRUNC, 00755);
		if (fd < 0) {
			printf("error: create file failed\n");
			free(output_file);
			output_file = NULL;
			exit(EXIT_FAILURE);
		}
		printf("create  %s !\n",output_file);
	}

	register_mmap(base_addr, size, &virtual_addr);

	for (i = 0; i < size; i += 4) {
		if (i % 16 == 0) {
			if (i != 0) {
				WRITE_STR(fd, "\n", 1, flag);
			}
			sprintf(buf, "0x%08x:        ", base_addr + i);
			WRITE_STR(fd, buf, 19, flag);
		}
		sprintf(buf, "0x%08x ", *(volatile unsigned int *)(virtual_addr + i));
		WRITE_STR(fd, buf, 11, flag);
	}
	WRITE_STR(fd, "\n", 1, flag);

	register_munmap(virtual_addr, size);
	if(flag)
		close(fd);

	return 0;
}

static int write_register(unsigned int addr, unsigned int value)
{
	unsigned int virtual_addr;
	register_mmap(addr, MMAP_SIZE_UNIT, &virtual_addr);
	*(volatile unsigned int *)virtual_addr = value;
	register_munmap(virtual_addr, MMAP_SIZE_UNIT);
	return 0;
}

int main(int argc,char *argv[])
{
	int opt;
	unsigned int addr = 0;
	unsigned int value = 0;
	unsigned int size = DEFAULT_READ_BYTES;
	int d_flag = 0;
	int f_flag = 0;
	int r_flag = 0;
	int w_flag = 0;
	int v_flag = 0;
	output_file = NULL;

	if (argc<2) {
		usage();
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc,argv,"r:d:f:n:w:v:hV")) !=-1 ) {
		switch(opt) {
			case 'd':
			case 'r':
				addr = strtoul(optarg, NULL, 0);
				if (addr % 4) {
					printf("error: addr 0x%x is not 4 bytes aligned.\n", addr);
					exit(EXIT_FAILURE);
				}
				if ('d' == opt)
					d_flag = 1;
				r_flag = 1;
				break;

			case 'n':
				size = strtoul(optarg, NULL, 0);
				if (!d_flag && size > MAX_READ_BYTES) {
					printf("warning: size=%d > MAX_READ_BYTES=%d, set size=%d\n", size, MAX_READ_BYTES, MAX_READ_BYTES);
					size = MAX_READ_BYTES;
				}
				break;

			case 'w':
				addr = strtoul(optarg, NULL, 0);
				if (addr % 4) {
					printf("error: addr 0x%x is not 4 bytes aligned.\n", addr);
					exit(EXIT_FAILURE);
				}

				w_flag = 1;
				break;

			case 'v':
				value = strtoul(optarg, NULL, 0);
				v_flag = 1;
				break;

			case 'V':
				printf("%s\n", REGISTER_VERSION);
				exit(EXIT_SUCCESS);

			case 'f':
				if (!d_flag)
					break;
				int size = strlen(optarg) + 1;
				output_file = (char *)malloc(size);
				strcpy(output_file, optarg);
				f_flag = 1;
				break;

			case 'h':
			default:
				usage();
				exit(EXIT_FAILURE);
		}
	}
	if ((d_flag | f_flag) && ((d_flag & f_flag) == 0)) {
		printf("error: forget -d or -f \n");
		exit(EXIT_FAILURE);
	}

	if (r_flag)
		read_register(addr, size, d_flag);

	if (w_flag) {
		if (v_flag == 0) {
			printf("error: have -w, but not have -v.\n");
			exit(EXIT_FAILURE);
		}
		write_register(addr, value);
	}
	if (output_file) {
		free(output_file);
		output_file = NULL;
	}

	return 0;
}
