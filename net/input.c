#include "ns.h"

#include <inc/lib.h>

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.

	int r;
	int i = 0;

	if ((r = sys_page_alloc(0, (void*) REQVA, PTE_SYSCALL)) < 0)
		panic("Can't alloc a page");

	if ((r = sys_page_alloc(0, (void*) UTEMP, PTE_SYSCALL)) < 0)
		panic("Can't alloc a page");

	while (true) {
		if (i) {
			struct jif_pkt* pkt = (struct jif_pkt*) REQVA;

			if ((r = sys_receive_package((struct package_desc*) &(pkt->jp_data))) < 0)
				continue;

			pkt->jp_len = r;

			ipc_send(ns_envid, NSREQ_INPUT, (void*) REQVA, PTE_SYSCALL);
		} else {
			struct jif_pkt* pkt = (struct jif_pkt*) UTEMP;

			if ((r = sys_receive_package((struct package_desc*) &(pkt->jp_data))) < 0)
				continue;

			pkt->jp_len = r;

			ipc_send(ns_envid, NSREQ_INPUT, (void*) UTEMP, PTE_SYSCALL);
		}

		i = (i + 1) % 2;
	}
}
