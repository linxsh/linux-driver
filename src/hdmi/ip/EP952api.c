#include "EP952api.h"
#include "EP952Controller.h"  // HDMI Transmiter
#include "log.h"

/*
============================================================================
	need use customer main chip function
	1. EP952 IIC/DDC - use customer's IIC function
		DDC_If.c	=>	DDC_Write(......) , DDC_Read(.......)
		EP952_If.c	=>	IIC_Write(......) , IIC_Read(.......)
		### customer' IIC function must can check IIC error (no ack, write error, read error) ###
   ============================================================================
	EP952 code process
	1. initial EP952 variable
		EP_HDMI_Init();
	2. set video interface and timing, timing need modify to fit with customer's require
		EP_HDMI_Set_Video_Timing( 4 ); //720P60Hz
	3. set audio interface
		EP_HDMI_Set_Audio_Fmt(AUD_I2S, AUD_SF_48000Hz); // IIS input , 48KHz 
	4.
		need use [timer] or [thread] to run EP952Controller_Task(); and EP952Controller_Timer(); every " 10ms "
	5. Task process
		EP_HDMI_Init();
		EP_HDMI_Set_Video_Timing( 4 );				        // 720P60Hz
		EP_HDMI_Set_Audio_Fmt(AUD_I2S, AUD_SF_48000Hz);	   // IIS input , 48KHz
		while (1) {
			if(Audio_Change) {
				EP_HDMI_Set_Audio_Fmt( XX, XX );
			}
			if(Video_Change) {
				EP_HDMI_Set_Video_Timing( XX );
			}
			EP952Controller_Timer();
			EP952Controller_Task();
		}
============================================================================
*/

EP952C_REGISTER_MAP EP952C_Registers;


void EP_EP952_Reset(void)
{
	// If use customer's GPIO to reset EP952 (optional)
	// 1. GPIO set to low level
	// 2. delay 5ms
	// 3. GPIO set to high level

}

void  EP_HDMI_Set_Audio_Fmt(HDMI_AudFmt_t  Audfmt, HDMI_AudFreq  Audfreq)
{
	///////////////////////////////////////////////////////////////
	// need to select EP952 Audio interface(IIS or SPDIF) and Audio sample frequency(32KHz, 44.1KHz, 48KHz)
	//
	//  [first parameter]:
	//		AUD_I2S
	//		AUD_SPDIF
	//  [second parameter]:
	//		AUD_SF_32000Hz
	//		AUD_SF_44100Hz
	//		AUD_SF_48000Hz
	//  [Example]:
	//		EP_HDMI_SetAudFmt(AUD_I2S, AUD_SF_44100Hz);	    // I2S input,   Sample Frequency = 44.1KHz 
	//		EP_HDMI_SetAudFmt(AUD_SPDIF, AUD_SF_48000Hz);	// SPDIF input, Sample Frequency = 48KHz 
	///////////////////////////////////////////////////////////////

	if (Audfmt == AUD_I2S) {
		EP952C_Registers.Audio_Interface = 0x18;		// 2 channel IIS
	} else {
		EP952C_Registers.Audio_Interface = 0x10;		// SPDIF
	}
	EP952C_Registers.Audio_Input_Format = Audfreq;		// set Audio frequency
	EP952C_Registers.Audio_change = 1;					// Audio setting change flag
	LogFormat(INFO, "%s %d: audio interface 0x%x, format %d\n",
			__FUNCTION__, __LINE__, EP952C_Registers.Audio_Interface, EP952C_Registers.Audio_Input_Format);
}

