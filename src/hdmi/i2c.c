#include "log.h"
#include "common.h"
#include "i2c.h"

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
} I2C_REG;

volatile I2C_REG *i2cReg = NULL;
#define I2C_BASE_ADDR 0x12112000
#define I2C_TIMEOUT   (0x10000)

int i2c_get_reg(unsigned int *regs, unsigned int *size)
{
	if (regs)
		*regs = I2C_BASE_ADDR;

	if (size)
		*size = sizeof(I2C_REG);

	return 0;
}

int i2c_set_reg(unsigned int regs)
{
	i2cReg = (volatile I2C_REG *)regs;
	LogFormat(INFO, "I2C Base Addr %08x, Map Addr %08x, size %x\n",
			(unsigned int)regs, I2C_BASE_ADDR, sizeof(I2C_REG));
	return 0;
}

#define REG_SET_VAL(reg, val) do {                  \
	(*(volatile unsigned int *)reg) = val;          \
} while(0)

#define REG_GET_VAL(reg)                            \
	(*(volatile unsigned int *)reg)

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

#define I2C_CLOCK_RATE  (40000000) /* 40MHZ  */
#define I2C_WORKS_FREQ  (300000)   /* 300KHZ */

static inline void set_i2c_disable(void)
{
	REG_CLR_BIT(&(i2cReg->enable),   0);
}

static inline void set_i2c_enable(void)
{
	REG_SET_BIT(&(i2cReg->enable),   0);
}

static unsigned int lastSlaveAddr = 0;

static inline void set_i2c_salve_addr(unsigned int addr)
{
	set_i2c_disable();
	REG_SET_FIELD(&(i2cReg->tar), addr, 0x3ff, 0);
	REG_SET_BIT(&(i2cReg->tar), 12);
	set_i2c_enable();
	lastSlaveAddr = addr;
}

static inline void set_i2c_mode(void)
{
	set_i2c_disable();
	REG_CLR_BIT(&(i2cReg->sequenceCmd0),  31);
	REG_SET_BIT(&(i2cReg->mstSingleCtrl), 31);
	set_i2c_enable();
}

static inline int wait_i2c_tx_nofull(void)
{
	unsigned int val;
	unsigned int time_cnt = 0;

	do {
		val = REG_GET_VAL(&(i2cReg->intrRaw));
		if (val & (0x1 << 6)) {
			val = REG_GET_VAL(&(i2cReg->txAbrtSrc));
			LogFormat(ERROR, "TX_ABRT: 0x%x\n", val);
			return -1;
		}

		val = REG_GET_VAL(&(i2cReg->mstSingleCtrl));
		if (val & (0x1 << 21))
			break;

		if (time_cnt >= I2C_TIMEOUT) {
			LogFormat(ERROR, "wait tx no full timeout, last CR1: %#x\n", val);
			return -1;
		}

		time_cnt++;
		udelay(50);
	} while(1);

	return 0;
}

static inline int wait_i2c_rx_noempty(void)
{
	unsigned int val;
	unsigned int time_cnt = 0;

	do {
		val = REG_GET_VAL(&(i2cReg->intrRaw));
		if (val & (0x1 << 6)) {
			val = REG_GET_VAL(&(i2cReg->txAbrtSrc));
			LogFormat(ERROR, "TX_ABRT: 0x%x\n", val);
			return -1;
		}

		val = REG_GET_VAL(&(i2cReg->mstSingleCtrl));
		if (val & (0x1 << 8))
			break;

		if (time_cnt >= I2C_TIMEOUT) {
			LogFormat(ERROR, "wait rx no empty timeout, last CR1: %#x\n", val);
			return -1;
		}

		time_cnt++;
		udelay(50);
	} while(1);

	return 0;
}

