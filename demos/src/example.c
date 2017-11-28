#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "hdmi_ep952.h"

const char *HDMI_DEVICE = "/dev/hdmi-ep952";

int main()
{
	int ret = 0;
	int handle = open(HDMI_DEVICE, O_RDWR);
	HDMI_EP952_AUDIO_FMT audioFmt;
	HDMI_EP952_VIDEO_FMT videoFmt;

	if (handle < 0) {
		printf("%s %d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	while (1) {
//		audioFmt.select     = AUDIO_SEL_I2S;
//		audioFmt.sampleRate = AUDIO_SF_48000Hz;
//		ret = ioctl(handle, HDMI_EP952_SET_AUDIO_FMT, (void*)&audioFmt);
//		if (ret < 0) {
//			printf("%s %d\n", __FUNCTION__, __LINE__);
//			return -1;
//		}

		videoFmt.format     = HDMI_EP952_YUV422;
		videoFmt.resolution = HDMI_EP952_1920x1080P_30Hz_16_9;
		ret = ioctl(handle, HDMI_EP952_SET_VIDEO_FMT, (void*)&videoFmt);
		if (ret < 0) {
			printf("%s %d\n", __FUNCTION__, __LINE__);
			return -1;
		}
		printf("enter key:");
		getchar();
	}

	close(handle);
}
