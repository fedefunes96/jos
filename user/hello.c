// hello, world
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
	cprintf("i am environment %08x\n", sys_getenvid());
	cprintf("Value: 0x%x\n", (-1) & (1024-1));
}
