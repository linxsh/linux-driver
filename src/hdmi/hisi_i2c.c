#include "log.h"

int hisi_i2c_read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	LogFormat(INFO, "%s %d\n", __FUNCTION__, __LINE__);
	return 0;
}

int hisi_i2c_write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	LogFormat(INFO, "%s %d\n", __FUNCTION__, __LINE__);
	return 0;
}
