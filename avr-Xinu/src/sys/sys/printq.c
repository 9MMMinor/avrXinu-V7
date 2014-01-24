#include <kernel.h>
#include <q.h>

void
printq(int head)
{
    int i, tail;

    tail = head + 1;
    kprintf("queue contents:\n\n");
    for(i=q[head].qnext; i != tail; i=q[i].qnext)
		kprintf("key: %d next: %d prev: %d\n",q[i].qkey,q[i].qnext,q[i].qprev);
}
