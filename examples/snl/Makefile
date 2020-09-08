PROGRAM_NAME = snl

ROOTDIR ?= $(abspath ../..)

APP_SRC = main.c

ifeq (sim, $(notdir $(MODULE)))
	APP_SRC += sim.c
else
	APP_SRC += bsp.c
endif

include $(ROOTDIR)/module/app.mk
