EXEC = pedometerstandalone$(STATIC_APP_SUFFIX)

MK_NAME = $(notdir $(CURDIR)/$(firstword $(MAKEFILE_LIST)))

CROSS = $(ANDROID_ROOT)/prebuilt/linux-x86/toolchain/arm-eabi-4.4.0/bin/arm-eabi-
COMP  = $(CROSS)gcc
LINK  = $(CROSS)gcc 

MLSDK_ROOT = ../../..
MLLITE_DIR = $(MLSDK_ROOT)/mllite
MPL_DIR = $(MLSDK_ROOT)/mldmp
MLPLATFORM_DIR = $(MLSDK_ROOT)/platform
ML_COMMON_DIR = ../../common

OBJFOLDER=$(CURDIR)/obj

include $(MLSDK_ROOT)/Android-common.mk

CFLAGS += $(CMDLINE_CFLAGS)
CFLAGS += -Wall -fpic
CFLAGS += -mthumb-interwork 
CFLAGS += -fno-exceptions -ffunction-sections -funwind-tables 
CFLAGS += -fstack-protector -fno-short-enums -fmessage-length=0
CFLAGS += -D_REENTRANT -DLINUX -DANDROID
CFLAGS += -I$(MLLITE_DIR) -I$(MPL_DIR) -I$(MLPLATFORM_DIR)/include 
CFLAGS += -I.. -I$(ML_COMMON_DIR) -I$(MLSDK_ROOT)/mltools/debugsupport

LLIBS = -lc -lm -lutils -lcutils -lgcc

PRE_LFLAGS += -Wl,-T,$(ANDROID_ROOT)/build/core/armelf.x
PRE_LFLAGS += \
	$(ANDROID_ROOT)/out/target/product/$(PRODUCT)/obj/lib/crtend_android.o \
	$(ANDROID_ROOT)/out/target/product/$(PRODUCT)/obj/lib/crtbegin_dynamic.o

LFLAGS += $(CMDLINE_LFLAGS)
LFLAGS += -rdynamic -nostdlib -fpic
LFLAGS += -Wl,--gc-sections -Wl,--no-whole-archive
LFLAGS += -Wl,-dynamic-linker,/system/bin/linker 
LFLAGS += -L$(MPL_DIR)/mpl/$(TARGET) -L$(MLLITE_DIR)/mpl/$(TARGET)
LFLAGS += $(ANDROID_LINK)

LRPATH += -Wl,-rpath,$(ANDROID_ROOT)/out/target/product/$(PRODUCT)/obj/lib:$(ANDROID_ROOT)/out/target/product/$(PRODUCT)/system/lib:$(MLPLATFORM_DIR)/linux:$(MPL_DIR)/mpl/$(TARGET):$(MLLITE_DIR)/mpl/$(TARGET):

VPATH += $(ML_COMMON_DIR) $(MLLITE_DIR) ..

####################################################################################################
## sources

ML_LIBS = \
	$(MPL_DIR)/mpl/$(TARGET)/$(LIB_PREFIX)$(MPL_LIB_NAME).$(STATIC_LIB_EXT) \
	$(MLLITE_DIR)/mpl/$(TARGET)/$(LIB_PREFIX)$(MLLITE_LIB_NAME).$(STATIC_LIB_EXT) \
	$(MLPLATFORM_DIR)/linux/$(LIB_PREFIX)$(MLPLATFORM_LIB_NAME).$(STATIC_LIB_EXT)

ML_SRCS = \
	pedometerstandalone.c \
	../pedometer_lp_example.c \
	$(ML_COMMON_DIR)/mlsetup.c \
	$(ML_COMMON_DIR)/helper.c \
	$(ML_COMMON_DIR)/mlerrorcode.c \
	$(ML_COMMON_DIR)/gestureMenu.c \
	$(MLLITE_DIR)/mlmath.c

ML_OBJS = $(addsuffix .o, $(ML_SRCS))
ML_OBJS_DST = $(addprefix $(OBJFOLDER)/,$(notdir $(ML_OBJS)))

####################################################################################################
## rules

.PHONY: all clean cleanall install

all: $(EXEC) $(MK_NAME)

$(EXEC) : $(OBJFOLDER) $(ML_OBJS_DST) $(ML_LIBS) $(MK_NAME)
	@$(call echo_in_colors, "\n<linking $(EXEC) with objects $(ML_OBJS_DST) and libraries $(ML_LIBS)\n")
	$(LINK) $(PRE_LFLAGS) $(ML_OBJS_DST) -o $(EXEC) $(LFLAGS) $(ML_LIBS) $(LLIBS) $(LRPATH) 

$(OBJFOLDER) : 
	@$(call echo_in_colors, "\n<creating object's folder 'obj/'>\n")
	mkdir obj

$(ML_OBJS_DST) : $(OBJFOLDER)/%.c.o : %.c $(MK_NAME)
	@$(call echo_in_colors, "\n<compile $< to $(OBJFOLDER)/$(notdir $@)>\n")
	$(COMP) $(CFLAGS) $(ANDROID_INCLUDES) $(ML_INCLUDES) -o $@ -c $<

clean : 
	rm -fR $(OBJFOLDER)

cleanall : 
	rm -fR $(EXEC) $(OBJFOLDER)

install : $(EXEC)
	cp -f $(EXEC) $(INSTALL_DIR)