void EP_HDMI_Set_Video_Timing(unsigned char VIC)
{
	// need to select EP952 video input interface
	unsigned char DK    = (0x4<<5)&0xf;
	unsigned char DKEN  = (0x1<<4)&0xf;
	unsigned char DSEL  = (0x1<<3)&0xf;
	unsigned char BSEL  = (0x1<<2)&0xf;
	unsigned char EDGE  = (0x1<<1)&0xf;
	unsigned char FMT12 = (0x0<<0)&0xf;
	unsigned char SYNC    = (0x0<<0)&0xf;
	unsigned char VIN_FMT = (0x1<<1)&0xf;

	// reference to Video_Interface[0].jpg;
	EP952C_Registers.Video_Interface[0] = DK|DKEN|DSEL|BSEL|EDGE|FMT12;
	// reference to Video_Interface[1].jpg;
	EP952C_Registers.Video_Interface[1] = SYNC|VIN_FMT;
	/*
	0: No video input
	1: CEA-861D 640 x 480		(60Hz or 59.94Hz)	Progressive 4:3
	2: CEA-861D 720 x 480		(60Hz or 59.94Hz)	Progressive 4:3		<-- 480P  60Hz (4:3)
	3: CEA-861D 720 x 480		(60Hz or 59.94Hz)	Progressive 16:9	<-- 480P  60Hz (16:9)
	4: CEA-861D 1280 x 720		(60Hz or 59.94Hz)	Progressive 16:9	<-- 720P  60Hz
	5: CEA-861D 1920 x 1080		(60Hz or 59.94Hz)	Interlaced 16:9		<-- 1080i 60Hz
	6: CEA-861D 720(1440) x 480	(60Hz or 59.94Hz)	Interlaced 4:3		<-- 480i  60Hz (4:3)
	7: CEA-861D 720(1440) x 480	(60Hz or 59.94Hz)	Interlaced 16:9		<-- 480i  60Hz (16:9)
	8: CEA-861D 720(1440) x 240	(60Hz or 59.94Hz)	Progressive 4:3
	9: CEA-861D 720(1440) x 240	(60Hz or 59.94Hz)	Progressive 16:9
	10: CEA-861D 2880 x 480		(60Hz or 59.94Hz)	Interlaced 4:3
	11: CEA-861D 2880 x 480		(60Hz or 59.94Hz)	Interlaced 16:9
	12: CEA-861D 2880 x 240		(60Hz or 59.94Hz)	Progressive 4:3
	13: CEA-861D 2880 x 240		(60Hz or 59.94Hz)	Progressive 16:9
	14: CEA-861D 1440 x 480		(60Hz or 59.94Hz)	Progressive 4:3
	15: CEA-861D 1440 x 480		(60Hz or 59.94Hz)	Progressive 16:9
	16: CEA-861D 1920 x 1080	(60Hz or 59.94Hz)	Progressive 16:9	<-- 1080P 60Hz
	17: CEA-861D 720 x 576		(50Hz)				Progressive 4:3		<-- 576P  50Hz (4:3)
	18: CEA-861D 720 x 576		(50Hz)				Progressive 16:9	<-- 576P  50Hz (16:9)
	19: CEA-861D 1280 x 720		(50Hz)				Progressive 16:9
	20: CEA-861D 1920 x 1080	(50Hz)				Interlaced 16:9
	21: CEA-861D 720(1440) x 576(50Hz)				Interlaced 4:3		<-- 576i  50Hz (4:3)
	22: CEA-861D 720(1440) x 576(50Hz)				Interlaced 16:9		<-- 576i  50Hz (16:9)
	23: CEA-861D 720(1440) x 288(50Hz)				Progressive 4:3
	24: CEA-861D 720(1440) x 288(50Hz)				Progressive 16:9
	25: CEA-861D 2880 x 576		(50Hz)				Interlaced 4:3
	26: CEA-861D 2880 x 576		(50Hz)				Interlaced 16:9
	27: CEA-861D 2880 x 288		(50Hz)				Progressive 4:3
	28: CEA-861D 2880 x 288		(50Hz)				Progressive 16:9
	29: CEA-861D 1440 x 576		(50Hz)				Progressive 4:3
	30: CEA-861D 1440 x 576		(50Hz)				Progressive 16:9
	31: CEA-861D 1920 x 1080	(50Hz)				Progressive 16:9
	32: CEA-861D 1920 x 1080	(24Hz)				Progressive 16:9
	33: CEA-861D 1920 x 1080	(25Hz)				Progressive 16:9
	34: CEA-861D 1920 x 1080	(30Hz)				Progressive 16:9
	*/
	// need to select video timing, timing detail description as below
	EP952C_Registers.Video_Input_Format[0] = VIC;
	EP952C_Registers.Video_change = 1;
	LogFormat(INFO, "%s %d: video interface0 0x%x, interface1 0x%x, format %d\n",
			__FUNCTION__, __LINE__,
			EP952C_Registers.Video_Interface[0], EP952C_Registers.Video_Interface[1], EP952C_Registers.Video_Input_Format[0]);
}

void EP_HDMI_Init(void)
{
	EP952Controller_Initial(&EP952C_Registers);
}

void EP_HDMI_Process(void)
{
	EP952Controller_Timer();
	EP952Controller_Task();
}
