#ifndef __HDMI_EP952_H__
#define __HDMI_EP952_H__

typedef enum {
	AUDIO_SEL_I2S = 0,
	AUDIO_SEL_SPDIF
} HDMI_EP952_AUDIO_SELECT;

typedef enum {
	AUDIO_SF_32000Hz = 1,
	AUDIO_SF_44100Hz,
	AUDIO_SF_48000Hz
} HDMI_EP952_AUDIO_SAMPLERATE;

typedef enum {
	HDMI_EP952_640x480P_60Hz_4_3    = 1,
	HDMI_EP952_720x240P_60Hz_4_3    = 8,
	HDMI_EP952_720x240P_60Hz_16_9   = 9,
	HDMI_EP952_720x288P_50Hz_4_3    = 23,
	HDMI_EP952_720x288P_50Hz_16_9   = 24,
	HDMI_EP952_720x480P_60Hz_4_3    = 2,
	HDMI_EP952_720x480I_60Hz_4_3    = 6,
	HDMI_EP952_720x480I_60Hz_16_9   = 7,
	HDMI_EP952_720x480P_60Hz_16_9   = 3,
	HDMI_EP952_720x576I_50Hz_4_3    = 21,
	HDMI_EP952_720x576I_50Hz_16_9	= 22,
	HDMI_EP952_720x576P_50Hz_4_3    = 17,
	HDMI_EP952_720x576P_50Hz_16_9	= 18,
	HDMI_EP952_1280x720P_50Hz_16_9  = 19,
	HDMI_EP952_1280x720P_60Hz_16_9  = 4,
	HDMI_EP952_1440x480P_60Hz_4_3   = 14,
	HDMI_EP952_1440x480P_60Hz_16_9  = 15,
	HDMI_EP952_1440x576P_50Hz_4_3   = 29,
	HDMI_EP952_1440x576P_50Hz_16_9  = 30,
	HDMI_EP952_1920x1080I_50Hz_16_9 = 20,
	HDMI_EP952_1920x1080I_60Hz_16_9 = 5,
	HDMI_EP952_1920x1080P_24Hz_16_9 = 32,
	HDMI_EP952_1920x1080P_25Hz_16_9 = 33,
	HDMI_EP952_1920x1080P_30Hz_16_9 = 34,
	HDMI_EP952_1920x1080P_50Hz_16_9 = 31,
	HDMI_EP952_1920x1080P_60Hz_16_9 = 16,
	HDMI_EP952_2880x480I_60Hz_4_3   = 10,
	HDMI_EP952_2880x480I_60Hz_16_9  = 11,
	HDMI_EP952_2880x576I_50Hz_4_3   = 25,
	HDMI_EP952_2880x576I_50Hz_16_9  = 26,
	HDMI_EP952_2880x240P_60Hz_4_3   = 12,
	HDMI_EP952_2880x240P_60Hz_16_9  = 13,
	HDMI_EP952_2880x288P_50Hz_4_3   = 27,
	HDMI_EP952_2880x288P_50Hz_16_9  = 28,
} HDMI_EP952_VIDEO_OUTPUT_RESOLUTION;

typedef enum {
	HDMI_EP952_RGB444 = 0,
	HDMI_EP952_YUV444 = 1,
	HDMI_EP952_YUV422 = 2
} HDMI_EP952_VIDEO_INPUT_FORMAT;

typedef struct {
	HDMI_EP952_AUDIO_SELECT     select;
	HDMI_EP952_AUDIO_SAMPLERATE sampleRate;
} HDMI_EP952_AUDIO_FMT;

typedef struct {
	HDMI_EP952_VIDEO_OUTPUT_RESOLUTION resolution;
	HDMI_EP952_VIDEO_INPUT_FORMAT      format;
} HDMI_EP952_VIDEO_FMT;

typedef enum {
	HDMI_EP952_GET_CAPACITY   = 0x1001,
	HDMI_EP952_SET_AUDIO_FMT  = 0x1002,
	HDMI_EP952_SET_VIDEO_FMT  = 0x1003
} HDMI_EP952_CMD;

#endif
