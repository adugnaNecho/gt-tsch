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
#include "net/routing/rpl-lite/rpl-neighbor.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/mac/tsch/sixtop/sixp.h"
#include "net/mac/tsch/sixtop/sixp-nbr.h"
#include "net/mac/tsch/sixtop/sixp-pkt.h"
#include "net/mac/tsch/sixtop/sixp-trans.h"
#include "net/routing/routing.h"
#include "sys/energest.h"
#include "random.h"
#include "sf-simple.h"
#include "sys/node-id.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip-icmp6.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (300*CLOCK_SECOND)
unsigned long seq_id=0;

static struct simple_udp_connection udp_conn;


PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);

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
 
// the main process
PROCESS_THREAD(udp_client_process, ev, data)
{
  
      
      static struct etimer periodic_timer;
      static struct etimer data_timer;
      static struct etimer start_timer;
      static struct etimer end_timer;
      static uip_ipaddr_t dest_ipaddr;
      uip_ipaddr_t dest_ipaddr1;

      static int count=1;
	  

      PROCESS_BEGIN();
      NETSTACK_MAC.on();
	  
	  //initialize sixtop and energy calculator
      sixtop_init();
      energest_init();
      sixtop_add_sf(&sf_simple_driver);
	  
	  
	  //initialize timers
      etimer_set(&start_timer,(60*10)*CLOCK_SECOND + CLOCK_SECOND*node_id*2 +CLOCK_SECOND/node_id);
      etimer_set(&end_timer,(60*130)*CLOCK_SECOND);


	  //make the UDP connection with the Server
      simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);
      
	  required_slots = 1;
      // printf("packet generation rate is %d\n",(int)(packet_generation_rate*100));
   
     int rand= random_rand()%20;
     etimer_set(&periodic_timer, CLOCK_SECOND*(240 + node_id));
     seq_id=(node_id)*1000;


     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

	 static  struct tsch_neighbor *parent=NULL;                
	 
	 //Allocating advertisement timeslots
	 while(adv_timeslots == 0) 
	 {
		 
		  //struct tsch_neighbor * ts1= tsch_queue_get_time_source();
		  struct tsch_neighbor *ts1 = tsch_queue_get_nbr(rpl_neighbor_get_lladdr(curr_instance.dag.preferred_parent));
		   
		 if(ts1!=NULL)
		 {

				 parent=ts1;              
				if(ts1->frequency_offset != 0)
				{

						dtsf_ask_adv_link(&ts1->addr, 2);
							
						  
				}
			
 
		 }
		 
		 etimer_set(&periodic_timer,CLOCK_SECOND*3 + CLOCK_SECOND/node_id);
		 PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));           
	 }
   
    rand= random_rand()%20;
    etimer_set(&periodic_timer,CLOCK_SECOND*(rand));
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
	
	//Asking the parent node the frequency channel can be used for communication with children
	while(children_channel == 0) 
	{
          
		dtsf_ask_channel(&parent->addr); 
		etimer_set(&periodic_timer,CLOCK_SECOND*3 + CLOCK_SECOND/node_id);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
	}
      
	printf("successfully received children channel %d\n",children_channel);
           
	etimer_set(&periodic_timer,CLOCK_SECOND*(90+node_id));
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
	
	//Waitung for the allocation of Tx timeslots
	while(current_number_slots_for_packet_generation == 0  && !etimer_expired(&start_timer))
	{

		   
		if(check_ask_uplink==1 )
		{
			printf("Asking uplink %d\n",required_slots);
			dtsf_send_add_uplink(&parent->addr, required_slots);
                
		}
		   
		   
		etimer_set(&periodic_timer,CLOCK_SECOND*5 + CLOCK_SECOND/node_id);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
	}
            
	etimer_set(&periodic_timer,CLOCK_SECOND/node_id);
	static int check;
	check=0;
      
	while(!etimer_expired(&start_timer))
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
		
		//Allocating Rx timeslots to increase uplink capacity of children nodes
		if(need_send_back==1  &&  free_uplink_timeslots>0)
		{

			dtsf_send_add_downlink();
                       
		}
		else
		{
			//When node is not able to allocate Rx timeslots, asks from its parent
			if( required_slots > 0 && check_ask_uplink==1)
			{
				 
				printf("Asking uplink %d\n",required_slots);
				dtsf_send_add_uplink(&parent->addr, required_slots);
              
                            
			}
		}
		
		etimer_set(&periodic_timer,CLOCK_SECOND*4+ CLOCK_SECOND/node_id);
             
		if(check==0)
		{
			if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
			{
				check=1;
                    
			}
                 
		}
             
             
                       
           
	}
             

	etimer_set(&data_timer,(10+node_id*2)*CLOCK_SECOND);
	etimer_set(&periodic_timer,CLOCK_SECOND + CLOCK_SECOND/node_id);
	printf("start sending\n");
    //Start sending data packets  
	while(1) 
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
         
		if(etimer_expired(&data_timer))
		{
            //Creatig packet ID    
			char buf11[12];
			memset(buf11,'\0',12);
			sprintf(buf11, "%lu", seq_id);
			int size1=strlen(buf11);
        
			if(size1<6)
			{ 
				 int numOfZero=6-size1;
				 uint8_t buf12[7];
				 memset(buf12,'\0',7);
				 memset(buf12,0x30,6);
				 memcpy(buf12+numOfZero, buf11,size1);
				 memcpy(buf11,buf12,6);
			}
			
			printf("SendData %s\n", buf11);
			seq_id++;
			count++;
			etimer_set(&data_timer,CLOCK_SECOND*(1/ packet_generation_rate));
                    
			if(check==0)
			{
				if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
				{
					check=1;
				}
						 
			}      
                      
			if( current_number_slots_for_packet_generation>0 &&  check==1)
			{
				simple_udp_sendto(&udp_conn, buf11, 6, &dest_ipaddr);
			}         
                        
		}
		else
		{
		   //Check the TSCH allocation requests received from children
		   if(need_send_back==1  &&  free_uplink_timeslots>0)
		   {
			   
				dtsf_send_add_downlink();
		   
		   }
		   else
		   {
			  
				if( required_slots > 0 && check_ask_uplink==1)
				{
					
					printf("Asking uplink %d\n",required_slots);
					 
					dtsf_send_add_uplink(&parent->addr, required_slots);
												
				}
		   }
                       
		}
           
         
        
		etimer_set(&periodic_timer,CLOCK_SECOND*(1/ packet_generation_rate));
           
	    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr1))
		{
			 NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr);
			 check=1;
							  
		}
           
		if(etimer_expired(&end_timer))
		{
			   //Printing the statistics
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
			   printf("parentChange %d\n",parent_change);
			   printf("icmpPackets   %d\n", IcmpPackets);
			   break;
		}
           
	}

	PROCESS_END();
	  
}