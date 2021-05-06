#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/ipv6/uip-sr.h"
#include "net/nbr-table.h"
#include "net/link-stats.h"
#include "sys/node-id.h"
#include "net/ipv6/uiplib.h"
#include "net/netstack.h"
#include "net/routing/rpl-classic/rpl.h"
#include "net/routing/rpl-classic/rpl-private.h"
#include "net/routing/rpl-classic/rpl-dag-root.h"
#include "net/ipv6/simple-udp.h"
#include "dev/button-sensor.h"
#include "sys/log.h"
#include <math.h>
#include "sys/node-id.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY 1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define SEND_INTERVAL (60 * CLOCK_SECOND)

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

  LOG_INFO("Received response '%.*s' from ", datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);
  // LOG_INFO("%d node rank\n", rpl_get_default_instance()->current_dag->rank);
  LOG_INFO("Clock seconds %lu\n", clock_seconds());
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while (1)
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
    {
      /* Send to DAG root */
      LOG_INFO("Sending request %u to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      // snprintf(str, sizeof(str), "hello %d", count);
      // simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      // count++;
      uip_ipaddr_t *parent_ipaddr = rpl_parent_get_ipaddr(default_instance->current_dag->preferred_parent);

      snprintf(str, sizeof(str), "%d %d %d %d", default_instance->current_dag->rank, node_id, (int)parent_ipaddr, default_instance->current_dag->preferred_parent->rank);
      // LOG_INFO("From client node id %d  node rank %d parent id %d parent rank %d\n", node_id, default_instance->current_dag->rank, (int)parent_ipaddr, default_instance->current_dag->preferred_parent->rank);
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      count++;
    }
    else
    {
      LOG_INFO("Not reachable yet\n");
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
