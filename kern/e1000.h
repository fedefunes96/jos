#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#define MAX_BYTES_ETHERNET 1518

enum {
	RETRY = 1,
	MAX_SIZE,
	EMPTY
};

//Exercise 5
struct tx_desc {
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
};

//Exercise 10
//From https://pdos.csail.mit.edu/6.828/2017/labs/lab6/e1000_hw.h
struct rx_desc {
    uint64_t addr; /* Address of the descriptor's data buffer */
    uint16_t length;     /* Length of data DMAed into data buffer */
    uint16_t csum;       /* Packet checksum */
    uint8_t status;      /* Descriptor status */
    uint8_t errors;      /* Descriptor Errors */
    uint16_t special;
};

struct package_desc {
	char bytes[MAX_BYTES_ETHERNET];
};

//Using C - Bit fields
struct tctl_bits {
	unsigned int rst : 1;
	unsigned int en : 1;
	unsigned int bce : 1;
	unsigned int psp : 1;
	//ct : 0xff = 8 bits
	unsigned int ct : 8;
	//cold : 0x3ff = 10 bits
	unsigned cold : 10;
};

struct tipg_bits {
	unsigned int ipgt : 10;
	unsigned int ipgr1 : 10;
	unsigned int ipgr2 : 10;
	unsigned int reserved : 2;
};

struct transmit_desc_tail {
	uint16_t tdt;
};

struct receive_desc_tail {
	uint16_t rdt;
};

int attach_initialize_e1000(struct pci_func *pcif);
/*
	Returns 0 on success
	-EMPTY if transmit queue is full
	-MAX_SIZE if trying to transmit a package with more bytes than MAX_BYTES_ETHERNET
*/
int transmit_packet(struct package_desc* package, ssize_t size);
/*
	Returns bytes received on success
	-EMPTY if receive queue is empty
*/
int receive_packet(struct package_desc* package);

#endif	// JOS_KERN_E1000_H