static inline int wait_i2c_idle(void)
{
	unsigned int val;
	unsigned int time_cnt = 0;

	do {
		val = REG_GET_VAL(&(i2cReg->intrRaw));
		if (val & (0x1 << 6)) {
			val = REG_GET_VAL(&(i2cReg->txAbrtSrc));
			LogFormat(ERROR, "TX_ABRT: 0x%x\n", val);
			return -1;
		}

		val = REG_GET_VAL(&(i2cReg->mstSingleCtrl));
		if (val & (~(0x1 << 8)) & (0x1 << 21))
			break;

		if (time_cnt >= I2C_TIMEOUT) {
			LogFormat(ERROR, "wait idle timeout, last CR1: %#x\n", val);
			return -1;
		}

		time_cnt++;
		udelay(50);
	} while(1);

	udelay(10);

	time_cnt = 0;

	do {
		val = REG_GET_VAL(&(i2cReg->intrRaw));
		if (val & (0x1 << 6)) {
			val = REG_GET_VAL(&(i2cReg->txAbrtSrc));
			LogFormat(ERROR, "TX_ABRT: 0x%x\n", val);
			return -1;
		}

		val = REG_GET_VAL(&(i2cReg->status));
		if (!(val & (0x1 << 0)))
			break;

		if (time_cnt >= I2C_TIMEOUT) {
			LogFormat(ERROR, "wait idle timeout, last CR1: %#x\n", val);
			return -1;
		}

		time_cnt++;
		udelay(50);
	} while(1);

	return 0;
}

static inline int clr_i2c_status(void)
{
	REG_CLR_BIT(&(i2cReg->mstSingleCtrl), 31);
	REG_SET_BIT(&(i2cReg->mstSingleCtrl), 31);
	REG_SET_VAL(&(i2cReg->clrIntr), 0x1);

	set_i2c_disable();
	set_i2c_enable();

	return 0;
}

int i2c_init_reg(void)
{
	unsigned int sclHcnt = ((I2C_CLOCK_RATE / 100) * 36) / I2C_WORKS_FREQ;
	unsigned int sclLcnt = ((I2C_CLOCK_RATE / 100) * 64) / I2C_WORKS_FREQ;
	unsigned int sdaHold = sclHcnt / 2;

	REG_SET_VAL(&(i2cReg->lock), 0x1ACCE551);

	set_i2c_disable();

	REG_SET_BIT(&(i2cReg->con), 5);

	REG_SET_FIELD(&(i2cReg->sclHcnt), sclHcnt, 0xffff, 0);
	REG_SET_FIELD(&(i2cReg->sclLcnt), sclLcnt, 0xffff, 0);
	REG_SET_FIELD(&(i2cReg->sdaHold), sdaHold, 0xffff, 0);

	REG_SET_VAL(&(i2cReg->intrMask), 0xffffffff);

	REG_SET_VAL(&(i2cReg->tar), 0x0);

	set_i2c_salve_addr(0);

	set_i2c_mode();

	set_i2c_enable();

	return 0;
}

int i2c_read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	unsigned int i;

	if (IICAddr != lastSlaveAddr) {
		set_i2c_salve_addr((unsigned int)IICAddr);
		set_i2c_mode();
	}

	for (i = 0; i < Size; i++) {
		unsigned int status = 0, val = 0;

		REG_SET_BIT(&(i2cReg->mstSingleCtrl), 30);
		REG_CLR_BIT(&(i2cReg->mstSingleCtrl), 29);
		REG_CLR_BIT(&(i2cReg->mstSingleCtrl), 28);

		status = wait_i2c_tx_nofull();
		if (status) {
			wait_i2c_idle();
			clr_i2c_status();
			return 4;
		}

		REG_SET_FIELD(&(i2cReg->mstSingleCmd), ByteAddr, 0xffff, 16);

		status = wait_i2c_rx_noempty();
		if (status) {
			wait_i2c_idle();
			clr_i2c_status();
			return 2;
		}

		val = REG_GET_VAL(&(i2cReg->mstSingleCmd));
		Data[i] = (val & 0xff);

		clr_i2c_status();
	}

//	LogFormat(INFO, "I2C Write 0x%02x ,0x%02X, 0x%02X, %d\r\n", IICAddr, ByteAddr, Data[0], Size);
	return 0;
}

int i2c_write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
//	LogFormat(INFO, "I2C Read 0x%02X, 0x%02X, 0x%02X, %d\r\n",IICAddr, ByteAddr, Data[0], Size);
	return 0;
}
