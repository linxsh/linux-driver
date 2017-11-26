#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define I2C_CTRL_NAME "/dev/i2c-2"

int handle = 0;
static struct i2c_rdwr_ioctl_data i2c_data;

int i2c_init()
{
	struct i2c_msg *msgs[2];

	 handle = open(I2C_CTRL_NAME, O_RDWR);
	if (handle < 0) {
		printf("%s %d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ioctl(handle, I2C_SLAVE, 0x52);
	ioctl(handle, I2C_TIMEOUT, 1000);
	ioctl(handle, I2C_RETRIES, 1);

	i2c_data.nmsgs = 2;
	i2c_data.msgs = malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));
	if (i2c_data.msgs == NULL) {
		printf("can not malloc\n");
		return -1;
	}

	msgs[0] = i2c_data.msgs;
	msgs[1] = i2c_data.msgs + 1;

	msgs[0]->buf = malloc(2);
	msgs[1]->buf = malloc(2);

	return 0;
}

int i2c_write(unsigned char byteAddr, unsigned char val)
{
	struct i2c_msg *p_msg = &i2c_data.msgs[0];

	i2c_data.nmsgs = 1;
	p_msg->len     = 2;
	p_msg->addr    = 0x52;
	p_msg->flags  = 0;
	p_msg->buf[0] = byteAddr;
	p_msg->buf[1] = val;
	if (ioctl(handle, I2C_RDWR, (unsigned long)&i2c_data) < 0) {
		printf("write error\n");
	}

	return 0;
}

int i2c_read(unsigned char byteAddr)
{
	struct i2c_msg *msgs[2];

	i2c_data.nmsgs  = 2;

	msgs[0] = i2c_data.msgs;
	msgs[1] = i2c_data.msgs + 1;

	msgs[0]->len    = 1;
	msgs[0]->addr   = 0x52;
	msgs[0]->flags  = 0;
	msgs[0]->buf[0] = byteAddr;

	msgs[1]->len    = 1;
	msgs[1]->addr   = 0x52;
	msgs[1]->flags  = 1;
	msgs[1]->buf[0] = 0;

	if (ioctl(handle, I2C_RDWR, (unsigned long) &i2c_data) < 0) {
		printf("read error\n");
	}

	printf("%d\n", msgs[1]->buf[0]);
	return 0;
}

int main (void)
{
	i2c_init();
	while (1) {
		i2c_write(0x00, 0x80);
		sleep(1);
	}
	return 0;
}
