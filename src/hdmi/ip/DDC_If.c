/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  DDC_If.c

  Description :  EP952 DDC Interface

\******************************************************************************/

#include "DDC_If.h"
#include "common.h"
#include "log.h"
#include "../i2c.h"

//--------------------------------------------------------------------------------------------------

#define EDID_ADDR               0xA0     // EDID Address
#define EDID_SEGMENT_PTR        0x60     // EDID segment pointer

//--------------------------------------------------------------------------------------------------

#define HDCP_RX_ADDR            0x74     // TV HDCP RX Address
#define HDCP_RX_BKSV_ADDR       0x00     // TV HDCP RX, BKSV Register Address
#define HDCP_RX_RI_ADDR         0x08     // TV HDCP RX, RI Register Address
#define HDCP_RX_AKSV_ADDR       0x10     // TV HDCP RX, AKSV Register Address
#define HDCP_RX_AINFO_ADDR      0x15     // TV HDCP RX, AINFO Register Address
#define HDCP_RX_AN_ADDR         0x18     // TV HDCP RX, AN Register Address
#define HDCP_RX_SHA1_HASH_ADDR  0x20     // TV HDCP RX, SHA-1 Hash Value Start Address
#define HDCP_RX_BCAPS_ADDR      0x40     // TV HDCP RX, BCAPS Register Address
#define HDCP_RX_BSTATUS_ADDR    0x41     // TV HDCP RX, BSTATUS Register Address
#define HDCP_RX_KSV_FIFO_ADDR   0x43     // TV HDCP RX, KSV FIFO Start Address

//--------------------------------------------------------------------------------------------------

// Private Data
SMBUS_STATUS status;

unsigned char DDC_Data[128];
unsigned char TempBit;

// Private Functions
SMBUS_STATUS DDC_Write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);
SMBUS_STATUS DDC_Read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);
//==================================================================================================

//--------------------------------------------------------------------------------------------------
//
// Downstream HDCP Control
//

unsigned char Downstream_Rx_read_BKSV(unsigned char *pBKSV)
{
	int i, j;
	status = DDC_Read(HDCP_RX_ADDR, HDCP_RX_BKSV_ADDR, pBKSV, 5);
	if(status != SMBUS_STATUS_Success) {
		LogFormat(ERROR, "BKSV read - DN DDC %d\r\n", (int)status);
		return 0;
	}

	i = 0;
	j = 0;
	while (i < 5) {
		TempBit = 1;
		while (TempBit) {
			if (pBKSV[i] & TempBit) j++;
			TempBit <<= 1;
		}
		i++;
	}
	if(j != 20) {
		LogFormat(ERROR, "BKSV read - Key Wrong\r\n");
		LogFormat(ERROR, "BKSV=0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n",
				(unsigned int)pBKSV[0], (unsigned int)pBKSV[1], (unsigned int)pBKSV[2], (unsigned int)pBKSV[3], (unsigned int)pBKSV[4]);
		return 0;
	}
	return 1;
}

unsigned char Downstream_Rx_BCAPS(void)
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_BCAPS_ADDR, DDC_Data, 1);
	return DDC_Data[0];
}

void Downstream_Rx_write_AINFO(char ainfo)
{
	DDC_Write(HDCP_RX_ADDR, HDCP_RX_AINFO_ADDR, &ainfo, 1);
}

void Downstream_Rx_write_AN(unsigned char *pAN)
{
	DDC_Write(HDCP_RX_ADDR, HDCP_RX_AN_ADDR, pAN, 8);
}

void Downstream_Rx_write_AKSV(unsigned char *pAKSV)
{
	DDC_Write(HDCP_RX_ADDR, HDCP_RX_AKSV_ADDR, pAKSV, 5);
}

unsigned char Downstream_Rx_read_RI(unsigned char *pRI)
{
	// Short Read
	status = DDC_Read(HDCP_RX_ADDR, HDCP_RX_RI_ADDR, pRI, 2);
	if(status != SMBUS_STATUS_Success) {
		LogFormat(ERROR, "Rx Ri read - MCU IIC %d\r\n", (int)status);
		return 0;
	}
	return 1;
}

void Downstream_Rx_read_BSTATUS(unsigned char *pBSTATUS)
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_BSTATUS_ADDR, pBSTATUS, 2);
}

