#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "sys/node-id.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (60 * CLOCK_SECOND)
unsigned long seq_id=0;

static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{


}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count;
  
  uip_ipaddr_t dest_ipaddr;
  PROCESS_BEGIN();
  NETSTACK_MAC.on();


  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
     
     
      if(seq_id==0)
            {
                
                seq_id=(unsigned long)node_id * 1000;

            
            }
          char buf[12];
          memset(buf,'\0',12);


          seq_id++;
         //  printf("seq=%lu\n",seq_id);
          unsigned long limit = (unsigned long)(node_id) * 1000;
          limit = limit + 1000;
          if(seq_id >= limit )
          {
            //  printf("seq=%lu   limit=%lu\n",(unsigned long)seq_id,(unsigned long)limit);
              seq_id = (unsigned long)node_id * 1000;
              
          }
          
          
          sprintf(buf, "%lu", seq_id);
          
          //omid/////
          int size=strlen(buf);
        //printf("size=%d\n",size);
          if(size<6)
          {
              int numOfZero=6-size;
              uint8_t buf1[7];
              memset(buf1,'\0',7);
              memset(buf1,0x30,6);
              memcpy(buf1+numOfZero, buf,size);
              memcpy(buf,buf1,6);
              
          }
          printf("SendData  %s\n", buf);
         simple_udp_sendto(&udp_conn, buf, 6, &dest_ipaddr);
      
      
      
      count++;
    } else {
     // printf("Not reachable yet\n");
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, 2 * CLOCK_SECOND);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/