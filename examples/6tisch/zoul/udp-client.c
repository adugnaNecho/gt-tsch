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

//#include "cpu.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
//#include "dev/uart.h"
#include "dev/button-sensor.h"
//#include "dev/adc-zoul.h"
//#include "dev/zoul-sensors.h"
#include "dev/watchdog.h"
#include "dev/serial-line.h"
//#include "dev/sys-ctrl.h"
#include "net/netstack.h"
//#include "net/rime/broadcast.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
/*
#define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source
#define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready
#define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added
#define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed
*/
#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (300*CLOCK_SECOND)
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

static void send_duty_cycle(char* buf)
{
        energest_flush();
        memset(buf,'\0',20);
        buf[0]='B';
        strcpy(buf+1,"dutycycle ");

        unsigned long radio_on=  to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)) +
        to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT));
                               //printf("%d  ",(int)radio_on);
        double test =  (double)radio_on /  (double)to_seconds(ENERGEST_GET_TOTAL_TIME());
        test=test*100;
                               // printf("%d  ",(int)to_seconds(ENERGEST_GET_TOTAL_TIME()));  
                               
        int test1 = (int)(test);
        unsigned long final1=test1;
                            
        printf("duty cycle=%d\n",(int)final1);
                               
        char buffConvert[10];
        memset(buffConvert,'\0',10);
        sprintf(buffConvert, "%lu", final1);
        int size1=strlen(buffConvert);
                
         if(size1<5)
        { 
                int numOfZero=5-size1;
                uint8_t buf_duty[5];
                memset(buf_duty,'\0',5);
                memset(buf_duty,0x30,5);
                memcpy(buf_duty+numOfZero, buffConvert,size1);
                memcpy(buf+11,buf_duty,5);
        }
}


static void send_drops(char* buf)
{
        
        memset(buf,'\0',20);
        buf[0]='C';
        strcpy(buf+1,"drops ");
                               
      
        unsigned long final1=(int)(drops);
                            
        printf("drops=%d\n",drops);
                               
        char buffConvert[10];
        memset(buffConvert,'\0',10);
        sprintf(buffConvert, "%lu", final1);
        int size1=strlen(buffConvert);
        if(size1<5)
        { 
                int numOfZero=5-size1;
                uint8_t buf_duty[5];
                memset(buf_duty,'\0',5);
                memset(buf_duty,0x30,5);
                memcpy(buf_duty+numOfZero, buffConvert,size1);
                memcpy(buf+7,buf_duty,5);
        }
}

static void send_icmp_packets(char* buf)
{
        
        memset(buf,'\0',20);
        buf[0]='D';
        strcpy(buf+1,"icmpPackets ");
                               
        unsigned long final1=(int)(IcmpPackets);
                            
        printf("icmpPackets=%d\n",IcmpPackets);
                               
        char buffConvert[10];
        memset(buffConvert,'\0',10);
        sprintf(buffConvert, "%lu", final1);
        int size1=strlen(buffConvert);
        if(size1<5)
        { 
                int numOfZero=5-size1;
                uint8_t buf_duty[5];
                memset(buf_duty,'\0',5);
                memset(buf_duty,0x30,5);
                memcpy(buf_duty+numOfZero, buffConvert,size1);
                memcpy(buf+13,buf_duty,5);
        }
}

static void send_parent_change(char* buf)
{
        
        memset(buf,'\0',20);
        buf[0]='E';
        strcpy(buf+1,"parentChange ");
                               
        unsigned long final1=(int)(parent_change);
                            
        printf("parent change=%d\n",parent_change);
                               
        char buffConvert[10];
        memset(buffConvert,'\0',10);
        sprintf(buffConvert, "%lu", final1);
        int size1=strlen(buffConvert);
        if(size1<5)
        { 
                int numOfZero=5-size1;
                uint8_t buf_duty[5];
                memset(buf_duty,'\0',5);
                memset(buf_duty,0x30,5);
                memcpy(buf_duty+numOfZero, buffConvert,size1);
                memcpy(buf+14,buf_duty,5);
        }
}

