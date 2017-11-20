#include "log.h"
#include "common.h"
#include "hisi_i2c.h"

typedef struct {
	unsigned int con;           /*0x0000*/
	unsigned int tar;
	unsigned int resv0[2];
	unsigned int dataCmd;       /*0x0010*/
	unsigned int resv1[2];
	unsigned int sclHcnt;
	unsigned int sclLcnt;       /*0x0020*/
	unsigned int resv2[2];
	unsigned int intrStat;
	unsigned int intrMask;      /*0x0030*/
	unsigned int intrRaw;
	unsigned int resv3[2];
	unsigned int clrIntr;       /*0x0040*/
	unsigned int resv4[10];
	unsigned int enable;
	unsigned int status;        /*0x0070*/
	unsigned int resv5[2];
	unsigned int sdaHold;
	unsigned int txAbrtSrc;     /*0x0080*/
	unsigned int resv6[1];
	unsigned int dmaCr;
	unsigned int dmaTdlr;
	unsigned int dmaRdlr;       /*0x0090*/
	unsigned int resv7[6];
	unsigned int lock;
	unsigned int mstSingleCtrl; /*0x00B0*/
	unsigned int mstSingleCmd;
	unsigned int sequenceCmd0;
	unsigned int sequenceCmd1;
	unsigned int sequenceCmd2;  /*0x00C0*/
	unsigned int resv8[463];
	unsigned int mode2I2cCtrl;  /*0x0800*/
	unsigned int mode2I2cCom;
	unsigned int mode2I2cIcr;
	unsigned int mode2I2cSr;
	unsigned int mode2I2cSclH;  /*0x0810*/
	unsigned int mode2I2cSclL;
	unsigned int mode2I2cTxr;
	unsigned int mode2I2cRxr;
	unsigned int resv9[504];
} HISI_I2C_REG;

volatile HISI_I2C_REG *i2cReg = NULL;
#define HISI_I2C_BASE_ADDR 0x12112000

int hisi_i2c_get_reg(unsigned int *regs, unsigned int *size)
{
	if (regs)
		*regs = HISI_I2C_BASE_ADDR;

	if (size)
		*size = sizeof(HISI_I2C_REG);

	return 0;
}

int hisi_i2c_set_reg(unsigned int regs)
{
	i2cReg = (volatile HISI_I2C_REG *)regs;
	LogFormat(INFO, "I2C Base Addr %08x, Map Addr %08x, size %x\n",
			(unsigned int)regs, HISI_I2C_BASE_ADDR, sizeof(HISI_I2C_REG));
	return 0;
}

#define REG_SET_BIT(reg, bit) do {                  \
	(*(volatile unsigned int *)reg) |= (0x1<<bit);  \
} while(0)

#define REG_CLR_BIT(reg, bit) do {                  \
	(*(volatile unsigned int *)reg) &= ~(0x1<<bit); \
} while(0)

#define REG_GET_BIT(reg, bit)                       \
	((*(volatile unsigned int *)reg)&(0x1<<bit))

#define REG_SET_FIELD(reg, val, mask, offset) do {              \
	unsigned int Reg = *(volatile unsigned int *)reg;           \
	Reg &= ~((mask)<<(offset));                                 \
	Reg |= ((val)&(mask))<<(offset);                            \
	(*(volatile unsigned int *)reg) = Reg;                      \
} while(0)

#define REG_GET_FIELD(reg, mask, offset)                        \
	(((*(volatile unsigned int *)reg)>>(offset))&(mask))

int hisi_i2c_init_reg(void)
{
	REG_CLR_BIT(&(i2cReg->con), 31);
	REG_SET_BIT(&(i2cReg->con), 5);

	REG_CLR_BIT(&(i2cReg->intrStat), 11);
	REG_CLR_BIT(&(i2cReg->intrStat), 10);
	REG_CLR_BIT(&(i2cReg->intrStat), 9);
	REG_CLR_BIT(&(i2cReg->intrStat), 6);

	return 0;
}

int hisi_i2c_read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	LogFormat(INFO, "I2C Write 0x%02x ,0x%02X, 0x%02X, %d\r\n", IICAddr, ByteAddr, Data[0], Size);
	return 0;
}

int hisi_i2c_write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	LogFormat(INFO, "I2C Read 0x%02X, 0x%02X, 0x%02X, %d\r\n",IICAddr, ByteAddr, Data[0], Size);
	return 0;
}
