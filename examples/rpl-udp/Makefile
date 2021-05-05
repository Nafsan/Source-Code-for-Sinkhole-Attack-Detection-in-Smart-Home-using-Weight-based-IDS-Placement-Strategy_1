CONTIKI_PROJECT = udp-client udp-server udp-client-malicious
all: $(CONTIKI_PROJECT)
CONTIKI_NO_NET=1
CONTIKI=../..
MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
TARGET_LIBFILES = -lm
CFLAGS += -ffunction-sections
 LDFLAGS += -Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__
include $(CONTIKI)/Makefile.include