void Downstream_Rx_read_SHA1_HASH(unsigned char *pSHA)
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_SHA1_HASH_ADDR, pSHA, 20);
}

// Retrive a 5 byte KSV at "Index" from FIFO
unsigned char Downstream_Rx_read_KSV_FIFO(unsigned char *pBKSV, unsigned char Index, unsigned char DevCount)
{
	int i, j;

	// Try not to re-read the previous KSV
	if(Index == 0) { // Start
		// Support a max 25 device count because of DDC_Data[] size is 128 byte
		DevCount = (DevCount > 25) ? 25 : DevCount;
		status = DDC_Read(HDCP_RX_ADDR, HDCP_RX_KSV_FIFO_ADDR, DDC_Data, DevCount);
	}
	dr_memcpy(pBKSV, DDC_Data+(Index*5), 5);

	if(status != SMBUS_STATUS_Success) {
		LogFormat(ERROR, "KSV FIFO read - DN DDC %d\r\n", (int)status);
		return 0;
	}

	i = 0;
	j = 0;
	while (i < 5) {
		TempBit = 1;
		while (TempBit) {
			if (pBKSV[i] & TempBit) j++;
			TempBit <<= 1;
		}
		i++;
	}
	if(j != 20) {
		LogFormat(ERROR, "KSV FIFO read - Key Wrong\r\n");
		return 0;
	}	
	return 1;
}

//--------------------------------------------------------------------------------------------------
//
// Downstream EDID Control
//

EDID_STATUS Downstream_Rx_read_EDID(unsigned char *pEDID)
{
	int i;
	unsigned char seg_ptr = 1, BlockCount = 0, Block1Found = 0;

	// =========================================================
	// I. Read the block 0

	status = DDC_Read(EDID_ADDR, 0, pEDID, 128);
	if(status != SMBUS_STATUS_Success) {
		LogFormat(ERROR, " EDID b0 read - DN DDC %d\r\n", (int)status);
		return status;
	}
	LogFormat(DEBUG, "EDID b0 read:\n");
	for(i=0; i<128; ++i) {
		LogFormat(DEBUG, "%d 0x%02X\n", i, (int)pEDID[i]);
	}
	LogFormat(DEBUG, "\r\n");

	// =========================================================
	// II. Read other blocks and find Timing Extension Block

	BlockCount = pEDID[126];
	Block1Found = 0;
	if(BlockCount >= 8)
	{
		BlockCount = 1;	
	}

	if(BlockCount != 0)
	{
		for(seg_ptr = 1; seg_ptr <= BlockCount; ++seg_ptr)
		{
			status = DDC_Read(EDID_ADDR, (seg_ptr & 0x01) << 7, DDC_Data, 128);
			
			if(status != SMBUS_STATUS_Success) {
				LogFormat(ERROR, " EDID bi read - DN DDC %d\r\n", (int)status);
				return status;
			}
	
			// Check EDID
			if(DDC_Data[0] == 0x02 && Block1Found == 0) {
				Block1Found = 1;
				dr_memcpy(&pEDID[128], DDC_Data, 128);
			}
			
			LogFormat(DEBUG, "EDID b%d read:\n", (int)seg_ptr);
			for(i=0; i<128; ++i) {
				LogFormat(DEBUG, "%d 0x%02X\n", i, (int)DDC_Data[i]);
			}
			LogFormat(DEBUG, "\r\n");
		}
		LogFormat(DEBUG, "\r\n");
	}
		
	if(Block1Found) {
		pEDID[126] = 1;
	}
	else {
		pEDID[126] = 0;
	}

	return EDID_STATUS_Success;
}

//==================================================================================================
//
// Private Functions
//

SMBUS_STATUS DDC_Write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// return 0; for success
	// return 2; for No_ACK
	// return 4; for Arbitration
	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	unsigned char status = 0;

	status = i2c_write(IICAddr, ByteAddr, Data, Size);

	return status;
}

SMBUS_STATUS DDC_Read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// return 0; for success
	// return 2; for No_ACK
	// return 4; for Arbitration
	/////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned char status = 0;

	status = i2c_read(IICAddr, ByteAddr, Data, Size);

	return status;
}
