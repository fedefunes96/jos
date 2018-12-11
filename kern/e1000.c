#include <kern/e1000.h>

// LAB 6: Your driver code here
#include <kern/pci.h>
#include <inc/stdio.h>
#include <kern/pmap.h>
#include <inc/string.h>

#define E1000_STATUS 0x00008
#define E1000_TDLEN 0x03808
#define E1000_TDBAL 0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH 0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN 0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH 0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT 0x03818  /* TX Descriptor Tail - RW */
#define E1000_TCTL 0x00400  /* TX Control - RW */
#define E1000_TCTL_EXT 0x00404 /* Extended TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */

#define E1000_RDBAL 0x02800  /* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH 0x02804  /* RX Descriptor Base Address High - RW */
#define E1000_RDLEN 0x02808  /* RX Descriptor Length - RW */
#define E1000_RDH 0x02810  /* RX Descriptor Head - RW */
#define E1000_RDT 0x02818  /* RX Descriptor Tail - RW */
#define E1000_MTA 0x05200  /* Multicast Table Array - RW Array */
#define E1000_RA 0x05400  /* Receive Address - RW Array */

//Receive 05400h-05478h RAL(8n) Receive Address Low (n)
//Receive 05404h-0547Ch RAH(8n) Receive Address High (n)
#define E1000_RAL 0x05400  /* Receive Address Low - RW Array */
#define E1000_RAH 0x05404 //Receive Address High - RW Array */

#define E1000_RAH_AV  0x80000000        /* Receive descriptor valid */
#define E1000_RCTL 0x00100  /* RX Control - RW */
#define E1000_RCTL_EN 0x00000002    /* enable */
#define E1000_RCTL_LPE  0x00000020    /* long packet enable */
#define E1000_RCTL_LBM_MAC 0x00000040    /* MAC loopback mode */
#define E1000_RCTL_LBM_SLP 0x00000080    /* serial link loopback mode */
#define E1000_RCTL_RDMTS_HALF 0x00000000    /* rx desc min threshold size */
#define E1000_RCTL_RDMTS_QUAT 0x00000100    /* rx desc min threshold size */
#define E1000_RCTL_RDMTS_EIGTH 0x00000200    /* rx desc min threshold size */
#define E1000_RCTL_MO_SHIFT 12            /* multicast offset shift */
#define E1000_RCTL_MO_0 0x00000000    /* multicast offset 11:0 */
#define E1000_RCTL_MO_1 0x00001000    /* multicast offset 12:1 */
#define E1000_RCTL_MO_2 0x00002000    /* multicast offset 13:2 */
#define E1000_RCTL_MO_3 0x00003000    /* multicast offset 15:4 */
#define E1000_RCTL_BAM 0x00008000    /* broadcast enable */
#define E1000_RCTL_SZ_2048 0x00000000    /* rx buffer size 2048 */
#define E1000_RCTL_SZ_1024 0x00010000    /* rx buffer size 1024 */
#define E1000_RCTL_SZ_512 0x00020000    /* rx buffer size 512 */
#define E1000_RCTL_SECRC 0x04000000    /* Strip Ethernet CRC */

#define E1000_RXD_STAT_DD 0x01    /* Descriptor Done */
#define E1000_RXD_STAT_EOP 0x02    /* End of Packet */

#define E1000_TCTL_EN 0x00000002    /* enable tx */
#define E1000_TCTL_PSP 0x00000008    /* pad short packets */
#define E1000_TCTL_CT 0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD 0x003ff000    /* collision distance */

//#define E1000_TXD_CMD_RS 0x08000000 /* Report Status */
#define E1000_TXD_CMD_RS 0x8 /* Report Status */
//#define E1000_TXD_STAT_DD 0x00000001 /* Descriptor Done */
#define E1000_TXD_STAT_DD 0x1 /* Descriptor Done */
//#define E1000_TXD_CMD_RPS 0x10000000 /* Report Packet Sent */
#define E1000_TXD_CMD_RPS 0x10 /* Report Packet Sent */
#define E1000_TXD_CMD_EOP 0x1/* End of Packet */
//Exercise 4
volatile uint32_t *base_e1000;

//Exercise 5
//128 byte aligned and each transmit/receive descriptor is 16 bytes
//we need a minimun of 8 descriptors
#define MIN_TD 8
//Jos -> don't use more than 64 descriptors (for transmit)
//So we use 32
#define TDLEN (4*MIN_TD)
//Jos -> You should use at least 128 receive descriptors
//So we use 128
#define RDLEN (16*MIN_TD)

