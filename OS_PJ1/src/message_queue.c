#include <sys/msg.h>
#include "message_queue.h"

struct message
{
    long mtype;
    int cpu_burst;
};