ifndef MODEL
MODEL=lmm
endif

ifeq ($(MODEL),lmm)
include $(PROPLIB)/lmmdemo.mk
endif

ifeq ($(MODEL),xmm)
include $(PROPLIB)/xmmdemo.mk
endif

ifeq ($(MODEL),xmmc)
include $(PROPLIB)/xmmcdemo.mk
endif
