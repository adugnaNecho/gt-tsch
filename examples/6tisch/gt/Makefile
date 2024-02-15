
CONTIKI_PROJECT = udp-client udp-server
all: $(CONTIKI_PROJECT)
PLATFORMS_EXCLUDE = sky nrf52dk native simplelink
PROJECT_SOURCEFILES += sf-simple.c
CONTIKI=../../..
MAKE_MAC = MAKE_MAC_TSCH
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell
MODULES += core/net/mac/tsch
MODULES += $(CONTIKI_NG_MAC_DIR)/tsch/sixtop
include $(CONTIKI)/Makefile.include

