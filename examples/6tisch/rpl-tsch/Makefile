
CONTIKI_PROJECT = udp-client udp-server
all: $(CONTIKI_PROJECT)
PLATFORMS_EXCLUDE = sky nrf52dk native simplelink
CONTIKI=../../..
MAKE_MAC = MAKE_MAC_TSCH
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell
#MODULES += $(CONTIKI_NG_SERVICES_DIR)/orchestra
#APPS+=orchestra
MODULES += core/net/mac/tsch
MODULES += $(CONTIKI_NG_MAC_DIR)/tsch/sixtop
include $(CONTIKI)/Makefile.include

