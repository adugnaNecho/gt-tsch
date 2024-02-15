#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "sys/node-id.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "sys/log.h"
#include "sys/energest.h"
#include "net/ipv6/uip-icmp6.h"
#include "services/orchestra/orchestra.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source
#define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready
#define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added
#define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed

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

static inline unsigned long
 to_seconds(uint64_t time)
{
  return (unsigned long)(time / ENERGEST_SECOND);
}
 
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
         static struct etimer periodic_timer;
         static struct etimer start_timer;
         static struct etimer end_timer;
         static uip_ipaddr_t dest_ipaddr;
         uip_ipaddr_t dest_ipaddr1;
         PROCESS_BEGIN();
         NETSTACK_MAC.on();
          energest_init();
         orchestra_init();
        

         
         
         simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);
      
         etimer_set(&end_timer,(60*20)*CLOCK_SECOND);
         etimer_set(&start_timer,(60*10)*CLOCK_SECOND+ CLOCK_SECOND*node_id*2 +CLOCK_SECOND/node_id);
         PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&start_timer));
        
         static int check=0;
         static int i=0;
         for(i=1; i<20; i++)
         {
             if(check==0)
             {
                 if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
                 {
                     check=1;
                     break;
                 }
                 
             }
             etimer_set(&periodic_timer,CLOCK_SECOND*5);
             PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
         }
      
          etimer_set(&periodic_timer,CLOCK_SECOND*node_id*2 +CLOCK_SECOND/node_id);
      
       static int count=0;
         
         while(1) 
         {
                            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
                            if(seq_id==0)
                            {
                            
                                seq_id=(unsigned long)node_id * 1000;

                        
                            }
                      
                           char buf[12];
                           memset(buf,'\0',12);


                          seq_id++;
							
						  count++;
                    
                            unsigned long limit = (unsigned long)(node_id) * 1000;
                            limit = limit + 1000;
                            if(seq_id >= limit )
                            {
                      
                                seq_id = (unsigned long)node_id * 1000;
                          
                            }
							
                      
                            sprintf(buf, "%lu", seq_id);
                      
                     
                            int size=strlen(buf);
                  
                            if(size<6)
                            {
                                  int numOfZero=6-size;
                                  uint8_t buf1[7];
                                  memset(buf1,'\0',7);
                                  memset(buf1,0x30,6);
                                  memcpy(buf1+numOfZero, buf,size);
                                  memcpy(buf,buf1,6);
                          
                            }
                            printf("SendData %s\n", buf);
                            
                             
                             if(check==0)
                            {
                                    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
                                    {
                                                 check=1;
                                   
                                    }
                            }    
                            
                            
                            if(check==1) 
                            {
                                    simple_udp_sendto(&udp_conn, buf, 6, &dest_ipaddr);
                            } 

							
							/*if(count>450)
							{
								break;
								
							}
							*/
							
                            etimer_set(&periodic_timer, CLOCK_SECOND*(1/packet_generation_rate));
                            
                             
                            if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr1))
                            {
                                    NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr);
                                    check=1;
                                                 
                            }
                
                           if(etimer_expired(&end_timer))
                              
                            {
                                    energest_flush();
                                    printf("dutycycle:");
               
                                   unsigned long radio_on=  to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)) +
                                   to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT));
                                   printf("%d  ",(int)radio_on);
                                   double test =  (double)radio_on /  (double)to_seconds(ENERGEST_GET_TOTAL_TIME());
                                   test=test*100;
                                   printf("%d  ",(int)to_seconds(ENERGEST_GET_TOTAL_TIME()));  
                                   int final=(int)(test);
                                   printf("  %d\n",final);
                                   printf("drops   %d\n",drops);
                                    printf("icmpPackets %d\n", IcmpPackets);
                                    printf("parentChange %d\n",parent_change);
                                   break;
                               
                            }
            }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