/*
static void send_parent_changes(char* buf)
{
        
        memset(buf,'\0',30);
        buf[0]='C';
        strcpy(buf+1,"drops ");
                               
      
        unsigned long final1=(int)(drops);
                            
        printf("drops=%d\n",drops);
                               
        char buffConvert[10];
        memset(buffConvert,'\0',10);
        sprintf(buffConvert, "%lu", final1);
        int size1=strlen(buffConvert);
        if(size1<5)
        { 
                int numOfZero=2-size1;
                uint8_t buf_duty[5];
                memset(buf_duty,'\0',5);
                memset(buf_duty,0x30,5);
                memcpy(buf_duty+numOfZero, buffConvert,size1);
                memcpy(buf+7,buf_duty,5);
        }
}
*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  
      
      static struct etimer periodic_timer;
      static struct etimer data_timer;
      static struct etimer start_timer;
      static struct etimer end_timer;
      static uip_ipaddr_t dest_ipaddr;
      uip_ipaddr_t dest_ipaddr1;
      static int number_of_packets;
      static int last_packet;
      last_packet=100;

      static int number_of_end_functions=0;
      static char end_buf[20];
      static int end_len;

      PROCESS_BEGIN();
      NETSTACK_MAC.on();
      sixtop_init();
      energest_init();
      int node_id_zoul=10;
      sixtop_add_sf(&sf_simple_driver);
      etimer_set(&start_timer,(60*9)*CLOCK_SECOND + CLOCK_SECOND*node_id_zoul*2 +CLOCK_SECOND/node_id_zoul);
    //  etimer_set(&end_timer,(60*5)*CLOCK_SECOND);

      simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);
      
      required_slots= convert_rate_to_slots(packet_generation_rate);

    // printf("dddd %d\n",(int)(packet_generation_rate*100));
   
     int rand= random_rand()%20;
     etimer_set(&periodic_timer, CLOCK_SECOND*(180+ node_id_zoul*3));
     seq_id=(node_id_zoul)*1000;


     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

                           
                           
    while(adv_timeslots == 0) 
    {
          //   test=rpl_neighbor_get_lladdr(curr_instance.dag.preferred_parent);
         // struct tsch_neighbor *ts1 = tsch_queue_get_nbr(test);
         struct tsch_neighbor * ts1= tsch_queue_get_time_source();
                          
         if(ts1!=NULL)
         {
                              
                if(ts1->frequency_offset != 0)
                {

                        dtsf_ask_adv_link(&ts1->addr, 2);
                          
                }
 
         }
         
         etimer_set(&periodic_timer,CLOCK_SECOND*node_id_zoul*2 + CLOCK_SECOND/node_id_zoul);
         PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));           
    }
      rand= random_rand()%20;
       etimer_set(&periodic_timer,CLOCK_SECOND*(rand));
       PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
       while(children_channel == 0) 
      {
           //test=rpl_neighbor_get_lladdr(curr_instance.dag.preferred_parent);
          //struct tsch_neighbor *test1 = tsch_queue_get_nbr(test);
          struct tsch_neighbor* test1= tsch_queue_get_time_source();
           if(test1!=NULL )
           {
                if(test1->addr.u8[7]!=0)
                {
                    dtsf_ask_channel(&test1->addr); 
                }
               
           }
               
           etimer_set(&periodic_timer,CLOCK_SECOND*node_id_zoul*2 + CLOCK_SECOND/node_id_zoul);
           PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      }
      
             printf("successfully received children channel %d\n",children_channel);
            //rand= random_rand()%30;

      etimer_set(&periodic_timer,CLOCK_SECOND*(1+node_id_zoul*2));
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
     while(current_number_slots_for_packet_generation == 0  && !etimer_expired(&start_timer))
      {
              // test=rpl_neighbor_get_lladdr(curr_instance.dag.preferred_parent);
             // struct tsch_neighbor *test2 = tsch_queue_get_nbr(test);
            struct tsch_neighbor* test2= tsch_queue_get_time_source();
           if(test2!=NULL && check_ask_uplink==1 )
           {
                  printf("Asking uplink %d\n",required_slots);
                  dtsf_send_add_uplink(&test2->addr, required_slots);
                
           }
           etimer_set(&periodic_timer,CLOCK_SECOND*1 + CLOCK_SECOND/node_id_zoul);
           PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      }
            
       etimer_set(&periodic_timer,CLOCK_SECOND*node_id_zoul*2);
      static int check;
      check=0;
      while(!etimer_expired(&start_timer))
      {
           PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
            if(need_send_back==1  &&  free_uplink_timeslots>0)
            {
                          

                            dtsf_send_add_downlink();
                       
            }
            else
            {
                          
                            if( required_slots > 0 && check_ask_uplink==1)
                            {
                                          
                                          //  test=rpl_neighbor_get_lladdr(curr_instance.dag.preferred_parent);
                                          //  struct tsch_neighbor *test3 = tsch_queue_get_nbr(test);
                                        struct tsch_neighbor* test3= tsch_queue_get_time_source();
                                        if(test3!=NULL)
                                        {
                                                printf("Asking uplink %d\n",required_slots);
                                 
                                                dtsf_send_add_uplink(&test3->addr, required_slots);
                                                            
                                        }
                                 
                            
                            }
            }
             etimer_set(&periodic_timer,CLOCK_SECOND*1+ CLOCK_SECOND/node_id_zoul);
             
             if(check==0)
             {
                 if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
                 {
                     check=1;
                    // printf("heyyyy\n");
                 }
                 
             }
             
             
                       
           
      }
             // rand= random_rand()%20;

      etimer_set(&data_timer,(1+node_id_zoul*2)*CLOCK_SECOND);
      etimer_set(&periodic_timer,CLOCK_SECOND + CLOCK_SECOND/node_id_zoul);
     // energest_init();
      printf("start sending\n");
      number_of_packets=0;
      while(1) 
      {
           PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
         
           if(etimer_expired(&data_timer))
            {
                
                    char buf11[20];
                    memset(buf11,'\0',20);
                    sprintf(buf11, "%lu", seq_id);
                    int size1=strlen(buf11);
        
                    if(size1<6)
                    { 
                         int numOfZero=6-size1;
                         uint8_t buf12[7];
                         memset(buf12,'\0',7);
                         memset(buf12,0x30,6);
                         memcpy(buf12+numOfZero, buf11,size1);
                         memcpy(buf11+1,buf12,6);
                    }
                    
                    
                     buf11[0]='A';
                     
                     
                    leds_on(LEDS_GREEN);
                    printf("SendData %s\n", &buf11[1]);
                    seq_id++;
                    number_of_packets++;
                    etimer_set(&data_timer,CLOCK_SECOND*(1/ packet_generation_rate));
                    
                    if(check==0)
                    {
                        if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
                        {
                                     check=1;
                         //             printf("heyyyy\n");
                        }
                                 
                    }      
                      
                    if( current_number_slots_for_packet_generation>0 &&  check==1)
                    {
                       // printf("yessss\n");
                        simple_udp_sendto(&udp_conn, buf11, 7, &dest_ipaddr);
                    }         
                        
           }
           else
           {

                       if(need_send_back==1  &&  free_uplink_timeslots>0)
                       {
                           
                            dtsf_send_add_downlink();
                       
                       }
                       else
                       {
                          
                            if( required_slots > 0 && check_ask_uplink==1)
                            {
                                        struct tsch_neighbor* test3= tsch_queue_get_time_source();
                                        if(test3!=NULL)
                                        {
                                                printf("Asking uplink %d\n",required_slots);
                                 
                                                dtsf_send_add_uplink(&test3->addr, required_slots);
                                                            
                                        }
                                 
                            
                            }
                       }
                       

               
           }
           
         
        
           etimer_set(&periodic_timer,CLOCK_SECOND*(1/ packet_generation_rate));
           
           if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr1))
            {
                                     NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr);
                                     check=1;
                                   //   printf("heyyyy\n");
            }
           
          //  if(etimer_expired(&end_timer))
          if(number_of_packets >= last_packet)
            {       
                    end_len=0;
                    
                    while(1)
                    {
                            etimer_set(&end_timer,CLOCK_SECOND);
                            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&end_timer));
                           
                            if(number_of_end_functions==0)
                            {
                                   // etimer_set(&end_timer,CLOCK_SECOND);
                                   // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&end_timer));
                                    send_duty_cycle(end_buf);
                                    end_len=13;
                            }
                            
                            if(number_of_end_functions==1)
                            {
                                   // etimer_set(&end_timer,CLOCK_SECOND);
                                   // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&end_timer));
                                    send_drops(end_buf);
                                    end_len=12;
                            }
                            if(number_of_end_functions==2)
                            {
                               // etimer_set(&end_timer,CLOCK_SECOND);
                               // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&end_timer));
                                send_icmp_packets(end_buf);
                                end_len=18;
                            }
                            
                            if(number_of_end_functions==3)
                            {
                               
                                send_parent_change(end_buf);
                                end_len=19;
                            }
                                   
                            if(check==0)
                            {
                                 if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
                                 {
                                        check=1;
                                 }
                                             
                            }      
                                  
                            if( check==1)
                            {
                                simple_udp_sendto(&udp_conn, end_buf, end_len, &dest_ipaddr);
                            }  

                           
                            number_of_end_functions++;
                            
                            if(number_of_end_functions>3)
                            {
                                break;
                            }
                            
                           
                    }
                               
                    break;
            }
            leds_off(LEDS_GREEN);

           
      }

      PROCESS_END();
}