//Only use in this file
static struct tx_desc transmit_desc[TDLEN];
static struct package_desc transmit_package[TDLEN];
static struct rx_desc receive_desc[RDLEN];
static struct package_desc receive_package[RDLEN];

int attach_initialize_e1000(struct pci_func *pcif) {
	//Exercise 3
	pci_func_enable(pcif);

	//Exercise 4;
	base_e1000 = (uint32_t*) mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);

	cprintf("device status register: 0x%x\n", base_e1000[E1000_STATUS/sizeof(uint32_t)]);

	//Exercise 5
	//Allocate a region of memory for the transmit descriptor list.
	//Software should insure this memory is aligned on a paragraph (16-byte) boundary
	assert((uint32_t) transmit_desc % 16 == 0);

	//Use only TDBAL (We have 32-bit addresses)
	base_e1000[E1000_TDBAL/sizeof(uint32_t)] = PADDR(transmit_desc);
	//This register must be 128-byte aligned.
	base_e1000[E1000_TDLEN/sizeof(uint32_t)] = TDLEN * sizeof(struct tx_desc);

	//The Transmit Descriptor Head and Tail (TDH/TDT) registers are initialized (by hardware) to 0b
	//Software should write 0b to both these registers to ensure this
	base_e1000[E1000_TDH/sizeof(uint32_t)] = 0;

	//struct transmit_desc_tail set to 0
	base_e1000[E1000_TDT/sizeof(uint32_t)] = 0;
	struct tctl_bits* tctl = (struct tctl_bits*) &base_e1000[E1000_TCTL/sizeof(uint32_t)];

	tctl->en = 1;
	tctl->psp = 1;
	//Configure the Collision Threshold (TCTL.CT) to the desired value. Ethernet standard is 10h.
	tctl->ct = 0x10;
	//Configure the Collision Distance (TCTL.COLD) to its expected value. For full duplex
	//operation, this value should be set to 40h.	
	tctl->cold = 0x40;

	struct tipg_bits* tipg = (struct tipg_bits*) &base_e1000[E1000_TIPG/sizeof(uint32_t)];

	//For the IEEE 802.3 standard IPG value of 96-bit time, the value
	//that should be programmed into IPGT is 10
	tipg->ipgt = 10;
	//For the IEEE 802.3 standard IPG value of 96-bit time, the value
	//that should be programmed into IPGR2 is six 
	tipg->ipgr2 = 6;
	//According to the IEEE802.3 standard, IPGR1 should be 2/3 of IPGR2 value.
	tipg->ipgr1 = tipg->ipgr2 * 2/3;

	for (int i = 0; i < TDLEN; i++) {
		//Link descriptors with buffers
		transmit_desc[i].addr = PADDR((void*) &transmit_package[i]);
		//Set DD flag for initialization
		transmit_desc[i].status |= E1000_TXD_STAT_DD;
	}

	//Exercise 10
	//You can simply hard-code QEMU's default MAC address of 52:54:00:12:34:56
	base_e1000[E1000_RAL/sizeof(uint32_t)] = 0x12005452;
	//"Address Valid" bit in RAH
	base_e1000[E1000_RAH/sizeof(uint32_t)] = 0x00005634 | E1000_RAH_AV;

	//Initialize the MTA (Multicast Table Array) to 0b
	base_e1000[E1000_MTA/sizeof(uint32_t)] = 0;

	//Allocate a region of memory for the receive descriptor list. Software should insure this memory is
	//aligned on a paragraph (16-byte) boundary. 
	assert((uint32_t) receive_desc % 16 == 0);	

	//Use only RDBAL (We have 32-bit addresses)
	base_e1000[E1000_RDBAL/sizeof(uint32_t)] = PADDR(receive_desc);
	//This register must be 128-byte aligned.
	base_e1000[E1000_RDLEN/sizeof(uint32_t)] = RDLEN * sizeof(struct rx_desc);	

	//The Receive Descriptor Head and Tail registers are initialized (by hardware) to 0b after a power-on
	//or a software-initiated Ethernet controller reset.
	//Software initializes the Receive Descriptor Head (RDH) register 
	//and Receive Descriptor Tail (RDT) with the appropriate head and tail addresses.

	//Head should point to the first valid receive descriptor in the
	//descriptor ring and tail should point to one descriptor beyond the last valid descriptor in the
	//descriptor ring
	base_e1000[E1000_RDH/sizeof(uint32_t)] = 0;
	base_e1000[E1000_RDT/sizeof(uint32_t)] = RDLEN - 1;

	//Set the receiver Enable (RCTL.EN) bit to 1b for normal operation
	base_e1000[E1000_RCTL/sizeof(uint32_t)] |= E1000_RCTL_EN;

	//Set the Long Packet Enable (RCTL.LPE) bit to 1b when processing packets greater than the
	//standard Ethernet packet size.
	//Jos -> You can either deal with this possibility in your driver, or simply configure the card to not accept
	//"long packets" (also known as jumbo frames) and make sure your receive buffers are large enough 
	//to store the largest possible standard Ethernet packet (1518 bytes).
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_LPE;

	//Loopback Mode (RCTL.LBM) should be set to 00b for normal operation
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_LBM_MAC;
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_LBM_SLP;

	//Configure the Receive Descriptor Minimum Threshold Size (RCTL.RDMTS) bits to the
	//desired value.
	//00b = Free Buffer threshold is set to 1/2 of RDLEN.
	//01b = Free Buffer threshold is set to 1/4 of RDLEN.
	//10b = Free Buffer threshold is set to 1/8 of RDLEN.
	//11b = Reserved.
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_RDMTS_QUAT;
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_RDMTS_EIGTH;

	//Configure the Multicast Offset (RCTL.MO) bits to the desired value
	//In this case 00b
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_MO_1;
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_MO_2;

	//Set the Broadcast Accept Mode (RCTL.BAM) bit to 1b allowing the hardware to accept
	//broadcast packets
	base_e1000[E1000_RCTL/sizeof(uint32_t)] |= E1000_RCTL_BAM;

	//The E1000 only supports a specific set of receive buffer sizes (given in the description of RCTL.BSIZE
	//in 13.4.22). If you make your receive packet buffers large enough and disable 
	//long packets, you won't have to worry about packets spanning multiple receive buffers.
	//00b = 2048 Bytes.
	//01b = 1024 Bytes.
	//10b = 512 Bytes.
	//11b = 256 Bytes.
	//I'll then use 2048 bytes
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_SZ_1024;
	base_e1000[E1000_RCTL/sizeof(uint32_t)] &= ~E1000_RCTL_SZ_512;

	//Also, configure the E1000 to strip the Ethernet CRC, since the grade script expects it to be stripped.
	base_e1000[E1000_RCTL/sizeof(uint32_t)] |= E1000_RCTL_SECRC;

	for (int i = 0; i < RDLEN; i++) {
		//Link descriptors with buffers
		receive_desc[i].addr = PADDR((void*) &receive_package[i]);
		//Set DD flag for initialization (No packet received)
		receive_desc[i].status &= ~E1000_RXD_STAT_DD;
	}
	return 0;
}

