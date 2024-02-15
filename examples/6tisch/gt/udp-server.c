/**
 * This file contains the client code for the GT-TSCH protocol.
 * Developed by Omid Tavallaie.
 * 
 * The GT-TSCH protocol aims to enhance synchronous communication in low-power IoT networks
 * by implementing efficient time-slotted channel hopping mechanisms. This client
 * code is designed to run on Contiki-NG operating system, leveraging its networking
 * stack to demonstrate the protocol's capabilities in a practical scenario.
 */


#include "contiki.h"
#include "node-id.h"
#include "net/routing/rpl-lite/rpl.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/mac/tsch/sixtop/sixp.h"
#include "net/mac/tsch/sixtop/sixp-nbr.h"
#include "net/mac/tsch/sixtop/sixp-pkt.h"
#include "net/mac/tsch/sixtop/sixp-trans.h"
#include "net/routing/routing.h"
#include "random.h"
#include "sf-simple.h"
#include "sys/node-id.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

//Printing the packet ID
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	char buf[12];
    memset(buf,'\0',10);
    memcpy(buf,data,6);
    printf("RecvData %s\n", buf);
}

PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

 
    NETSTACK_ROUTING.root_start();
    NETSTACK_MAC.on();
    
	//initializing SixTop
	sixtop_init();

	//Using GT-TSCH as the scheduling function
	sixtop_add_sf(&sf_simple_driver);

    simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL, UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/