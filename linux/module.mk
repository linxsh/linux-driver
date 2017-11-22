base-objs += ../src/common/common.o
base-objs += ../src/common/log.o

base-objs += ../src/hdmi/driver.o
base-objs += ../src/hdmi/ep952_core.o
base-objs += ../src/hdmi/i2c.o
base-objs += ../src/hdmi/ip/DDC_If.o
base-objs += ../src/hdmi/ip/Edid.o
base-objs += ../src/hdmi/ip/EP952api.o
base-objs += ../src/hdmi/ip/EP952Controller.o
base-objs += ../src/hdmi/ip/EP952_If.o
base-objs += ../src/hdmi/ip/EP952SettingsData.o
base-objs += ../src/hdmi/ip/HDCP.o

EXTRA_CFLAGS += -I${DR_SRC_PATH}/configs
