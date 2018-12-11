#include "ns.h"

#include <inc/lib.h>

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	int r;

	while (true) {
		int32_t value = ipc_recv(&ns_envid, (void*) &nsipcbuf, NULL);

		if (value == NSREQ_OUTPUT) {
			struct jif_pkt* pkt = &nsipcbuf.pkt;

			if ((r = sys_transmit_package((struct package_desc*) &(pkt->jp_data), pkt->jp_len)) < 0)
				continue;
		}
	}
}
