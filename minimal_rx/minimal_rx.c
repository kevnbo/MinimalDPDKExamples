// Adapted from: https://github.com/DPDK/dpdk/blob/master/examples/skeleton/basicfwd.c
// by Thomas Edwards, Walt Disney Television

/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 0 

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

void DumpHex(const void*, size_t);
void rx_packets(void);
void exit_stats(int);

uint64_t packet_count = 0;

static const struct rte_eth_conf port_conf_default = {
	.rxmode = {
		.max_rx_pkt_len = ETHER_MAX_LEN,
	},
};

void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((const unsigned char*)data)[i]);
		if (((const unsigned char*)data)[i] >= ' ' && ((const unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((const unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
static inline int
port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
	struct rte_eth_conf port_conf = port_conf_default;
	const uint16_t rx_rings = 1, tx_rings = 0;
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	uint16_t q;

	if (!rte_eth_dev_is_valid_port(port))
		return -1;

	/* Configure the Ethernet device. */
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if (retval != 0)
		return retval;

	/* Allocate and set up 1 RX queue per Ethernet port. */
	for (q = 0; q < rx_rings; q++) {
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
				rte_eth_dev_socket_id(port), NULL, mbuf_pool);
		if (retval < 0)
			return retval;
	}

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);
	if (retval < 0)
		return retval;

	/* Display the port MAC address. */
	struct ether_addr addr;
	rte_eth_macaddr_get(port, &addr);
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port,
			addr.addr_bytes[0], addr.addr_bytes[1],
			addr.addr_bytes[2], addr.addr_bytes[3],
			addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	rte_eth_promiscuous_enable(port);

	return 0;
}

// recieve packets
void rx_packets(void)
{
	uint16_t port;
	int i;

	printf("\nCore %u receiving packets. [Ctrl+C to quit]\n",
			rte_lcore_id());

	/* Run until the application is quit or killed. */
	uint64_t last = 0;
	for (;;) {
	  RTE_ETH_FOREACH_DEV(port) {

			struct rte_mbuf *bufs[BURST_SIZE];
			const uint16_t nb_rx = rte_eth_rx_burst(port, 0,
					bufs, BURST_SIZE);

			if (unlikely(nb_rx == 0))
				continue;

			packet_count += nb_rx;

			//printf("received %d packets:\n",nb_rx);

			for(i=0;i<nb_rx;++i){
			  if (packet_count - last > 100000) {
			    last = packet_count;
			    printf("----->processing packet %lu[..%d]:\n",(unsigned long)packet_count, i);
				printf("----->pkt_len=%d\n",bufs[i]->pkt_len);
				//DumpHex(rte_pktmbuf_mtod(bufs[i],char *),bufs[i]->pkt_len);
				uint8_t* p = rte_pktmbuf_mtod(bufs[i], uint8_t*);
				struct ether_hdr* ether = (struct ether_hdr*)p;
				printf("ether type %x\n", ether->ether_type);
				if (ntohs(ether->ether_type) ==0x0806) {
				  printf("looks like arp!\n");
				  char src[32];
				  char dst[32];
				  ether_format_addr(src, sizeof src, &ether->s_addr);
				  ether_format_addr(dst, sizeof dst, &ether->d_addr);
				  p += 14;
				  uint16_t hwtype = p[0] << 0 | p[1];
				  uint16_t prototype = ntohs((p[2] << 0) | p[3]);
				  uint8_t hlen = p[4];
				  uint8_t plen = p[5];
				  uint16_t opcode = (p[6] << 8) | p[7];
				  printf("%s -> %s op code 0x%d hwtype 0x%x ptype 0x%x hlen=%d plen=%d\n", src, dst, opcode, hwtype, prototype, hlen, plen);
				  if (hlen != 6 || plen != 4) {
				    continue;
				  }
				  ether_format_addr(src, sizeof src, (struct ether_addr*)(p + 8));
				  ether_format_addr(dst, sizeof dst, (struct ether_addr*)(p + 8 + hlen+plen));
				  printf("src %s dst %s\n", src, dst);

				  struct in_addr sa, da;
				  da.s_addr = *(uint32_t*)(p+8+hlen);
				  da.s_addr = *(uint32_t*)(p+8+hlen+plen+hlen);
				  printf("src=%s\n", inet_ntoa(sa));
				  printf("dst=%s\n", inet_ntoa(da));
				}
			  }

			  rte_pktmbuf_free(bufs[i]);
			}

		}
	}
}

void exit_stats(int sig)
{
	printf("Caught signal %d\n", sig);
	printf("Total received packets: %lu\n", packet_count);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct rte_mempool *mbuf_pool;
	unsigned nb_ports;
	uint16_t portid;

	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	argc -= ret;
	argv += ret;

	nb_ports = rte_eth_dev_count_avail();
	printf("rte_eth_dev_count_avail()=%d\n",nb_ports);

	/* Creates a new mempool in memory to hold the mbufs. */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	/* Initialize all ports. */
	RTE_ETH_FOREACH_DEV(portid)
		if (port_init(portid, mbuf_pool) != 0)
			rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
					portid);

	signal(SIGINT, exit_stats);
	rx_packets();

	return 0;
}
