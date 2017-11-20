#include "ep952_core.h"
#include "common.h"
#include "ip/EP952api.h"
#include "hdmi_ep952.h"
#include "log.h"

typedef struct {
	unsigned char inited;
	void *threadId;
	unsigned int threadRuning;
	HDMI_EP952_AUDIO_SELECT     audioSelect;
	HDMI_EP952_AUDIO_SAMPLERATE audioSampleRate;
	HDMI_EP952_VIDEO_RESOLUTION videoResolution;
} DR_HDMI_EP592;

DR_HDMI_EP592 *hdmiEP952 = NULL;

static void ep952CoreThread(void *priv)
{
	while (hdmiEP952->threadRuning) {
		EP_HDMI_Process();
		dr_thread_delay(10);
	}
}

int ep952CoreInit(void)
{
	int ret = 0;
	hdmiEP952 = dr_mallocz(sizeof(DR_HDMI_EP592));

	EP_HDMI_Init();
	EP_EP952_Reset();

	hdmiEP952->threadRuning = 1;
	ret = dr_thread_create("hdmi,ep952", &hdmiEP952->threadId, ep952CoreThread, NULL);
	if (ret < 0) {
		LogFormat(ERROR, "%s %d\n", __FUNCTION__, __LINE__);
		return -1;
	}
	printk("%s %d\n", __FUNCTION__, __LINE__);

	return 0;
}

int ep952CoreDestory(void)
{
	hdmiEP952->threadRuning = 0;
	dr_thread_delete(hdmiEP952->threadId);
	dr_free(hdmiEP952);
	hdmiEP952 = NULL;
	printk("%s %d\n", __FUNCTION__, __LINE__);
	return 0;
}

void* ep952CoreOpen(void)
{
	hdmiEP952->inited = 1;
	return (void*)hdmiEP952;
}

int ep952CoreClose(void* priv)
{
	if (priv != hdmiEP952)
		return -1;

	return 0;
}

int ep952CoreIoctl(void* priv, unsigned int cmd, void *arg)
{
	switch (cmd) {
		case HDMI_EP952_SET_AUDIO_FMT:
			{
				HDMI_EP952_AUDIO_FMT fmt;

				if (copy_from_user((void*)&fmt,
							(void*)arg, sizeof(HDMI_EP952_SET_AUDIO_FMT))) {
					LogFormat(ERROR, "%s %d\n", __FUNCTION__, __LINE__);
					return -1;
				}

				hdmiEP952->audioSelect     = fmt.select;
				hdmiEP952->audioSampleRate = fmt.sampleRate;
				EP_HDMI_Set_Audio_Fmt(fmt.select, fmt.sampleRate);
			}
			break;
		case HDMI_EP952_SET_VIDEO_FMT:
			{
				HDMI_EP952_VIDEO_FMT fmt;

				if (copy_from_user((void *)&fmt,
							(void *)arg, sizeof(HDMI_EP952_VIDEO_FMT))) {
					LogFormat(ERROR, "%s %d\n", __FUNCTION__, __LINE__);
					return -1;
				}
				hdmiEP952->videoResolution = fmt.resolution;
				EP_HDMI_Set_Video_Timing(fmt.resolution);
			}
			break;
		default:
			LogFormat(ERROR, "%s %d\n", __FUNCTION__, __LINE__);
			return -1;
	}

	return 0;
}