int transmit_packet(struct package_desc* package, ssize_t size) {
	struct transmit_desc_tail* tail = (struct transmit_desc_tail*) &base_e1000[E1000_TDT/sizeof(uint32_t)];

	//Check if next descriptor is free
	if (!(transmit_desc[tail->tdt].status & E1000_TXD_STAT_DD)) {
		//transmit queue is full?
		return -RETRY;
	}

	//Sending more than max bytes
	if (size > MAX_BYTES_ETHERNET)
		return -MAX_SIZE;

	//Copying the packet data into the next packet buffer
	memmove((void*) &transmit_package[tail->tdt], package->bytes, size);

	transmit_desc[tail->tdt].length = size;
	transmit_desc[tail->tdt].cmd |= E1000_TXD_CMD_RS;
	transmit_desc[tail->tdt].cmd |= E1000_TXD_CMD_EOP;
	transmit_desc[tail->tdt].status &= ~E1000_TXD_STAT_DD;

	//Make it circular
	tail->tdt = (tail->tdt + 1) % TDLEN;

	return 0;
}

int receive_packet(struct package_desc* package) {	
	struct receive_desc_tail* tail = (struct receive_desc_tail*) &base_e1000[E1000_RDT/sizeof(uint32_t)];

	//Head has been incremented!
	//Can't use head value, so i'll use tail value
	uint16_t head_value = (tail->rdt + 1) % RDLEN;

	//Check if receive queue is empty
	if (!(receive_desc[head_value].status & E1000_RXD_STAT_DD)) {
		return -EMPTY;
	}

	//Copying the buffer packet to package
	memmove(package->bytes, (void*) &receive_package[head_value], receive_desc[head_value].length);

	receive_desc[head_value].status &= ~E1000_RXD_STAT_DD;

	//Make it circular
	tail->rdt = (tail->rdt + 1) % RDLEN;

	return receive_desc[head_value].length;
}
