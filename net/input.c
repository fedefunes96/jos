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

	//First alloc a page
	int r;

	if ((r = sys_page_alloc(0, &nsipcbuf, PTE_SYSCALL)) < 0)
		panic("Can't alloc a page");

	while (true) {
		struct jif_pkt* pkt = &nsipcbuf.pkt;

		if ((r = sys_receive_package((struct package_desc*) &(pkt->jp_data))) < 0)
			continue;

		pkt->jp_len = r;

		ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_SYSCALL);

		for (int i = 0; i < 20; i++)
			sys_yield();
	}
}